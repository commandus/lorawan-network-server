#ifndef EMBEDDED_LISTENER_H
#define EMBEDDED_LISTENER_H 1

#include <string>
#include "packet-listener.h"
#include "utillora.h"
#include "lora-packet-handler-abstract.h"
#include "identity-service.h"
#include "device-history-service-abstract.h"
#include "gateway-list.h"
#include "lora-gateway-listener.h"

/**
 * Listen SPI connected gateway
 * TODO not implemented yet
 */
class EmbeddedListener : public PacketListener
{
public:
    LoraGatewayListener listener;
    EmbeddedListener();
	~EmbeddedListener();

	std::string toString() const override;

	void clear() override;
    bool add(const std::string &value, int hint) override;
	int listen(void *config) override;
};

#endif
