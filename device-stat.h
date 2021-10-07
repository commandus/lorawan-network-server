#ifndef DEVICE_STAT_H_
#define DEVICE_STAT_H_	1

#include <time.h>
#include <inttypes.h>
#include <netinet/in.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

#include "platform.h"
#include "utillora.h"

typedef ALIGN struct {
	// uint32_t addr;		///< network address
	time_t t;				///< last session time
	uint32_t fcntup;		///< Last frame count sent by end device
	uint32_t fcntdown;		///< Last frame count sent by network service
} PACKED DEVICESTAT;		// 4 bytes

/** 
 * Device stat keep FCntUp and FCntDown counter values.
 * Gateway identified by the network address.
 */
class DeviceStat {
public:
	uint32_t addr;		///< network address
	time_t t;				///< last session time
	uint32_t fcntup;		///< Last frame count sent by end device
	uint32_t fcntdown;		///< Last frame count sent by network service

	DeviceStat();
	DeviceStat(const DeviceStat &value);

	DeviceStat(const uint32_t &addr, const DEVICESTAT &value);
	void set(const uint32_t &addr, const DEVICESTAT &value);
	bool operator==(DeviceStat &rhs) const;

	void toJSON(rapidjson::Value &value, rapidjson::Document::AllocatorType& allocator) const;
	int parse(
		rapidjson::Value &value
	);
	std::string toJsonString() const;
	std::string toString() const;
};

#endif
