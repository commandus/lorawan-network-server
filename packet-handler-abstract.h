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
		virtual int enqueuePayload(
			struct timeval &time,
			DeviceId id,
			semtechUDPPacket &value
		) = 0;

		virtual int enqueueMAC(
			struct timeval &time,
			DeviceId id,
			semtechUDPPacket &value
		) = 0;
};

#endif
