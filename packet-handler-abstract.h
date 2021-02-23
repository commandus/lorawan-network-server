#ifndef PACKET_HANDLER_ABSTRACT_H_
#define PACKET_HANDLER_ABSTRACT_H_ 1

#include "utillora.h"

/**
 * Packet handler interface
 */ 
class PacketHandler {
	public:
		// Return 0, retval = EUI and keys
		virtual int onPacket(semtechUDPPacket &value) = 0;
};

#endif
