#include <fstream>
#include <sstream>
#include <regex>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "device-history-service-json.h"
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

JsonFileDeviceHistoryService::JsonFileDeviceHistoryService()
	: path(""), errcode(0), errmessage("")
{

}

JsonFileDeviceHistoryService::~JsonFileDeviceHistoryService()
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
		JsonFileDeviceHistoryService *service;
		bool isNetworkIdentity;
		int idx;
		DEVADDR k;
		DEVICE_HISTORY_ITEM v;
	public:
		DeviceStatJsonHandler(JsonFileDeviceHistoryService *svc)
			: service(svc), isNetworkIdentity(false), idx(-1)
		{
			memset(&k, 0, sizeof(DEVADDR));
			memset(&v, 0, sizeof(DEVICE_HISTORY_ITEM));
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
					v.fcntdown = u;
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

void JsonFileDeviceHistoryService::clear()
{
	storage.clear();
}

int JsonFileDeviceHistoryService::load()
{
	clear();
    DeviceStatJsonHandler handler(this);
    rapidjson::Reader reader;
	FILE* fp = fopen(path.c_str(), "rb");
	if (!fp)
		return ERR_CODE_INVALID_JSON;
 	char readBuffer[4096];
	rapidjson::FileReadStream istrm(fp, readBuffer, sizeof(readBuffer));
    rapidjson::ParseResult r = reader.Parse<rapidjson::kParseCommentsFlag>(istrm, handler);
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

int JsonFileDeviceHistoryService::save()
{
	std::fstream os;
	os.open(path, std::ios::out);
	os << "[";
	bool addSeparator(false);
	for (std::map<DEVADDRINT, DEVICE_HISTORY_ITEM>::const_iterator it = storage.begin(); it != storage.end(); it++) {
		if (addSeparator)
			os << ",";
		os << std::endl << "{\"" 
			<< ATTR_NAMES[0] << "\": \"" << DEVADDRINT2string(it->first) << "\", \"" 
			<< ATTR_NAMES[1] << "\": \"" << time2string(it->second.t) << "\", \"" 
			<< ATTR_NAMES[2] << "\": " << it->second.fcntup << ", \""
			<< ATTR_NAMES[3] << "\":" << it->second.fcntdown << "}" << std::endl;
		addSeparator = true;
	}
	os << "]" << std::endl;
	int r = os.bad() ? ERR_CODE_OPEN_DEVICE : 0;
	os.close();
	return r;
}

int JsonFileDeviceHistoryService::get(
        DEVADDR &devaddr,
        DeviceHistoryItem &retval
) 
{
	mutexMap.lock();
	std::map<DEVADDRINT, DEVICE_HISTORY_ITEM>::const_iterator it(storage.find(DEVADDRINT(devaddr)));
    if (it == storage.end()) {
		mutexMap.unlock();
		return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
	}
	retval.set(it->first.a, it->second);
	mutexMap.unlock();
	return 0;
}

// List entries
void JsonFileDeviceHistoryService::list(
	std::vector<DeviceHistoryItem> &retval,
	size_t offset,
	size_t size
) {
	int64_t c = -1;
	if (size == 0)
		size = SIZE_MAX;
	for (std::map<DEVADDRINT, DEVICE_HISTORY_ITEM>::const_iterator it(storage.begin()); it != storage.end(); it++) {
		c++;
		if (c < offset)
			continue;
		if (c >= size)
			break;
		DeviceHistoryItem v(it->first.a, it->second);
		retval.push_back(v);
	}
}

void JsonFileDeviceHistoryService::put(
        DEVADDR &devaddr,
        DEVICE_HISTORY_ITEM &value
)
{
	mutexMap.lock();
	storage[devaddr] = value;
	mutexMap.unlock();
}

void JsonFileDeviceHistoryService::putUp(
	DEVADDR &devaddr,
	const time_t &tm,
	uint32_t value
)
{
	mutexMap.lock();
	DEVICE_HISTORY_ITEM v = storage[devaddr];
	v.fcntup = value;
	v.t = tm;
	storage[devaddr] = v;
	mutexMap.unlock();
}

void JsonFileDeviceHistoryService::putDown(
	DEVADDR &devaddr,
	const time_t &tm,
	uint32_t value
)
{
	mutexMap.lock();
	DEVICE_HISTORY_ITEM v = storage[devaddr];
	v.fcntdown = value;
	v.t = tm;
	storage[devaddr] = v;
	mutexMap.unlock();
}

// increment downstream from network server to the device
uint32_t JsonFileDeviceHistoryService::incrementDown
(
	const DEVADDR &devaddr,
	const time_t &time
)
{
	mutexMap.lock();
	DEVICE_HISTORY_ITEM v = storage[devaddr];
	v.fcntdown++;
	v.t = time;
	storage[devaddr] = v;
	mutexMap.unlock();
	return v.fcntdown;
}

void JsonFileDeviceHistoryService::rm(
	DEVADDR &addr
)
{
	mutexMap.lock();
	storage.erase(addr);
	mutexMap.unlock();
}

int JsonFileDeviceHistoryService::init(
	const std::string &option, 
	void *data
)
{
	path = option;
	return load();
}

void JsonFileDeviceHistoryService::flush()
{
	save();
}

void JsonFileDeviceHistoryService::done()
{

}

std::string JsonFileDeviceHistoryService::toJsonString()
{
	std::stringstream ss;
	ss << "[" << std::endl;
	bool needComma = false;
	for (std::map<DEVADDRINT, DEVICE_HISTORY_ITEM, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
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
