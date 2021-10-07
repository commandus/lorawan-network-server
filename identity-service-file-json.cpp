#include <fstream>
#include <regex>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#pragma clang diagnostic pop
 
#include "identity-service-file-json.h"
#include "utilstring.h"
#include "errlist.h"

/**
 * 	JSON attribute names
 */
#define ATTRS_COUNT	8
static const char *ATTR_NAMES[ATTRS_COUNT] = {
	"addr", 		// 0 network address (hex string, 4 bytes)
	"activation",	// 1 ABP or OTAA
	"eui",			// 2 device identifier (hex string, 8 bytes)
	"nwkSKey",		// 3 shared session key (hex string, 16 bytes)
	"appSKey",		// 4 private key (hex string, 16 bytes)
	"class", 		// 5 A, B or C
	"version",		// 6 LoraWAN version
	"name"			// 7 added for search
};

static const char *ACTIVATION_NAMES[2] = {
	"ABP",
	"OTAA"
};

static int getAttrByName(
	const char *name
)
{
	int r = -1;
	for (int i = 0; i < ATTRS_COUNT; i++) {
		if (strcmp(ATTR_NAMES[i], name) == 0)
			return i;
	}
	return r;
}

static ACTIVATION getActivationByName(
	const char *name
)
{
	for (int i = 0; i < 2; i++) {
		if (strcmp(ACTIVATION_NAMES[i], name) == 0)
			return (ACTIVATION) i;
	}
	// default ABP
	return ABP;
}

static std::string getActivationName(
	ACTIVATION value
)
{
	if (value > OTAA)
		value = ABP;
	return ACTIVATION_NAMES[value];
}

JsonFileIdentityService::JsonFileIdentityService() 
	: path(""), errcode(0), errmessage("")
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
			memset(&k, 0, sizeof(DEVADDR));
			memset(&v, 0, sizeof(DEVICEID));
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
			std::string s; 
			/*
			 * 0- addr 1- activation 2- eui 3- nwkSKey 4- appSKey 5- class 6- version, 7- name
			 */
			switch(idx) {
				case 0:
					string2DEVADDR(k, str);
					break;
				case 1:
					v.activation = getActivationByName(str);
					break;
				case 2:
					string2DEVEUI(v.deviceEUI, str);
					break;
				case 3:
					s = hex2string(str);
					string2KEY(v.nwkSKey, s);
					break;
				case 4:
					s = hex2string(str);
					string2KEY(v.appSKey, s);
					break;
				case 5:
					v.deviceclass = string2deviceclass(str);
					break;
				case 6:
					v.version = string2LORAWAN_VERSION(str);
					break;
				case 7:
					string2DEVICENAME(v.name, str);
					break;
				default:
					break;
			}
			return true;
		}
		bool StartObject() { 
			isNetworkIdentity = true;
			v.version.major = 1;
			v.version.minor = 0;
			v.version.release = 0;
			return true; 
		}

		bool Key(const char* str, rapidjson::SizeType length, bool copy) { 
			idx = getAttrByName(str);
			return true;
		}
		bool EndObject(rapidjson::SizeType memberCount)
		{
			isNetworkIdentity = false;
			if (*((uint64_t *) &k))
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
	FILE* fp = fopen(path.c_str(), "rb");
	if (!fp)
		return ERR_CODE_INVALID_JSON;
 	char readBuffer[4096];
	rapidjson::FileReadStream istrm(fp, readBuffer, sizeof(readBuffer));
    rapidjson::ParseResult r = reader.Parse(istrm, handler);
	if (r.IsError()) {
		errcode = r.Code();
		std::stringstream ss;
		ss << rapidjson::GetParseError_En(r.Code()) << " at " << r.Offset();
		errmessage = ss.str();
	} else {
		errcode = 0;
		errmessage = "";
	}
	fclose(fp);
	return r.IsError() ? ERR_CODE_INVALID_JSON : 0;
} 

int JsonFileIdentityService::save()
{
	std::fstream os;
	os.open(path, std::ios::out);
	os << "[" << std::endl;
	bool addSeparator(false);
	for (std::map<DEVADDRINT, DEVICEID>::const_iterator it = storage.begin(); it != storage.end(); it++) {
		if (addSeparator)
			os << ",";
		os << std::endl << "{\"" 
			<< ATTR_NAMES[0] << "\": \"" << DEVADDRINT2string(it->first) << "\",\"" 
			<< ATTR_NAMES[1] << "\": \"" << getActivationName(it->second.activation) << "\",\"" 
			<< ATTR_NAMES[2] << "\": \"" << DEVEUI2string(it->second.deviceEUI) << "\",\""
			<< ATTR_NAMES[3] << "\": \"" << KEY2string(it->second.nwkSKey) << "\",\""
			<< ATTR_NAMES[4] << "\": \"" << KEY2string(it->second.appSKey) << "\",\""
			<< ATTR_NAMES[5] << "\": \"" << deviceclass2string(it->second.deviceclass) << "\",\""
			<< ATTR_NAMES[6] << "\": \"" << LORAWAN_VERSION2string(it->second.version) << "\",\""
			<< ATTR_NAMES[7] << "\": \"" << DEVICENAME2string(it->second.name) << "\"}";

		addSeparator = true;
	}
	os << "]" << std::endl;
	int r = os.bad() ? ERR_CODE_OPEN_DEVICE : 0;
	os.close();
	return r;
}

