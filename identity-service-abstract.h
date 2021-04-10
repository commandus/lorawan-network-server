#ifndef IDENTITY_SERVICE_H_
#define IDENTITY_SERVICE_H_ 1

#include <map>
#include "utillora.h"

/**
 * Identity service interface
 * Get device identifier and keys by the network address
 */ 

class IdentityService {
	public:
		// Return 0, retval = EUI and keys
		virtual int get(DEVADDR &devaddr, DeviceId &retval) = 0;
		// Add or replace Address = EUI and keys pair
		virtual void put(DEVADDR &devaddr, DEVICEID &id) = 0;
		// Remove entry
		virtual void rm(DEVADDR &addr) = 0;
		// List entries
		void list(std::vector<NetworkIdentity> &retval, size_t offset, size_t size);
		// force save
		virtual void flush() = 0;
		// reload
		virtual int init(const std::string &option, void *data) = 0;
		// close resources
		virtual void done() = 0;
		// validate list
		virtual bool isValid(const std::vector<TDEVEUI> &ids) = 0;
};

#endif
