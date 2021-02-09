#include <map>
#include "identity-service-abstract.h"
#include "rapidjson/document.h"

class JsonFileIdentityService: public IdentityService {
	private:
		std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare> storage;
		std::string filename;
		void clear();
		int load();
		int save();
	public:
		JsonFileIdentityService();
		~JsonFileIdentityService();
		int get(DEVADDR &devaddr, DeviceId &retval);
		void put(DEVADDR &devaddr, DEVICEID &id);
		void rm(DEVADDR &addr);
		
		int init(const std::string &option, void *data);
		void flush();
		void done();
};
