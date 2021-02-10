#ifndef LORA_PACKET_HANDLER_ABSTRACT_H
#define LORA_PACKET_HANDLER_ABSTRACT_H 1

#include "utillora.h"

class LoraPacketHandler {
	public:
		// Return 0, retval = EUI and keys
		virtual int process(semtechUDPPacket &packet) = 0;
};

#endif
