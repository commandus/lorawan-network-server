#ifndef DEVICE_HISTORY_SERVICE_ABSTRACT_H_
#define DEVICE_HISTORY_SERVICE_ABSTRACT_H_ 1

#include <map>
#include "device-history-item.h"
#include "utillora.h"

/**
 * Device counters and last received time service interface
 * Access by the network address
 */ 
class DeviceHistoryService {
	public:
        virtual ~DeviceHistoryService() {};
		// Return 0, retval = DeviceHistoryItem and keys
		virtual int get(DEVADDR &devaddr, DeviceHistoryItem &retval) = 0;
		virtual void put(DEVADDR &devaddr, DEVICE_HISTORY_ITEM &value) = 0;
		// Add or replace Address = FCntUo
		virtual void putUp(DEVADDR &devaddr, const time_t &time, uint32_t fcntup) = 0;
		// Add or replace Address = FCntDown
		virtual void putDown(DEVADDR &devaddr, const time_t &time, uint32_t fcntdown) = 0;
		// increment downstream from network server to the device
		virtual uint32_t incrementDown(const DEVADDR &devaddr, const time_t &time) = 0;
		// Remove entry
		virtual void rm(DEVADDR &addr) = 0;
		// List entries
		void list(std::vector<DeviceHistoryItem> &retval, size_t offset, size_t size);
		// force save
		virtual void flush() = 0;
		// reload
		virtual int init(const std::string &option, void *data) = 0;
		// close resources
		virtual void done() = 0;
};

#endif
