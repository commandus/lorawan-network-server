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
	stopped(false), onLog(NULL), handler(NULL), identityService(NULL), gatewayList(NULL)
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
	std::function<void(
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value) {
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
				<< strerror_client(s.errcode) << " " << address
				<< ", errno " << s.lasterrno << ": " << strerror(s.lasterrno)
				;
			onLog(LOG_ERR, LOG_UDP_LISTENER, s.errcode, ss.str());
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

/**
 * @return 0- success, ERR_CODE_SEND_ACK- error occured
 */ 
int UDPListener::sendAck(
	const UDPSocket &socket,
	const struct sockaddr_in *dest,
	const SEMTECH_ACK &response
)
{
	size_t r = sendto(socket.sock, &response, sizeof(SEMTECH_ACK), 0,
		(const struct sockaddr*) dest,
		((dest->sin_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)));
	return r == sizeof(SEMTECH_ACK) ? 0 : ERR_CODE_SEND_ACK;
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
        switch (rs) {
			case -1: 
				if (onLog) {
					std::stringstream ss;
					ss << ERR_MESSAGE << ERR_CODE_SELECT << ": " << ERR_SELECT 
						<< ", errno " << errno << ": " << strerror(errno);
					onLog(LOG_WARNING, LOG_UDP_LISTENER, ERR_CODE_SELECT, ss.str());
					return ERR_CODE_SELECT;
				}
				break;
			case 0:
				// timeout, nothing to do
				// std::stringstream ss;ss << MSG_TIMEOUT;onLog(LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
				break;
        	default:
			{
				// By default two sockets: one for IPv4, second for IPv6
				for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
					if (!FD_ISSET(it->sock, &readHandles))
						continue;
					struct sockaddr_in6 clientAddress;
					int bytesReceived = it->recv((void *) buffer.c_str(), buffer.size() - 1, &clientAddress);	// add extra trailing byte for null-terminated string
					if (bytesReceived >= 0) {
						std::vector<semtechUDPPacket> packets;
						{
							std::stringstream ss;
							ss << MSG_RECEIVED
								<< UDPSocket::addrString((const struct sockaddr *) &clientAddress)
								<< ": " << hexString(buffer.c_str(), bytesReceived);
							onLog(LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
						}
						// get packets
						SEMTECH_DATA_PREFIX dataprefix;
						// rapidjson operates with \0 terminated string, just in case add terminator. Extra space is reserved
						buffer[bytesReceived] = '\0';
						int pr = semtechUDPPacket::parse(dataprefix, gatewayStat, packets, buffer.c_str(), bytesReceived, identityService);

						// std::cerr << "===" << pr << ": " << strerror_client(pr) << std::endl;

						// send ACK immediately
						switch (pr) {
							case ERR_CODE_INVALID_JSON:
								// it is completely invalid packet, do not response
								break;
							default: // including ERR_CODE_INVALID_PACKET, it can contains some valid packets in the JSON, continue

								// send ACK immediately
								SEMTECH_ACK response;
								response.version = 2;
								response.token = dataprefix.token;
								switch(dataprefix.tag) {
									default:
										response.tag = dataprefix.tag + 1;
										break;
								}
							
								int ar = sendAck(*it, (const sockaddr_in*) &clientAddress, response);
								if (ar) {
									std::stringstream ss;
									ss << ERR_MESSAGE << ERR_CODE_SEND_ACK << " "
										<< UDPSocket::addrString((const struct sockaddr *) &clientAddress)
										<< " " << ar << ": " << strerror_client(ar)
										<< ", errno: " << errno << ": " << strerror(errno);
									onLog(LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SEND_ACK, ss.str());
								} else {
									std::stringstream ss;
									ss << MSG_SENT_ACK_TO
										<< UDPSocket::addrString((const struct sockaddr *) &clientAddress);
									onLog(LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
								}
								break;
						}
						// process PULL data if exists
						switch (dataprefix.tag) {
							case 0:
								// process data packets if exists
								for (std::vector<semtechUDPPacket>::iterator itp(packets.begin()); itp != packets.end(); itp++) {
									memmove(&itp->clientAddress, &clientAddress,
										(clientAddress.sin6_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)));
									if (itp->errcode) {
										std::string v = std::string(buffer.c_str(), bytesReceived);
										std::stringstream ss;
										ss << ERR_MESSAGE << ERR_CODE_INVALID_PACKET << " "
											<< UDPSocket::addrString((const struct sockaddr *) &clientAddress)
											<< ": " << ERR_INVALID_PACKET << ", " << hexString(v);
										onLog(LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_INVALID_PACKET, ss.str());
										break;
									} else {
										if (onLog) {
											std::stringstream ss;
											ss << MSG_RXPK
												<< itp->toDebugString();
											onLog(LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
										}
									}
									if (handler) {
										handler->put(*itp);
									} else {
										if (onLog) {
											std::stringstream ss;
											ss << MSG_READ_BYTES 
												<< UDPSocket::addrString((const struct sockaddr *) &clientAddress) << ": "
												<< itp->toString();
											onLog(LOG_INFO, LOG_UDP_LISTENER, ERR_CODE_SOCKET_READ, ss.str());
										}
									}
								}
								// reflect stat
								if (gatewayStat.errcode == 0) {
									if (gatewayList) {
										gatewayList->update(gatewayStat);
									}
									std::stringstream ss;
									ss << MSG_GATEWAY_STAT
										<< gatewayStat.toString();
									onLog(LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
								}
								break;
							case 1:
								break;
							case 2:	// PULL_DATA
								// check is gateway in service
								break;
							default:
								break;
						}
					} else {
						if (onLog) {
							std::stringstream ss;
							ss << ERR_MESSAGE << ERR_CODE_SOCKET_READ << " "
								<< UDPSocket::addrString((const struct sockaddr *) &clientAddress) << ", errno "
								<< errno << ": " << strerror(errno);
							onLog(LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SOCKET_READ, ss.str());
						}
					}
				}
				break;
			}
        }
    }
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
