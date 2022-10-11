#ifndef EMBEDDED_LISTENER_H
#define EMBEDDED_LISTENER_H 1

#include <string>
#include "packet-listener.h"
#include "utillora.h"
#include "lora-packet-handler-abstract.h"
#include "identity-service.h"
#include "device-history-service-abstract.h"
#include "gateway-list.h"

/**
 * Listen Semtech's gateway directly
 */
class EmbeddedListener : public PacketListener
{
private:
    std::string buffer;
public:
    EmbeddedListener();
	~EmbeddedListener();

	std::string toString() const override;

	void clear() override;
    bool add(const std::string &value, int hint) override;
	int listen() override;

    void setBufferSize(size_t value) override;
};

#endif
