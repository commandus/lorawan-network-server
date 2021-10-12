#ifndef DEVICE_STAT_SERVICE_ABSTRACT_H_
#define DEVICE_STAT_SERVICE_ABSTRACT_H_ 1

#include <map>
#include "device-stat.h"
#include "utillora.h"

/**
 * Identity service interface
 * Get device statistics by the network address
 */ 
class DeviceStatService {
	public:
		// Return 0, retval = DeviceStat and keys
		virtual int get(DEVADDR &devaddr, DeviceStat &retval) = 0;
		virtual void put(DEVADDR &devaddr, DEVICESTAT &value) = 0;
		// Add or replace Address = FCntUo
		virtual void putUp(DEVADDR &devaddr, const time_t &time, uint32_t fcntup) = 0;
		// Add or replace Address = FCntDown
		virtual void putDown(DEVADDR &devaddr, const time_t &time, uint32_t fcntdown) = 0;
		// Remove entry
		virtual void rm(DEVADDR &addr) = 0;
		// List entries
		void list(std::vector<DeviceStat> &retval, size_t offset, size_t size);
		// force save
		virtual void flush() = 0;
		// reload
		virtual int init(const std::string &option, void *data) = 0;
		// close resources
		virtual void done() = 0;
};

#endif
