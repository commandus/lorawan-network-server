#ifndef PACKET_HANDLER_ABSTRACT_H_
#define PACKET_HANDLER_ABSTRACT_H_ 1

#ifdef _MSC_VER
#else
#include <sys/time.h>
#endif

#include "utillora.h"

/**
 * Packet handler interface
 */ 
class PacketHandler {
	public:
		// Return 0, retval = EUI and keys
		virtual int enqueuePayload(
                const struct timeval &time,
                SemtechUDPPacket &value
		) = 0;

		virtual int enqueueMAC(
                const struct timeval &time,
                SemtechUDPPacket &value
		) = 0;
};

#endif
