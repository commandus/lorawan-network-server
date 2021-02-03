#include <sstream>
#include <cstring>
#include <unistd.h>

#include "udp-socket.h"
#include "errlist.h"

UDPSocket::UDPSocket() 
      : sock(0), errcode(0), lasterrno(0)
{
	memset(&addr, 0, sizeof(struct addrinfo));
	memset(&addrStorage, 0, sizeof(struct sockaddr));
}

/**
 * Copy constructor prevents re-open socket on copy
 */ 
UDPSocket::UDPSocket(const UDPSocket &value) 
    : sock(value.sock), errcode(value.errcode), lasterrno(value.lasterrno)
{
	memmove(&addrStorage, value.addr.ai_addr, sizeof(struct sockaddr));
	memmove(&addr, &value.addr, sizeof(struct addrinfo));
	addr.ai_addr = &addrStorage;
}

/**
 * 
 **/
UDPSocket::~UDPSocket() 
{
	// Do not close socket becausa I copy socket by assignment UDPSocket to the std::vector
	// closeSocket();
}

void UDPSocket::closeSocket() {
	if (sock > 0) {
		close(sock);
		sock = 0;
	}
}

std::string UDPSocket::toString() const {
	char buf[INET6_ADDRSTRLEN];
	int port;
	if (addr.ai_family == AF_INET) {
		if (inet_ntop(AF_INET, &(((struct sockaddr_in *) addr.ai_addr)->sin_addr), buf, sizeof(buf)) == NULL)
			return "";
		port = ntohs(((struct sockaddr_in *) addr.ai_addr)->sin_port);
	} else {
		if (addr.ai_family == AF_INET6) {
			if (inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) addr.ai_addr)->sin6_addr), buf, sizeof(buf)) == NULL) {
				return "";
			}
		}
		port = ntohs(((struct sockaddr_in6 *) addr.ai_addr)->sin6_port);
	}
	std::stringstream ss;
	ss << sock << " " << buf << ":" << port;
	return ss.str();
}

bool UDPSocket::isIPv6() const {
	return addr.ai_family == AF_INET6;
}

static int addressFamily(
	struct addrinfo *retAddrInfo,
	struct sockaddr *addrStorage,
	std::string &addr,
	int port,
	MODE_FAMILY familyHint
) {
	if (addr.empty() || (addr.length() == 1 && addr[0] == '*') ) {
		memset(addrStorage, 0, sizeof(struct sockaddr));
		addrStorage->sa_family = (familyHint == MODE_FAMILY_HINT_IPV6) ? AF_INET6 : AF_INET;
		if (addrStorage->sa_family == AF_INET) {
			struct sockaddr_in *a = (struct sockaddr_in *) addrStorage;
			a->sin_addr.s_addr = INADDR_ANY;
			a->sin_port = htons(port);
			
			retAddrInfo->ai_addr = (struct sockaddr *) addrStorage;
			retAddrInfo->ai_family = AF_INET;
			retAddrInfo->ai_addrlen = sizeof(struct sockaddr_in);
		} else {
			struct sockaddr_in6 *a = (struct sockaddr_in6 *) addrStorage;
			a->sin6_addr = in6addr_any;
			a->sin6_port = htons(port);
			
			retAddrInfo->ai_addr = (struct sockaddr *) addrStorage;
			retAddrInfo->ai_family = AF_INET6;
			retAddrInfo->ai_addrlen = sizeof(struct sockaddr_in6);
		}
		return addrStorage->sa_family;
	}
	struct addrinfo hint;
    memset(&hint, 0, sizeof hint);
	
	hint.ai_family = familyHint;
    // hint.ai_flags = AI_NUMERICHOST;
	struct addrinfo *res = NULL;
    int r = getaddrinfo(addr.c_str(), NULL, &hint, &res);
    if (r)
        return ERR_CODE_INVALID_ADDRESS;
    r = res->ai_family;
	memmove(addrStorage, res->ai_addr, res->ai_addrlen);
	memmove(retAddrInfo, res, sizeof(struct addrinfo));
	retAddrInfo->ai_addr = addrStorage;
	if (r == AF_INET) {
		struct sockaddr_in *a = (struct sockaddr_in *) addrStorage;
		a->sin_port = htons(port);

		retAddrInfo->ai_family = AF_INET;
		retAddrInfo->ai_addrlen = sizeof(struct sockaddr_in);

	} else {
		if (r == AF_INET6) {
			struct sockaddr_in6 *a = (struct sockaddr_in6 *) addrStorage;
			a->sin6_port = htons(port);
		
			retAddrInfo->ai_family = AF_INET6;
			retAddrInfo->ai_addrlen = sizeof(struct sockaddr_in6);
		}
	}
   	freeaddrinfo(res);
   	return r;
}
/**
 * ADRESS:PORT
 */ 
