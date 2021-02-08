#include "identity-service-abstract.h"

class JsonFileIdentityService: public IdentityService {
	public:
		JsonFileIdentityService();
		~JsonFileIdentityService();
		int get(DeviceId &retval, DEVADDR &devaddr);
};
