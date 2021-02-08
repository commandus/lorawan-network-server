#include "utillora.h"

class IdentityService {
	public:
		virtual int get(DeviceId &retval, DEVADDR &devaddr) = 0;
		virtual void put(DeviceId &id, DEVADDR &devaddr) = 0;
		virtual void rm(DeviceId &id) = 0;
		virtual int init(const std::string &option) = 0;
		virtual void done() = 0;
};
