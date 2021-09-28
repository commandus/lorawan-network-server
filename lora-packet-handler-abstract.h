#ifndef LORA_PACKET_HANDLER_ABSTRACT_H
#define LORA_PACKET_HANDLER_ABSTRACT_H 1

#include <sys/time.h>
#include "utillora.h"

/**
 * Handle uplink messages interface
 */ 
class LoraPacketHandler {
	public:
		// immediately send ACK
		virtual int ack(
			int socket,
			const sockaddr_in* gwAddress,
			const SEMTECH_PREFIX_GW &dataprefix
		) = 0;
		// Return 0, retval = EUI and keys
		virtual int put(
			struct timeval &time,
			semtechUDPPacket &packet
		) = 0;
};

#endif
