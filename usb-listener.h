#ifndef USB_LISTENER_H
#define USB_LISTENER_H 1

#include <string>
#include <vector>
#include <functional>

#include "packet-listener.h"
#include "lora-packet-handler-abstract.h"
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

    USBListener();
	~USBListener();

	std::string toString() const override;

	void clear() override;
    bool add(const std::string &value, int hint) override;
    /**
     * Start listening
     * @param config GatewaySettings
     * @return 0- success
     */
	int listen(
		const std::string &regionName,
		int regionIndex,
		void *config,
		int flags,
		ThreadStartFinish *threadStartFinish
	) override;
};

#endif