int JsonFileIdentityService::get(
	DEVADDR &devaddr,
	DeviceId &retval
) 
{
	mutexMap.lock();
	std::map<DEVADDRINT, DEVICEID>::const_iterator it(storage.find(DEVADDRINT(devaddr)));
    if (it == storage.end()) {
		mutexMap.unlock();
		return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
	}
	retval.set(it->second);
	mutexMap.unlock();
	return 0;
}

int JsonFileIdentityService::getNetworkIdentity(
	NetworkIdentity &retval,
	const DEVEUI &eui
) {
	mutexMap.lock();
	for (std::map<DEVADDRINT, DEVICEID>::const_iterator it(storage.begin()); it != storage.end(); it++) {
		if (memcmp(it->second.deviceEUI, eui, sizeof(DEVEUI)) == 0) {
			retval.set(it->first, it->second);
			mutexMap.unlock();
			return 0;
		}
	}
	mutexMap.unlock();
	return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
}

// List entries
void JsonFileIdentityService::list(
	std::vector<NetworkIdentity> &retval,
	size_t offset,
	size_t size
) {
	int64_t c = -1;
	if (size == 0)
		size = UINT64_MAX;
	for (std::map<DEVADDRINT, DEVICEID>::const_iterator it(storage.begin()); it != storage.end(); it++) {
		c++;
		if (c < offset)
			continue;
		if (c >= size)
			break;
		NetworkIdentity v(it->first, it->second);
		retval.push_back(v);
	}
}

void JsonFileIdentityService::put(
	DEVADDR &devaddr,
	DEVICEID &id
)
{
	mutexMap.lock();
	storage[devaddr] = id;
	mutexMap.unlock();
}

void JsonFileIdentityService::rm(
	DEVADDR &addr
)
{
	mutexMap.lock();
	storage.erase(addr);
	mutexMap.unlock();
}

int JsonFileIdentityService::init(
	const std::string &option, 
	void *data
)
{
	path = option;
	return load();
}

void JsonFileIdentityService::flush()
{
	save();
}

void JsonFileIdentityService::done()
{

}

int JsonFileIdentityService::parseIdentifiers(
	std::vector<TDEVEUI> &retval,
	const std::vector<std::string> &list,
	bool useRegex
) {
	for (std::vector<std::string>::const_iterator it(list.begin()); it != list.end(); it++) {
		if (isHex(*it)) {
			// identifier itself
			TDEVEUI v(*it);
			for (std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
				if (memcmp(v.eui, dit->second.deviceEUI, sizeof(DEVEUI)) == 0) {
					retval.push_back(v);
					break;
				}
			}
		} else {
			// can contain regex "*"
			try {
				std::string re;
				if (useRegex)
					re = *it;
				else
					re = replaceAll(replaceAll(*it, "*", ".*"), "?", ".");
				std::regex rex(re, std::regex_constants::grep);
				for (std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
					std::string s2 = DEVEUI2string(dit->second.deviceEUI);
					if (std::regex_search(s2, rex))
						retval.push_back(TDEVEUI(dit->second.deviceEUI));
				}
		    }
    		catch (const std::regex_error& e) {
				return ERR_CODE_INVALID_REGEX;
			}
		}
	}
	return 0;
}

int JsonFileIdentityService::parseNames(
	std::vector<TDEVEUI> &retval,
	const std::vector<std::string> &list,
	bool useRegex
) {
	for (std::vector<std::string>::const_iterator it(list.begin()); it != list.end(); it++) {
		try {
			std::string re;
			if (useRegex)
				re = *it;
			else
				re = replaceAll(replaceAll(*it, "*", ".*"), "?", ".");
			std::regex rex(re, std::regex_constants::grep);
			for (std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
				std::string s2 = DEVICENAME2string(dit->second.name);
				if (std::regex_search(s2, rex))
					retval.push_back(TDEVEUI(dit->second.deviceEUI));
			}
		}
		catch (const std::regex_error& e) {
			return ERR_CODE_INVALID_REGEX;
		}
	}
	return 0;
}

std::string JsonFileIdentityService::toJsonString()
{
	std::stringstream ss;
	ss << "[";
	bool needComma = false;
	for (std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
		if (needComma)
			ss << ", ";
		else
			needComma = true;
		ss << "{"
			<< "\"" << ATTR_NAMES[0] << "\":\"" << DEVADDRINT2string(dit->first) << "\", "
			<< "\"" << ATTR_NAMES[1] << "\":\"" << getActivationName(dit->second.activation) << "\", "
			<< "\"" << ATTR_NAMES[2] << "\":\"" << DEVEUI2string(dit->second.deviceEUI) << "\", "
			<< "\"" << ATTR_NAMES[3] << "\":\"" << KEY2string(dit->second.nwkSKey) << "\", "
			<< "\"" << ATTR_NAMES[4] << "\":\"" << KEY2string(dit->second.appSKey) << "\", "
			<< "\"" << ATTR_NAMES[5] << "\":\"" << deviceclass2string(dit->second.deviceclass) << "\", "
			<< "\"" << ATTR_NAMES[6] << "\":\"" << LORAWAN_VERSION2string(dit->second.version) << "\", "
			<< "\"" << ATTR_NAMES[7] << "\":\"" << DEVICENAME2string(dit->second.name) << "\"}";
	}
	ss << "]";
	return ss.str();
}
