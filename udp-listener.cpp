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
#include "utillora.h"

#define DEF_BUFFER_SIZE     4096

UDPListener::UDPListener() :
	stopped(false), onLog(NULL)
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

int UDPListener::listen() {
	int sz = sockets.size();
	if (!sz)
		return ERR_CODE_SOCKET_NO_ONE;
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
				break;
        	default:
			{
				for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
					if (!FD_ISSET(it->sock, &readHandles))
						continue;
					struct sockaddr_in6 clientAddress;
					int bytesReceived = it->recv((void *) buffer.c_str(), buffer.size(), &clientAddress);
					if (bytesReceived >= 0) {
						std::vector<semtechUDPPacket> packets;
						semtechUDPPacket::parse(packets, buffer.c_str(), bytesReceived);
						for (std::vector<semtechUDPPacket>::iterator it(packets.begin()); it != packets.end(); it++) {
							if (it->errcode) {
								std::string v = std::string(buffer.c_str(), bytesReceived);
								std::stringstream ss;
								ss << ERR_MESSAGE << ERR_CODE_INVALID_PACKET << " "
									<< UDPSocket::addrString((const struct sockaddr *) &clientAddress)
									<< ": " << ERR_INVALID_PACKET << ", " << hexString(v);
								onLog(LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_INVALID_PACKET, ss.str());
								break;
							}
							if (onLog) {
								std::stringstream ss;
								ss << MSG_READ_BYTES 
									<< UDPSocket::addrString((const struct sockaddr *) &clientAddress) << ": "
									<< it->toString();
								onLog(LOG_INFO, LOG_UDP_LISTENER, ERR_CODE_SOCKET_READ, ss.str());
							}
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
