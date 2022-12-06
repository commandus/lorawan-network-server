#ifndef DEVICE_STAT_H_
#define DEVICE_STAT_H_	1

#include <time.h>
#include <inttypes.h>
#ifdef _MSC_VER
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/document.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "platform.h"
#include "utillora.h"

typedef PACK (struct {
	// uint32_t addr;		///< network address
	time_t t;				///< last session time
	uint32_t fcntup;		///< Last frame count sent by end device
	uint32_t fcntdown;		///< Last frame count sent by network service
} ) DEVICE_HISTORY_ITEM;		// 4 bytes

/** 
 * Device stat keep FCntUp and FCntDown counter bands.
 * Gateway identified by the network address.
 */
class DeviceHistoryItem {
public:
	uint32_t addr;		///< network address
	time_t t;				///< last session time
	uint32_t fcntup;		///< Last frame count sent by end device
	uint32_t fcntdown;		///< Last frame count sent by network service

	DeviceHistoryItem();
	DeviceHistoryItem(const DeviceHistoryItem &value);

	DeviceHistoryItem(const uint32_t &addr, const DEVICE_HISTORY_ITEM &value);
	void set(const uint32_t &addr, const DEVICE_HISTORY_ITEM &value);
	bool operator==(DeviceHistoryItem &rhs) const;

	void toJSON(rapidjson::Value &value, rapidjson::Document::AllocatorType& allocator) const;
	int parse(
		rapidjson::Value &value
	);
	std::string toJsonString() const;
	std::string toString() const;
};

#endif
