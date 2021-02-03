#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H 1

#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum MODE_OPEN_SOCKET {
  MODE_OPEN_SOCKET_LISTEN = 0,
  MODE_OPEN_SOCKET_CONNECT = 1
};

enum MODE_FAMILY {
  MODE_FAMILY_HINT_UNSPEC = AF_UNSPEC,
  MODE_FAMILY_HINT_IPV4 = AF_INET,
  MODE_FAMILY_HINT_IPV6 = AF_INET6
};

class UDPSocket {
  public:
    int sock;       // socket handle
    int errcode;    // internal error code
    int lasterrno;  // last system errno
    struct addrinfo addr;
    struct sockaddr addrStorage;
    void closeSocket();
    UDPSocket();
    UDPSocket(const UDPSocket &value);
    UDPSocket(const std::string &address, MODE_OPEN_SOCKET mode, MODE_FAMILY familyHint);
    ~UDPSocket();
    std::string toString() const;
    int recv(void *retbuf, size_t size, void *peerAddress) const;
    bool isIPv6() const;
    bool isPeerAddr(struct sockaddr *remotePeerAddr) const;
};

#endif
