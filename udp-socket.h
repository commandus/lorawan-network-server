#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class UDPSocket {
  public:
    int socket;
    struct addrinfo addr;
    struct sockaddr addrStorage;
    UDPSocket();
    UDPSocket(const UDPSocket &value);
    std::string toString();
};
