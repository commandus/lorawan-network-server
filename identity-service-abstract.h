#include "utillora.h"

class IdentityService {
	public:
		virtual int get(DEVADDR &devaddr, DeviceId &retval) = 0;
		virtual void put(DEVADDR &devaddr, DEVICEID &id) = 0;
		virtual void rm(DEVADDR &addr) = 0;
		virtual void flush() = 0;
		virtual int init(const std::string &option, void *data) = 0;
		virtual void done() = 0;
};
