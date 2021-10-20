/*
 * @file control-packet.cpp
 */
#include "control-packet.h"

std::string mkControlPacket
(
	const DEVEUI &eui,
	const uint64_t gatewayId,
	const std::string &payload
)
{
	std::string s;
	s.resize(sizeof(CONTROL_DEVICE_HEADER) + payload.size());
	char *p = (char *) s.c_str();
	memmove(p, eui, sizeof(DEVEUI));
	p += sizeof(DEVEUI);
	memmove(p, &gatewayId, sizeof(uint64_t));
	p += sizeof(uint64_t);
	uint8_t tag = 0;
	memmove(p, &tag, sizeof(uint8_t));
	p += sizeof(uint8_t);
	memmove(p, payload.c_str(), payload.size());
	return s;
}
