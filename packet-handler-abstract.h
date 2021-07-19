#ifndef PACKET_HANDLER_ABSTRACT_H_
#define PACKET_HANDLER_ABSTRACT_H_ 1

#include <sys/time.h>
#include "utillora.h"

/**
 * Packet handler interface
 */ 
class PacketHandler {
	public:
		// Return 0, retval = EUI and keys
		virtual int onPacket(
			struct timeval &time,
			const DEVADDR &addr,
			DeviceId id,
			semtechUDPPacket &value
		) = 0;
};

#endif
