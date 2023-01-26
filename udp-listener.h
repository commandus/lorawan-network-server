#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H 1

#include <string>
#include <vector>
#include <functional>
#include "packet-listener.h"
#include "udp-socket.h"
#include "utillora.h"
#include "lora-packet-handler-abstract.h"
#include "identity-service.h"
#include "device-history-service-abstract.h"
#include "gateway-list.h"

/**
 * Listen UDP port(s) for packets sent by Semtech's gateway on interfaces with IPv4 or IPv6 address
 * from gateways listed in the GatewayList.
 * Gateway send PULL request with address and port on which gateway now listen, UDPListener 
 * save received address and port in the GatewayList.
 * Devices are identified by IdentityService.
 * Identified packets with application payload with optional MAC commands passed to the LoraPacketHandler.
 * LoraPacketHandler handles MAC commands in another thread, application payload pass further via queue.
 */
class UDPListener : public PacketListener
{
private:
    std::string buffer;
    int largestSocket();
    struct sockaddr_in6 remotePeerAddress;
    void setLastRemoteAddress(
        struct sockaddr *value
    );
    bool addSocket(
        const std::string &address,
        MODE_FAMILY familyHint
    );
protected:
    int parseBuffer(
        const std::string &buffer,
        size_t bytesReceived,
        int socket,
        const struct timeval &receivedTime,
        const struct sockaddr_in6 &gwAddress
    );
public:
	std::vector<UDPSocket> sockets;

	UDPListener();
	~UDPListener();

	std::string toString() const override;

	void clear() override;
    bool add(const std::string &value, int hint) override;
	int listen(void *config, int flags) override;
    void setBufferSize(size_t value);
};

#endif
