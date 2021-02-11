#include <map>
#include "identity-service-abstract.h"
#include "db-identity.h"

class LmdbIdentityService: public IdentityService {
	private:
		dbenv *env;
		std::string filename;
		void clear();
		int open();
		int close();
	public:
		LmdbIdentityService();
		~LmdbIdentityService();
		int get(DEVADDR &devaddr, DeviceId &retval);
		void put(DEVADDR &devaddr, DEVICEID &id);
		void rm(DEVADDR &addr);
		
		int init(const std::string &option, void *data);
		void flush();
		void done();
};
