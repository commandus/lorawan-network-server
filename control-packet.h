/*
 * control-packet.h
 */
#ifndef CONTROL_PACKET_H
#define CONTROL_PACKET_H     1

#include <string>
#include "platform.h"
#include "utillora.h"

typedef ALIGN struct {
	DEVEUI eui;						// device identifier
	uint64_t gwid;					// gateway identifer
	uint8_t tag;					// 0- MAC commands
} PACKED CONTROL_DEVICE_HEADER;		// 17 bytes

typedef ALIGN struct {
	CONTROL_DEVICE_HEADER header;	// 17 bytes long header 
	uint8_t data[1];				// at least 1 byte
} PACKED CONTROL_DEVICE_PACKET;		// 16 bytes

std::string mkControlPacket
(
	const DEVEUI &eui,
	const uint64_t gatewayId,
	const std::string &payload
);

#endif
