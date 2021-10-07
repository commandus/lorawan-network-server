#include <fstream>
#include <regex>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#pragma clang diagnostic pop
 
#include "device-stat-service-json.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"

/**
 * 	JSON attribute names
 */
#define ATTRS_COUNT	4
static const char *ATTR_NAMES[ATTRS_COUNT] = {
	"addr", 		// 0 network address (hex string, 4 bytes)
	"time",			// 1 last received packet time stamp
	"fcntup",		// 2 device up counter
	"fcntdown"		// 3 network service down counter
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

JsonFileDeviceStatService::JsonFileDeviceStatService() 
	: path(""), errcode(0), errmessage("")
{

}

JsonFileDeviceStatService::~JsonFileDeviceStatService() 
{
	done();
}

/**
 * 
 * Loads device statistics
 *	[
 *		{
 *	 		"addr": "network address (hex string, 4 bytes)"
 * 			"time": "time stamp",
 * 			"fcntup": number,
 *			"fcntdown": number
 *		},
 *		..
 *	]
 */ 
class DeviceStatJsonHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, DeviceStatJsonHandler> {
	private:
		JsonFileDeviceStatService *service;
		bool isNetworkIdentity;
		int idx;
		DEVADDR k;
		DEVICESTAT v;
	public:
		DeviceStatJsonHandler(JsonFileDeviceStatService *svc)
			: service(svc), isNetworkIdentity(false), idx(-1)
		{
			memset(&k, 0, sizeof(DEVADDR));
			memset(&v, 0, sizeof(DEVICESTAT));
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
			switch(idx) {
				case 2:
					v.fcntup = u;
					break;
				case 3:
					v.fcntup = u;
					break;
				default:
					break;

			}
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
			 * 0- addr 1- time 2- fcntup 3- fcntdown
			 */
			switch(idx) {
				case 0:
					string2DEVADDR(k, str);
					break;
				case 1:
					v.t = parseDate(str);
					break;
				default:
					break;
			}
			return true;
		}
		bool StartObject() { 
			v.t = 0;
			v.fcntdown = 0;
			v.fcntup = 0;
			return true; 
		}

		bool Key(const char* str, rapidjson::SizeType length, bool copy) { 
			idx = getAttrByName(str);
			return true;
		}
		bool EndObject(rapidjson::SizeType memberCount)
		{
			if (*((uint32_t *) &k))
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

void JsonFileDeviceStatService::clear()
{
	storage.clear();
}

int JsonFileDeviceStatService::load()
{
	clear();
    DeviceStatJsonHandler handler(this);
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

int JsonFileDeviceStatService::save()
{
	std::fstream os;
	os.open(path, std::ios::out);
	os << "[" << std::endl;
	bool addSeparator(false);
	for (std::map<DEVADDRINT, DEVICESTAT>::const_iterator it = storage.begin(); it != storage.end(); it++) {
		if (addSeparator)
			os << ",";
		os << std::endl << "{\"" 
			<< ATTR_NAMES[0] << "\": \"" << DEVADDRINT2string(it->first) << "\",\"" 
			<< ATTR_NAMES[1] << "\": \"" << time2string(it->second.t) << "\",\"" 
			<< ATTR_NAMES[2] << "\": " << it->second.fcntup << ",\""
			<< ATTR_NAMES[3] << "\": \"" << it->second.fcntdown << "}" << std::endl;
		addSeparator = true;
	}
	os << "]" << std::endl;
	int r = os.bad() ? ERR_CODE_OPEN_DEVICE : 0;
	os.close();
	return r;
}

int JsonFileDeviceStatService::get(
	DEVADDR &devaddr,
	DeviceStat &retval
) 
{
	mutexMap.lock();
	std::map<DEVADDRINT, DEVICESTAT>::const_iterator it(storage.find(DEVADDRINT(devaddr)));
    if (it == storage.end()) {
		mutexMap.unlock();
		return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
	}
	retval.set(it->first.a, it->second);
	mutexMap.unlock();
	return 0;
}

// List entries
void JsonFileDeviceStatService::list(
	std::vector<DeviceStat> &retval,
	size_t offset,
	size_t size
) {
	int64_t c = -1;
	if (size == 0)
		size = UINT64_MAX;
	for (std::map<DEVADDRINT, DEVICESTAT>::const_iterator it(storage.begin()); it != storage.end(); it++) {
		c++;
		if (c < offset)
			continue;
		if (c >= size)
			break;
		DeviceStat v(it->first.a, it->second);
		retval.push_back(v);
	}
}

void JsonFileDeviceStatService::put(
	DEVADDR &devaddr,
	DEVICESTAT &value
)
{
	mutexMap.lock();
	storage[devaddr] = value;
	mutexMap.unlock();
}

void JsonFileDeviceStatService::putUp(
	DEVADDR &devaddr,
	uint32_t &value
)
{
	mutexMap.lock();
	DEVICESTAT v = storage[devaddr];
	v.fcntup = value;
	storage[devaddr] = v;
	mutexMap.unlock();
}

void JsonFileDeviceStatService::putDown(
	DEVADDR &devaddr,
	uint32_t &value
)
{
	mutexMap.lock();
	DEVICESTAT v = storage[devaddr];
	v.fcntdown = value;
	storage[devaddr] = v;
	mutexMap.unlock();
}

void JsonFileDeviceStatService::rm(
	DEVADDR &addr
)
{
	mutexMap.lock();
	storage.erase(addr);
	mutexMap.unlock();
}

int JsonFileDeviceStatService::init(
	const std::string &option, 
	void *data
)
{
	path = option;
	return load();
}

void JsonFileDeviceStatService::flush()
{
	save();
}

void JsonFileDeviceStatService::done()
{

}

std::string JsonFileDeviceStatService::toJsonString()
{
	std::stringstream ss;
	ss << "[" << std::endl;;
	bool needComma = false;
	for (std::map<DEVADDRINT, DEVICESTAT, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
		if (needComma)
			ss << ", ";
		else
			needComma = true;
		ss << "{"
			<< "\"" << ATTR_NAMES[0] << "\":\"" << DEVADDRINT2string(dit->first) << "\", "
			<< "\"" << ATTR_NAMES[1] << "\":\"" << time2string(dit->second.t) << "\", "
			<< "\"" << ATTR_NAMES[2] << "\":" << dit->second.fcntup << ", "
			<< "\"" << ATTR_NAMES[3] << "\":" << dit->second.fcntdown << "}" << std::endl;
	}
	ss << "]";
	return ss.str();
}
