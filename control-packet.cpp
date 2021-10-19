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
	memmove(p, eui, sizeof(CONTROL_DEVICE_HEADER));
	p += sizeof(CONTROL_DEVICE_HEADER);
	memmove(p, &gatewayId, sizeof(uint64_t));
	p += sizeof(uint64_t);
	memmove(p, payload.c_str(), payload.size());
	return s;
}
