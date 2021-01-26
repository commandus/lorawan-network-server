#include "udp-emitter.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

#define DEF_BUFFER_SIZE     4096

UDPEmitter::UDPEmitter() :
	stopped(false), daemonize(false), verbosity(0),
	logfilename(""), logstream(&std::cerr)
{
	memset(&remotePeerAddress, 0, sizeof(struct sockaddr_in));
	setBufferSize(DEF_BUFFER_SIZE);
}

UDPEmitter::~UDPEmitter() {
	if (logstream) {
	if (!logfilename.empty())
		delete logstream;
	logstream = NULL;
	}
}

void UDPEmitter::setBufferSize(size_t value) {
	buffer.resize(value);
}

std::string UDPEmitter::toString() {
	return mSocket.toString();
}

void UDPEmitter::closeSocket() {
	close(mSocket.socket);
}

int UDPEmitter::openSocket(
	UDPSocket &retval,
	const char* address,
	const char* port
) {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	struct addrinfo *addr;
	
	int r = getaddrinfo(address, port, &hints, &addr);
	if (r != 0 || addr == NULL) {
		std::cerr << ERR_GET_ADDRESS << errno << ": " << strerror(errno) << std::endl;
		return -1;
	}
	retval.socket = socket(addr->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
	memmove(&retval.addr, addr, sizeof(struct addrinfo));
	freeaddrinfo(addr);
	
	if (retval.socket == -1) {
		std::cerr << ERR_OPEN_SOCKET << errno << ": " << strerror(errno) << std::endl;
		return -1;
	}
	return retval.socket;
}

int UDPEmitter::listenSocket(
	UDPSocket &retval,
	const char* address,
	const char* port
) {
	int sock = openSocket(retval, address, port);
	if (sock >= 0) {
	int r = bind(sock, retval.addr.ai_addr, retval.addr.ai_addrlen);
	if (r < 0) {
		std::cerr << ERR_BIND << errno << ": "<< strerror(errno) << std::endl;
		close(sock);
		return r;
	}
	}
	return sock;
}

bool UDPEmitter::setAddress(
	const std::string &address
) {
	size_t pos = address.find(":");
	if (pos == std::string::npos)
	return false;
	int sock;
	UDPSocket s;

	std::string h(address.substr(0, pos));
	std::string p(address.substr(pos + 1));

	// mSocket.socket = listenSocket(s, h.c_str(), p.c_str());
	mSocket.socket = openSocket(s, h.c_str(), p.c_str());
	return (mSocket.socket >= 0);
}

int UDPEmitter::receive(
	struct sockaddr_in *remotePeerAddr,
	int max_wait_s
) {
	fd_set s;
	FD_ZERO(&s);
	FD_SET(mSocket.socket, &s);
	struct timeval timeout;
	timeout.tv_sec = max_wait_s;
	timeout.tv_usec = 0;
	int retval = select(mSocket.socket + 1, &s, &s, &s, &timeout);
	if (retval == -1) {
		// select() set errno accordingly
		return -1;
	}
	if (retval > 0) {
		// our socket has data
		socklen_t addrlen = sizeof(struct sockaddr_in); 
		return recvfrom(mSocket.socket, (void *) buffer.c_str(), buffer.size(), 0, (struct sockaddr *) remotePeerAddr, &addrlen);
	}

	// our socket has no data
	errno = EAGAIN;
	return -1;
}

bool UDPEmitter::isPeerAddr(
	struct sockaddr_in *remotePeerAddr
) {
	struct sockaddr_in *s = (struct sockaddr_in *) mSocket.addr.ai_addr;
	return (s->sin_family == remotePeerAddr->sin_family && 
	memcmp(&s->sin_addr, &remotePeerAddr->sin_addr, sizeof(struct in_addr)) == 0 && 
	s->sin_port == remotePeerAddr->sin_port
	);
}

int UDPEmitter::sendDown(
	size_t size
) {
	if (logstream && verbosity > 2) {
	struct sockaddr_in *s = (struct sockaddr_in *) mSocket.addr.ai_addr;
	*logstream << hexString(buffer.c_str(), size) 
		<< " -> " << inet_ntoa(s->sin_addr) << ":" << ntohs(s->sin_port) << std::endl;
	}

	size_t r = sendto(mSocket.socket, buffer.c_str(), size, 0, mSocket.addr.ai_addr, mSocket.addr.ai_addrlen);
	if (r < 0)
	return r;
	return 0;
}

int UDPEmitter::sendUp(
	size_t size
) {
	// skip first socket
	// size_t r = sendto(sockets[0].socket, msg, size, 0, addr.ai_addr, addr.ai_addrlen);
	if (logstream && verbosity > 2) {
	*logstream << hexString(buffer.c_str(), size) 
		<< " <- " << inet_ntoa(remotePeerAddress.sin_addr) << ":" << ntohs(remotePeerAddress.sin_port) << std::endl;
	}
	size_t r = sendto(mSocket.socket, buffer.c_str(), size, 0, (struct sockaddr *) &remotePeerAddress, sizeof(remotePeerAddress));
	return r;
}

void UDPEmitter::setLastRemoteAddress(
	struct sockaddr_in *value
	) {
	memmove(&remotePeerAddress, value, sizeof(struct sockaddr_in));
}
