#include "udp-listener.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <syslog.h>
#include <sys/select.h>

#include "utilstring.h"
#include "errlist.h"

#define DEF_BUFFER_SIZE     4096

UDPListener::UDPListener() :
	verbosity(0), stopped(false), onLog(NULL), handler(NULL), identityService(NULL), gatewayList(NULL)
{
	memset(&remotePeerAddress, 0, sizeof(struct sockaddr_in));
	setBufferSize(DEF_BUFFER_SIZE);
}

UDPListener::~UDPListener() {
	clear();
}

void UDPListener::setBufferSize(size_t value) {
	buffer.resize(value);
}

void UDPListener::clearLogger() {
	onLog = NULL;
}

void UDPListener::setLogger(
	int averbosity,
	std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value) 
{
	verbosity = averbosity;
	onLog = value;
}

void UDPListener::setHandler(
	LoraPacketHandler *value
) {
	handler = value;
}

void UDPListener::setGatewayList(
	GatewayList *value
)
{
	gatewayList = value;
}

void UDPListener::setIdentityService
(
	IdentityService* value
)
{
	identityService = value;
}


std::string UDPListener::toString() {
	std::stringstream ss;
	for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
		ss << it->toString() << std::endl;
	}
	return ss.str();
}

void UDPListener::clear() {
	for (std::vector<UDPSocket>::iterator it = sockets.begin(); it != sockets.end(); it++) {
		it->closeSocket();
	}
	sockets.clear();	
}

bool UDPListener::add(
	const std::string &address,
	MODE_FAMILY familyHint
) {
	UDPSocket s(address, MODE_OPEN_SOCKET_LISTEN, familyHint);
	if (s.errcode) {
		if (onLog) {
			std::stringstream ss;
			ss << ERR_MESSAGE << s.errcode << ": " 
				<< strerror_lorawan_ns(s.errcode) << " " << address
				<< ", errno " << s.lasterrno << ": " << strerror(s.lasterrno)
				;
			onLog(this, LOG_ERR, LOG_UDP_LISTENER, s.errcode, ss.str());
		}
		return false;
	}
	sockets.push_back(s);	// copy socket to the vector
	return true;
}

int UDPListener::largestSocket() {
	int r = -1;
	for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
		if (it->sock > r)
			r = it->sock;
	}
	return r;
}

