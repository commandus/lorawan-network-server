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
		virtual int getNetworkIdentity(NetworkIdentity &retval, const DEVEUI &eui) = 0;
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
		// parse list of identifiers, wildcards or regexes and copy found EUI into retval
		virtual int parseIdentifiers(
			std::vector<TDEVEUI> &retval,
			const std::vector<std::string> &list,
			bool useRegex
		) = 0;
		// parse list of names, wildcards or regexes and copy found EUI into retval
		virtual int parseNames(
			std::vector<TDEVEUI> &retval,
			const std::vector<std::string> &list,
			bool useRegex
		) = 0;
};

#endif