static bool splitAddress(
	std::string &retAddress,
	int &retPort,
	const std::string &address
)
{
	size_t pos = address.find_last_of(":");
	if (pos == std::string::npos)
		return false;
	retAddress = address.substr(0, pos);
	std::string p(address.substr(pos + 1));
	retPort = atoi(p.c_str());
	return true;
}

UDPSocket::UDPSocket(
	const std::string &address,
	MODE_OPEN_SOCKET mode,
	MODE_FAMILY familyHint
) 
	: sock(0), errcode(0), lasterrno(0)
{
	std::string a;
	int p;
	if (!splitAddress(a, p, address)) {
		errcode = ERR_CODE_INVALID_ADDRESS;
		return;
	}
	int f = addressFamily(&addr, &addrStorage, a, p, familyHint);
	if (f < 0) {
		errcode = f;
		return;
	}

	if (!((f == AF_INET) || (f == AF_INET6))) {
		errcode = ERR_CODE_INVALID_FAMILY;
		return;
	}

	if (mode == MODE_OPEN_SOCKET_LISTEN) {
		// bind
		if (f == AF_INET6) {
			sock = socket(PF_INET6, SOCK_DGRAM, 0);
			if (sock < 0) {
				errcode = ERR_CODE_SOCKET_CREATE;
				lasterrno = errno;
				return;
			}
			if (bind(sock, (struct sockaddr *) &addrStorage, addr.ai_addrlen) < 0) {
				errcode = ERR_CODE_SOCKET_BIND;
				lasterrno = errno;
				return;
			}
		} else {
			if (f == AF_INET) {
				sock = socket(PF_INET, SOCK_DGRAM, 0);
				if (sock < 0) {
					errcode = ERR_CODE_SOCKET_CREATE;
					lasterrno = errno;
					return;
				}
				if (bind(sock, (struct sockaddr *) &addrStorage, addr.ai_addrlen) < 0) {
					errcode = ERR_CODE_SOCKET_BIND;
					lasterrno = errno;
					return;
				}
			} 
		}
	}
}

int UDPSocket::recv(
	void *retbuf,
	size_t size,
	void *peerAddress
) const {
	socklen_t sz = (addr.ai_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
	return recvfrom(sock, retbuf, size, 0, (struct sockaddr *) peerAddress, &sz);
}

bool UDPSocket::isPeerAddr(
	struct sockaddr *addr
) const {
	if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *remotePeerAddr = (struct sockaddr_in6 *) addr;
		struct sockaddr_in6 *s = (struct sockaddr_in6 *) &addrStorage;
		return (s->sin6_family == remotePeerAddr->sin6_family && 
			memcmp(&s->sin6_addr, &remotePeerAddr->sin6_addr, sizeof(struct in_addr)) == 0 && 
			s->sin6_port == remotePeerAddr->sin6_port
		);
	} else {
		struct sockaddr_in *remotePeerAddr = (struct sockaddr_in *) addr;
		struct sockaddr_in *s = (struct sockaddr_in *) &addrStorage;
		return (s->sin_family == remotePeerAddr->sin_family && 
			memcmp(&s->sin_addr, &remotePeerAddr->sin_addr, sizeof(struct in_addr)) == 0 && 
			s->sin_port == remotePeerAddr->sin_port
		);
	}
}
