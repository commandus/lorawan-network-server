#include "embedded-listener.h"
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

EmbeddedListener::EmbeddedListener() : PacketListener()
{
}

EmbeddedListener::~EmbeddedListener() {
	clear();
}

void EmbeddedListener::setBufferSize(size_t value) {
}

std::string EmbeddedListener::toString() const{
	return "";
}

void EmbeddedListener::clear() {
}

bool EmbeddedListener::add(
    const std::string& value,
    int hint
)
{
    return true;
}

int EmbeddedListener::listen() {
    std::stringstream ss;
    ss << MSG_LISTEN_SOCKETS;
    onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
    struct sockaddr_in6 gwAddress;
    memset(&gwAddress, 0, sizeof(struct sockaddr_in6));

    while (!stopped) {
		struct timeval receivedTime;
		gettimeofday(&receivedTime, nullptr);
		// By default, there are two sockets: one for IPv4, second for IPv6
        int bytesReceived = 0; // it->recv((void *) buffer.c_str(), buffer.size() - 1, &gwAddress);
        if (bytesReceived <= 0) {
            if (onLog) {
                std::stringstream ss;
                ss << ERR_MESSAGE << ERR_CODE_SOCKET_READ << " "
                    << ", errno "
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
            << " (" << bytesReceived
            << " bytes): " << hexString(buffer.c_str(), bytesReceived);
        if (json)
            ss << "; " << json;
        onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());

        // parseRX packet result code
        int pr = parseBuffer(buffer, bytesReceived, 0, receivedTime, gwAddress);
	}
	return LORA_OK;
}
