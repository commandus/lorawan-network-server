#ifndef USB_LISTENER_H
#define USB_LISTENER_H 1

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
#include "lora-gateway-listener.h"

/**
 * Listen embedded USB gateway @see https://docs.rakwireless.com/Product-Categories/WisLink/RAK2287/Datasheet/#specifications
 * Devices are identified by IdentityService.
 * Identified packets with application payload with optional MAC commands passed to the LoraPacketHandler.
 * LoraPacketHandler handles MAC commands in another thread, application payload pass further via queue.
 */
class USBListener : public PacketListener
{
public:
    LoraGatewayListener listener;   ///< USB gateway listener
    GatewayConfigFileJson config;   ///< gateway config

    USBListener();
	~USBListener();

	std::string toString() const override;

	void clear() override;
    bool add(const std::string &value, int hint) override;
	int listen() override;
};

#endif
