#include <iostream>
#include <fstream>

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
 
#include "identity-service-file-json.h"
#include "utilstring.h"
#include "errlist.h"

/**
 * 	Section 3.3	 
 */
const char* ATTR_NAMES[4] = {
	"addr", 	// network address (hex string, 4 bytes)
	"eui",		// device identifier (hex string, 8 bytes)
	"nwkSKey",	// shared session key (hex string, 16 bytes)
	"appSKey"	// private key (hex string, 16 bytes)
};

int getAttrName(
	const char *name
)
{
	int r = -1;
	for (int i = 0; i < 4; i++) {
		if (strcmp(ATTR_NAMES[i], name) == 0)
			return i;
	}
	return r;
}

JsonFileIdentityService::JsonFileIdentityService() 
	: filename("")
{

}

JsonFileIdentityService::~JsonFileIdentityService() 
{
	done();
}

/**
 * 
 * Loads NetworkIdentities
 *	[
 *		{
 *	 		"addr": "network address (hex string, 4 bytes)"
 * 			"eui": "device identifier (hex string, 8 bytes)",
 * 			"nwkSKey": "shared session key (hex string, 16 bytes)",
 *			"appSKey": "private key (hex string, 16 bytes)"
 *		},
 *		..
 *	]
 */ 
class IdentityJsonHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, IdentityJsonHandler> {
	private:
		JsonFileIdentityService *service;
		bool isNetworkIdentity;
		int idx;
		DEVADDR k;
		DEVICEID v;
	public:
		IdentityJsonHandler(JsonFileIdentityService *svc)
			: service(svc), isNetworkIdentity(false), idx(-1)
		{

		}

		bool Null() {
			return true; 
		}

		bool Bool(bool b) {
			return true;
		}

		bool Int(int i) {
			return true;
		}

		bool Uint(unsigned u) {
			return true;
		}

		bool Int64(int64_t i) {
			return true;
		}

		bool Uint64(uint64_t u) {
			return true;
		}

		bool Double(double d) {
			return true;
		}

		bool String(const char* str, rapidjson::SizeType length, bool copy) { 
			std::string s = hex2string(str); 
			switch(idx) {
				case 0:
					string2DEVADDR(k, s);
					break;
				case 1:
					string2DEVEUI(v.deviceEUI, s);
					break;
				case 2:
					string2KEY(v.nwkSKey, s);
					break;
				case 3:
					string2KEY(v.appSKey, s);
					break;
				default:
					break;
			}
			return true;
		}
		bool StartObject() { 
			isNetworkIdentity = true;
			return true; 
		}

		bool Key(const char* str, rapidjson::SizeType length, bool copy) { 
			idx = getAttrName(str);
			return true;
		}
		bool EndObject(rapidjson::SizeType memberCount)
		{
			isNetworkIdentity = false;
			service->put(k, v);
			return true;
		}

		bool StartArray() {
			return true; 
		}

		bool EndArray(rapidjson::SizeType elementCount) { 
			return true; 
		}
};

void JsonFileIdentityService::clear()
{
	storage.clear();
}

int JsonFileIdentityService::load()
{
	clear();
    IdentityJsonHandler handler(this);
    rapidjson::Reader reader;
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp)
		return ERR_CODE_INVALID_JSON;
 	char readBuffer[4096];
	rapidjson::FileReadStream istrm(fp, readBuffer, sizeof(readBuffer));
    reader.Parse(istrm, handler);
	fclose(fp);
} 

int JsonFileIdentityService::save()
{
	std::fstream os;
	os.open(filename, std::ios::out);
	os << "[";
	bool addSeparator(false);
	for (std::map<DEVADDR, DEVICEID>::const_iterator it = storage.begin(); it != storage.end(); it++) {
		if (addSeparator)
			os << ",";
		os << "{\"" 
			<< ATTR_NAMES[0] << "\":\"" << DEVADDR2string(it->first) << "\",\"" 
			<< ATTR_NAMES[1] << "\":\"" << DEVEUI2string(it->second.deviceEUI) << "\",\""
			<< ATTR_NAMES[2] << "\":\"" << KEY2string(it->second.nwkSKey) << "\",\""
			<< ATTR_NAMES[3] << "\":\"" << KEY2string(it->second.appSKey) << "\"}";

		addSeparator = true;
	}
	os << "]";
	os.close();
}

int JsonFileIdentityService::get(
	DEVADDR &devaddr,
	DeviceId &retval
) 
{
	int r = 0;
	std::map<DEVADDR, DEVICEID>::const_iterator it(storage.find(devaddr));
    if (it == storage.end())
		return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
	retval.set(it->second);
	return r;
}

void JsonFileIdentityService::put(
	DEVADDR &devaddr,
	DEVICEID &id
)
{
	//storage[devaddr] = id;
}

void JsonFileIdentityService::rm(
	DEVADDR &addr
)
{
	storage.erase(addr);
}

int JsonFileIdentityService::init(
	const std::string &option, 
	void *data
)
{
	filename = option;
	load();
}

void JsonFileIdentityService::flush()
{
	save();
}

void JsonFileIdentityService::done()
{

}
