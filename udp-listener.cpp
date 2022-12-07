#include "udp-listener.h"
#include <iostream>
#include <cstring>
#include <syslog.h>
#include <csignal>
#include <cerrno>
#include <sys/select.h>

#include "third_party/get_rss/get_rss.h"
#include "utildate.h"
#include "utilstring.h"
#include "errlist.h"

#define DEF_BUFFER_SIZE     4096

UDPListener::UDPListener() : PacketListener()
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

std::string UDPListener::toString() const{
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

bool UDPListener::addSocket(
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
			onLog->logMessage(this, LOG_ERR, LOG_UDP_LISTENER, s.errcode, ss.str());
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

bool UDPListener::add(
    const std::string& value,
    int hint
)
{
    return addSocket(value, (MODE_FAMILY) hint);
}

int UDPListener::parseBuffer(
    const std::string &buffer,
    size_t bytesReceived,
    int socket,
    const struct timeval &receivedTime,
    const struct sockaddr_in6 &gwAddress
) {
    std::vector<SemtechUDPPacket> packets;
    // get packets
    SEMTECH_PREFIX_GW dataPrefix;
    GatewayStat gatewayStat;
    int pr = LORA_OK;
    if (bytesReceived < sizeof(SEMTECH_PREFIX))
        pr = ERR_CODE_PACKET_TOO_SHORT;
    else {
        SEMTECH_PREFIX *prefix = (SEMTECH_PREFIX *) buffer.c_str();
        switch (prefix->tag) {
            case SEMTECH_GW_PUSH_DATA:
                pr = SemtechUDPPacket::parse((const struct sockaddr *) &gwAddress, dataPrefix,
                                             gatewayStat, packets, buffer.c_str(), bytesReceived, identityService);
                if (handler) {
                    if (pr == LORA_OK) {
                        // send ACK immediately
                        handler->ack(socket, (const sockaddr_in *) &gwAddress, dataPrefix);
                    }
                    if (pr == ERR_CODE_IS_JOIN) {
                        // send ACK immediately too
                        handler->ack(socket, (const sockaddr_in *) &gwAddress, dataPrefix);
                        if (packets.size() > 0) {
                            // enqueueTxPacket JOIN request packet
                            handler->join(receivedTime, socket, (const sockaddr_in *) &gwAddress, packets[0]);
                        }
                    }
                }
                break;
            case SEMTECH_GW_PULL_DATA:	// PULL_DATA
                gatewayStat.errcode = ERR_CODE_NO_GATEWAY_STAT;
                pr = SemtechUDPPacket::parsePrefixGw(dataPrefix, buffer.c_str(), bytesReceived);
                if (pr != LORA_OK)
                    break;
                // check is gateway in service
                {
                    std::stringstream sse;
                    sse << strerror_lorawan_ns(pr)
                        << " " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
                        << ", token: " << std::hex << dataPrefix.token;
                    onLog->logMessage(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, sse.str());
                    // send PULL ACK immediately
                    if (handler) {
                        handler->ack(socket, (const sockaddr_in *) &gwAddress, dataPrefix);
                    }
                    if (gatewayList) {
                        if (!gatewayList->setSocketAddress(dataPrefix.mac, socket, (const struct sockaddr_in *) &gwAddress)) {
                            std::stringstream ss;
                            ss << ERR_MESSAGE << ERR_CODE_INVALID_GATEWAY_ID << ": "
                               << ERR_INVALID_GATEWAY_ID
                               << " from " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
                               << " gateway: " << DEVEUI2string(dataPrefix.mac);
                            ;
                            onLog->logMessage(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SEND_ACK, ss.str());
                            break;
                        }
                    }
                }
                break;
            case SEMTECH_GW_TX_ACK:	// TX_ACK
                //
                gatewayStat.errcode = ERR_CODE_NO_GATEWAY_STAT;
                {
                    pr = SemtechUDPPacket::parsePrefixGw(dataPrefix, buffer.c_str(), bytesReceived);
                    if (pr != LORA_OK)
                        break;
                    ERR_CODE_TX r = extractTXAckCode(buffer.c_str(), bytesReceived);
                    std::stringstream ss;
                    if (r) {
                        ss << ERR_MESSAGE << " " << r << ": ";
                        // TODO re-send in second window or later if device is class C
                    }
                    ss << "TX ACK " << getTXAckCodeName(r)
                       << " from " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
                       << " gateway: " << DEVEUI2string(dataPrefix.mac);
                    onLog->logMessage(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
                    pr = LORA_OK;
                }
                break;
            default:
                pr = ERR_CODE_INVALID_PACKET;
                break;
        }
    }

    if (onDeviceStatDump) {
        for (std::vector<SemtechUDPPacket>::iterator itp(packets.begin()); itp != packets.end(); itp++) {
            onDeviceStatDump(deviceStatEnv, *itp);
        }
    }
    switch (pr) {
        case ERR_CODE_IS_JOIN:
            break;
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
            onLog->logMessage(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, sse.str());
        }
            break;
        case ERR_CODE_PULLOUT:
        {
        }
            break;
        default: // including ERR_CODE_INVALID_PACKET, it will contain some valid packets in the JSON, continue
            // process data packets if exists
            for (std::vector<SemtechUDPPacket>::iterator itp(packets.begin()); itp != packets.end(); itp++) {
                if (itp->errcode) {
                    std::string v = std::string(buffer.c_str(), bytesReceived);
                    std::stringstream ss;
                    ss << ERR_MESSAGE << ERR_CODE_INVALID_PACKET << " "
                       << ": " << ERR_INVALID_PACKET
                       << ", gateway address: " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
                       << ", packet: " << hexString(v);
                    onLog->logMessage(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_INVALID_PACKET, ss.str());
                    continue;
                } else {
                    if (onLog) {
                        std::stringstream ss;
                        ss << MSG_RXPK
                           << itp->toDebugString();
                        onLog->logMessage(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
                    }

                    if (handler) {
                        handler->put(receivedTime, *itp);
                    } else {
                        if (onLog) {
                            std::stringstream ss;
                            ss << MSG_READ_BYTES
                               << UDPSocket::addrString((const struct sockaddr *) &gwAddress) << ": "
                               << itp->toString();
                            onLog->logMessage(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
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
                   << gatewayStat.toString() << ". Server memory " << getCurrentRSS()/ 1024 << "K";
                onLog->logMessage(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
                if (onGatewayStatDump)
                    onGatewayStatDump(gwStatEnv, &gatewayStat);
            }
            break;
    }
    return pr;
}

/**
 * Listen UDP port(s)
 * @param config not used,
 * @return
 */
int UDPListener::listen(void *config) {
    int sz = sockets.size();
	if (!sz)
		return ERR_CODE_SOCKET_NO_ONE;

    std::stringstream ss;
    ss << MSG_LISTEN_SOCKETS;
    for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
        ss << it->toString() << " ";
    }
    ss << sz << MSG_LISTEN_SOCKET_COUNT;
    onLog->logMessage(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());

    while (!stopped) {
		fd_set readHandles;
	    FD_ZERO(&readHandles);
		for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
			FD_SET(it->sock, &readHandles);
		}

		struct timeval timeoutInterval;
        timeoutInterval.tv_sec = 1;
        timeoutInterval.tv_usec = 0;

        int rs = select(largestSocket() + 1, &readHandles, nullptr, nullptr, &timeoutInterval);
        if (rs == -1) {
			int serrno = errno;
			if (onLog) {
				std::stringstream ss;
				ss << ERR_MESSAGE << ERR_CODE_SELECT << ": " << ERR_SELECT
					<< ", errno " << serrno << ": " << strerror(errno);
				onLog->logMessage(this, LOG_WARNING, LOG_UDP_LISTENER, ERR_CODE_SELECT, ss.str());
			}
			if (serrno == EINTR){ // Interrupted system call
				if (sysSignalPtr) {
					if (*sysSignalPtr == 0 || *sysSignalPtr == SIGUSR2)
						continue;
				}
			}
			return ERR_CODE_SELECT;
		}
		if (rs == 0) {
			// timeout, nothing to do
			// std::stringstream ss;ss << MSG_TIMEOUT;logMessage(LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
			continue;
		}
		struct timeval receivedTime;
		gettimeofday(&receivedTime, nullptr);
		// By default, there are two sockets: one for IPv4, second for IPv6
		for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
			if (!FD_ISSET(it->sock, &readHandles))
				continue;
			struct sockaddr_in6 gwAddress;
			int bytesReceived = it->recv((void *) buffer.c_str(), buffer.size() - 1, &gwAddress);	// add extra trailing byte for null-terminated string
			if (bytesReceived <= 0) {
				if (onLog) {
					std::stringstream ss;
					ss << ERR_MESSAGE << ERR_CODE_SOCKET_READ << " "
						<< UDPSocket::addrString((const struct sockaddr *) &gwAddress) << ", errno "
						<< errno << ": " << strerror(errno);
					onLog->logMessage(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SOCKET_READ, ss.str());
				}
				continue;
			}
			// rapidjson operates with \0 terminated string, just in case add terminator. Extra space is reserved
			buffer[bytesReceived] = '\0';
			std::stringstream ss;
			char *json = SemtechUDPPacket::getSemtechJSONCharPtr(buffer.c_str(), bytesReceived);
			ss << MSG_RECEIVED
				<< UDPSocket::addrString((const struct sockaddr *) &gwAddress)
				<< " (" << bytesReceived
				<< " bytes): " << hexString(buffer.c_str(), bytesReceived);
			if (json)
				ss << "; " << json;
			onLog->logMessage(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());

			// parseRX packet result code
			int pr = parseBuffer(buffer, bytesReceived, it->sock, receivedTime, gwAddress);
		}
	}
	return LORA_OK;
}

void UDPListener::setLastRemoteAddress(
	struct sockaddr *value
	) {
	if (value->sa_family == AF_INET6)
		memmove(&remotePeerAddress, value, sizeof(struct sockaddr_in6));
	else
		memmove(&remotePeerAddress, value, sizeof(struct sockaddr_in));
}
