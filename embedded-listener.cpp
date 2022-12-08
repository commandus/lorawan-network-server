#include "embedded-listener.h"

#include "utilstring.h"
#include "errlist.h"

EmbeddedListener::EmbeddedListener() : PacketListener()
{
}

EmbeddedListener::~EmbeddedListener() {
	clear();
}

std::string EmbeddedListener::toString() const{
	return "";
}

void EmbeddedListener::clear() {
}

bool EmbeddedListener::add(
    const std::string& value,
    int hint
)
{
    return true;
}

int EmbeddedListener::listen(void *config)
{
    onLog->logMessage(this, LOG_INFO, LOG_UDP_LISTENER, 0, MSG_LISTEN_EMBEDDED_PCI);
	return LORA_OK;
}