int UDPListener::listen() {
	int sz = sockets.size();
	if (!sz)
		return ERR_CODE_SOCKET_NO_ONE;
	GatewayStat gatewayStat;
	while (!stopped) {
		fd_set readHandles;
	    FD_ZERO(&readHandles);
		for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
			FD_SET(it->sock, &readHandles);
		}

		struct timeval timeoutInterval;
        timeoutInterval.tv_sec = 1;
        timeoutInterval.tv_usec = 0;

        int rs = select(largestSocket() + 1, &readHandles, NULL, NULL, &timeoutInterval);
        if (rs == -1) {
			if (onLog) {
				std::stringstream ss;
				ss << ERR_MESSAGE << ERR_CODE_SELECT << ": " << ERR_SELECT 
					<< ", errno " << errno << ": " << strerror(errno);
				onLog(this, LOG_WARNING, LOG_UDP_LISTENER, ERR_CODE_SELECT, ss.str());
			}
			return ERR_CODE_SELECT;
		}
		if (rs == 0) {
			// timeout, nothing to do
			// std::stringstream ss;ss << MSG_TIMEOUT;onLog(LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
			continue;
		}

		struct timeval recievedTime;
		gettimeofday(&recievedTime, NULL);
		
		// By default two sockets: one for IPv4, second for IPv6
		for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
			if (!FD_ISSET(it->sock, &readHandles))
				continue;
			struct sockaddr_in6 gwAddress;
			int bytesReceived = it->recv((void *) buffer.c_str(), buffer.size() - 1, &gwAddress);	// add extra trailing byte for null-terminated string
			if (bytesReceived <=0) {
				if (onLog) {
					std::stringstream ss;
					ss << ERR_MESSAGE << ERR_CODE_SOCKET_READ << " "
						<< UDPSocket::addrString((const struct sockaddr *) &gwAddress) << ", errno "
						<< errno << ": " << strerror(errno);
					onLog(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SOCKET_READ, ss.str());
				}
				continue;
			}

			std::vector<semtechUDPPacket> packets;

			// get packets
			SEMTECH_PREFIX_GW dataprefix;
			// rapidjson operates with \0 terminated string, just in case add terminator. Extra space is reserved
			buffer[bytesReceived] = '\0';
			std::stringstream ss;
			char *json = semtechUDPPacket::getSemtechJSONCharPtr(buffer.c_str(), bytesReceived);
			ss << MSG_RECEIVED
				<< UDPSocket::addrString((const struct sockaddr *) &gwAddress)
				<< " (" << bytesReceived
				<< " bytes): " << hexString(buffer.c_str(), bytesReceived);
			if (json) {
				ss << "; " << json;
			}
			onLog(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());

			// parse packet result code
			int pr = LORA_OK;
			if (bytesReceived < sizeof(SEMTECH_PREFIX)) 
				pr = ERR_CODE_PACKET_TOO_SHORT;
			else {
				SEMTECH_PREFIX *prefix = (SEMTECH_PREFIX *) buffer.c_str();
				switch (prefix->tag) {
					case SEMTECH_GW_PUSH_DATA:
						pr = semtechUDPPacket::parse((const struct sockaddr *) &gwAddress, dataprefix, gatewayStat, packets, buffer.c_str(), bytesReceived, identityService);
						// send ACK immediately
						if (pr == LORA_OK && handler) {
							handler->ack(it->sock, (const sockaddr_in *) &gwAddress, dataprefix);
						}
						break;
					case SEMTECH_GW_PULL_DATA:	// PULL_DATA
						// check is gateway in service
						if (bytesReceived < sizeof(SEMTECH_PREFIX_GW))
							pr = ERR_CODE_PACKET_TOO_SHORT;
						else {
							std::stringstream sse;
							sse << strerror_lorawan_ns(pr)
								<< " " << UDPSocket::addrString((const struct sockaddr *) &gwAddress) 
								<< ", token: " << std::hex << dataprefix.token;
							onLog(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, sse.str());
							// send PULL ACK immediately
							if (handler) {
								handler->ack(it->sock, (const sockaddr_in *) &gwAddress, dataprefix);
							}
							if (gatewayList) {
								if (!gatewayList->setSocketAddress(dataprefix.mac, it->sock, (const struct sockaddr_in *) &gwAddress)) {
									std::stringstream ss;
									ss << ERR_MESSAGE << ERR_CODE_INVALID_GATEWAY_ID << ": "
										<< ERR_INVALID_GATEWAY_ID
										<< " from " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
										<< " gateway: " << DEVEUI2string(dataprefix.mac);
										;
									onLog(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SEND_ACK, ss.str());
									break;
								}
							}
						}
						break;
					case SEMTECH_GW_TX_ACK:	// TX_ACK
						//
						if (bytesReceived < sizeof(SEMTECH_PREFIX_GW))
							pr = ERR_CODE_PACKET_TOO_SHORT;
						else {
							SEMTECH_PREFIX_GW *prefixGw = (SEMTECH_PREFIX_GW *) buffer.c_str();

							ERR_CODE_TX r = extractTXAckCode(buffer.c_str(), bytesReceived);
							std::stringstream ss;
							ss << "TX ACK: " << getTXAckCodeName(r)
								<< " from " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
								<< " gateway: " << DEVEUI2string(prefixGw->mac);
								;
							onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
							pr = LORA_OK;
						}
						break;
					default:
						pr = ERR_CODE_INVALID_PACKET;
						break;
				}
			}

			// std::cerr << "===" << pr << ": " << strerror_lorawan_ns(pr) << std::endl;
			switch (pr) {
				case ERR_CODE_PACKET_TOO_SHORT:
				case ERR_CODE_INVALID_PROTOCOL_VERSION:
				case ERR_CODE_NO_GATEWAY_STAT:
				case ERR_CODE_INVALID_STAT:
				case ERR_CODE_INVALID_PACKET:
				case ERR_CODE_INVALID_JSON:
				// ignore invalid mic, inconsistent packet removed, json can contain other valid packets
				// case ERR_CODE_INVALID_MIC: 
					{
						std::stringstream sse;
						sse << strerror_lorawan_ns(pr)
							<< " " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
							<< " (" << bytesReceived
							<< " bytes): " << hexString(buffer.c_str(), bytesReceived);
						onLog(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, sse.str());
					}
					break;
				case ERR_CODE_PULLOUT:
					{
					}
					break;
				default: // including ERR_CODE_INVALID_PACKET, it can contains some valid packets in the JSON, continue
					// process data packets if exists
					for (std::vector<semtechUDPPacket>::iterator itp(packets.begin()); itp != packets.end(); itp++) {
						if (itp->errcode) {
							std::string v = std::string(buffer.c_str(), bytesReceived);
							std::stringstream ss;
							ss << ERR_MESSAGE << ERR_CODE_INVALID_PACKET << " "
								<< UDPSocket::addrString((const struct sockaddr *) &gwAddress)
								<< ": " << ERR_INVALID_PACKET << ", " << hexString(v);
							onLog(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_INVALID_PACKET, ss.str());
							continue;
						} else {
							if (onLog) {
								std::stringstream ss;
								ss << MSG_RXPK
									<< itp->toDebugString();
								onLog(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
							}
							if (handler) {
								handler->put(recievedTime, *itp);
							} else {
								if (onLog) {
									std::stringstream ss;
									ss << MSG_READ_BYTES 
										<< UDPSocket::addrString((const struct sockaddr *) &gwAddress) << ": "
										<< itp->toString();
									onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
								}
							}

						}
					}
					// reflect stat
					if (gatewayStat.errcode == 0) {
						if (gatewayList)
							gatewayList->copyId(gatewayStat, (const sockaddr *) &gwAddress);
						std::stringstream ss;
						ss << MSG_GATEWAY_STAT
							<< gatewayStat.toString();
						onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
					}
					break;
			}
		}
	}
	return LORA_OK;
}

int UDPListener::peerAddrIndex(
	struct sockaddr *remotePeerAddr
) {
	int r = 0;
	for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
		if (it->isPeerAddr(remotePeerAddr))
			return r;
		r++;
	}
	return -1;
}

void UDPListener::setLastRemoteAddress(
	struct sockaddr *value
	) {
	if (value->sa_family == AF_INET6)
		memmove(&remotePeerAddress, value, sizeof(struct sockaddr_in6));
	else
		memmove(&remotePeerAddress, value, sizeof(struct sockaddr_in));
}
