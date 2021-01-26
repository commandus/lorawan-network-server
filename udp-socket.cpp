#include <sstream>
#include <cstring>

#include "udp-socket.h"
#include "errlist.h"

UDPSocket::UDPSocket() 
      : socket(0)
{
	memset(&addr, 0, sizeof(struct addrinfo));
	memset(&addrStorage, 0, sizeof(struct sockaddr));
}

UDPSocket::UDPSocket(const UDPSocket &value) 
    : socket(value.socket)
{
	memmove(&addrStorage, value.addr.ai_addr, sizeof(struct sockaddr));
	memmove(&addr, &value.addr, sizeof(struct addrinfo));
	addr.ai_addr = &addrStorage;
}

std::string UDPSocket::toString() {
	std::stringstream ss;
	ss << socket << " " << inet_ntoa(((struct sockaddr_in *) addr.ai_addr)->sin_addr) 
	<< ":" << ntohs(((struct sockaddr_in *) addr.ai_addr)->sin_port);
	return ss.str();
}
