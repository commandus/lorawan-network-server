#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H 1

#include <string>

#ifdef _MSC_VER
#include <WinSock2.h>
#else
#define SOCKET int
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


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
    SOCKET sock;       // socket handle
    int errcode;    // internal error code
    int lasterrno;  // last system errno
    struct addrinfo addr;
    struct sockaddr addrStorage;
    void closeSocket();
    UDPSocket();
    UDPSocket(const UDPSocket &value);
    UDPSocket(const std::string &address, MODE_OPEN_SOCKET mode, MODE_FAMILY familyHint);
    ~UDPSocket();
    int reOpenSocket(const std::string &address,
        MODE_OPEN_SOCKET mode,
        MODE_FAMILY familyHint
    );
    std::string toString() const;
    static std::string addrString(const struct sockaddr *addr);
    /**
     * Trying parseRX I v6 address, then IPv4
     * @param retval return address into struct sockaddr_in6 struct pointer
     * @param value IPv8 or IPv4 address string
     * @return true if success
     */
    static bool string2addr(struct sockaddr *retval, const std::string &value);
    int recv(void *retbuf, size_t size, void *peerAddress) const;
    bool isIPv6() const;
    bool isPeerAddr(struct sockaddr *remotePeerAddr) const;
};

#endif
