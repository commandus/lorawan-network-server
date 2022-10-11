#include "udp-listener.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <syslog.h>
#include <csignal>
#include <cerrno>
#include <sys/select.h>

#include "third_party/get_rss/get_rss.h"
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

bool UDPListener::add(
    const std::string& value,
    int hint
)
{
    return addSocket(value, (MODE_FAMILY) hint);
}

int UDPListener::listen() {
    int sz = sockets.size();
	if (!sz)
		return ERR_CODE_SOCKET_NO_ONE;

    std::stringstream ss;
    ss << MSG_LISTEN_SOCKETS;
    for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
        ss << it->toString() << " ";
    }
    ss << sz << MSG_LISTEN_SOCKET_COUNT;
    onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());

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
				onLog(this, LOG_WARNING, LOG_UDP_LISTENER, ERR_CODE_SELECT, ss.str());
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
		gettimeofday(&receivedTime, NULL);
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
					onLog(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SOCKET_READ, ss.str());
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
			onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());

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
