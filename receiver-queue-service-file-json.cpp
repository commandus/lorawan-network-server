#include <fstream>
#include <regex>
#include <base64/base64.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#pragma clang diagnostic pop
 
#include "receiver-queue-service-file-json.h"
#include "utilstring.h"
#include "errlist.h"

/**
 * 	JSON attribute names
 */
#define ATTRS_COUNT	11
static const char *ATTR_NAMES[ATTRS_COUNT] = {
	"time", 		// 0 time value
	"id",			// 1 identifier
	"payload",		// 2 payload
	"dbids",		// 3 database identifiers
	"deviceId",		// 4 device id object
	// deviceId members
	"activation",
	"class",
	"eui",
	"nwkSKey",
	"appSKey",
	"name"
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

JsonFileReceiverQueueService::JsonFileReceiverQueueService() 
	: ReceiverQueueService(), path(""), errcode(0), errmessage("")
{

}

JsonFileReceiverQueueService::~JsonFileReceiverQueueService() 
{
	done();
}

/**
 * 
 * Loads ReceiverQueueEntry
 *	[
 *		{
 *	 		"time": 1624327719,
 *          "id": 1,
 *          "packet": "string",
 *          "dbids": [1,2,3]
 *		},
 *		..
 *	]
 */ 
class MessageQueueJsonHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, MessageQueueJsonHandler> {
	private:
		int idx;
		int level;
		JsonFileReceiverQueueService *service;
		ReceiverQueueEntry entry;
	public:
		MessageQueueJsonHandler(JsonFileReceiverQueueService *svc)
			: idx(-1), level(0), service(svc)
		{
		}

		void putUInt(unsigned int value) {
			switch(idx) {
				case 0:	// time
					entry.key.time.tv_sec = value;
					entry.key.time.tv_usec = 0;
					break;
				case 1:	// id
					entry.key.id = value;
					break;
				case 3:	// db id
					entry.value.dbids.push_back(value);
					break;
			}
		}

		bool Int(int value) {
			putUInt(value);
			return true;
		}

		bool Uint(unsigned value) {
			putUInt(value);
			return true;
		}

		bool Int64(int64_t value) {
			putUInt(value);
			return true;
		}

		bool Uint64(uint64_t value) {
			putUInt(value);
			return true;
		}

		bool Double(double value) {
			putUInt(value);
			return true;
		}

		bool String(const char* str, rapidjson::SizeType length, bool copy) { 
			std::string s(str, length); 
			switch(idx) {
				case 2:	// payload
					try {
						// base64
						entry.value.payload = base64_decode(s, true);
					}
					catch (const std::exception& e) {
						return false;
					}
					break;
				case 4:	// deviceId:
					break;
				case 5:	// 	device activation
					entry.value.deviceId.activation = string2activation(s);
					break;
				case 6:	// 	device class
					entry.value.deviceId.deviceclass = string2deviceclass(s);
					break;
				case 7:	// 	device eui
					string2DEVEUI(entry.value.deviceId.devEUI, s);
					break;
				case 8:	// 	device nwkSKey
					string2KEY(entry.value.deviceId.nwkSKey, s);
					break;
				case 9:	// 	device appSKey
					string2KEY(entry.value.deviceId.appSKey, s);
					break;
				case 10:	// device name
					string2DEVICENAME(entry.value.deviceId.name, s.c_str());
					break;
				default:
					{
					unsigned long value = atoll(s.c_str());
					putUInt(value);
					}
					break;
			}
			return true;
		}

		bool StartObject() { 
			level++;
			return true; 
		}

		bool Key(const char* str, rapidjson::SizeType length, bool copy) { 
			idx = getAttrByName(str);
			return true;
		}
		bool EndObject(rapidjson::SizeType memberCount)
		{
			if (level == 1) {
				service->pushEntry(entry);
				entry.clear();
			}
			return true;
		}

		bool StartArray() {
			return true; 
		}

		bool EndArray(rapidjson::SizeType elementCount) { 
			return true; 
		}
};

void JsonFileReceiverQueueService::clear(
	time_t olderthan
)
{
	if (olderthan == 0) {
		clear();
		return;
	}

	for (std::map<ReceiverQueueKey, ReceiverQueueValue, ReceiverQueueKeyCompare>::iterator mit(storage.begin()); mit != storage.end(); ) {
		if (mit->first.time.tv_sec <= olderthan)
			mit = storage.erase(mit); // remove itself
		else
			mit++;
	}
}

void JsonFileReceiverQueueService::clear()
{
	storage.clear();
}

// Remove all entries with databases
void JsonFileReceiverQueueService::clearDbs(
	const std::vector <int> &dbs2delete
)
{
	for (std::vector<int>::const_iterator it(dbs2delete.begin()); it != dbs2delete.end(); it++) {
		for (std::map<ReceiverQueueKey, ReceiverQueueValue, ReceiverQueueKeyCompare>::iterator mit(storage.begin()); mit != storage.end(); ) {
			int remainDbCount = mit->second.popDbId(*it);
			if (remainDbCount == 0)
				mit = storage.erase(mit); // remove itself
			else
				mit++;
		}
	}
}

int JsonFileReceiverQueueService::load()
{
	clear();
    MessageQueueJsonHandler handler(this);
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

int JsonFileReceiverQueueService::save()
{
	std::fstream os;
	os.open(path, std::ios::out);
	os << "[" << std::endl;
	bool addSeparator(false);
	for (std::map<ReceiverQueueKey, ReceiverQueueValue>::const_iterator it = storage.begin(); it != storage.end(); it++) {
		if (addSeparator)
			os << "," << std::endl;
		os << "{\"" 
			<< ATTR_NAMES[0] << "\":" << it->first.time.tv_sec << ",\"" 
			<< ATTR_NAMES[1] << "\":" << it->first.id << ",\"" 
			<< ATTR_NAMES[4] << "\":" << it->second.deviceId.toJsonString() << ",\"" 
			<< ATTR_NAMES[2] << "\":\"" << it->second.jsonPayload() << "\"" ;
		if (it->second.dbids.size()) {
			os << ",\"" << ATTR_NAMES[3] << "\":[";
			bool isFirst = true;
			for (std::vector<int>::const_iterator itd(it->second.dbids.begin()); itd != it->second.dbids.end(); itd++) {
				if (isFirst)
					isFirst = false;
				else
					os << ", ";
				os << *itd;
			}
			os << "]";
		}
		os << "}";
		addSeparator = true;
	}
	os << std::endl << "]" << std::endl;
	int r = os.bad() ? ERR_CODE_OPEN_DEVICE : 0;
	os.close();
	return r;
}

int JsonFileReceiverQueueService::get(
	int onum,
	ReceiverQueueEntry &retval
) 
{
	mutexMap.lock();
	std::map<ReceiverQueueKey, ReceiverQueueValue>::const_iterator it(storage.begin());
    if (it == storage.end()) {
		mutexMap.unlock();
		return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
	}
	retval.set(it->first, it->second);
	mutexMap.unlock();
	return 0;
}

// List entries
void JsonFileReceiverQueueService::list(
	std::vector<ReceiverQueueEntry> &retval,
	size_t offset,
	size_t size
) {
	int64_t c = -1;
	if (size == 0)
		size = SIZE_MAX;
	for (std::map<ReceiverQueueKey, ReceiverQueueValue>::const_iterator it(storage.begin()); it != storage.end(); it++) {
		c++;
		if (c < offset)
			continue;
		if (c >= size)
			break;
		ReceiverQueueEntry v(it->first, it->second);
		retval.push_back(v);
	}
}

void JsonFileReceiverQueueService::pushEntry(
	ReceiverQueueEntry &value
)
{
	mutexMap.lock();
	storage[value.key] = value.value;
	mutexMap.unlock();
}

void JsonFileReceiverQueueService::remove
(
	int onum
)
{
	mutexMap.lock();
	// 
	int c = 0;
	for (std::map<ReceiverQueueKey, ReceiverQueueValue, ReceiverQueueKeyCompare>::const_iterator it(storage.begin()); it != storage.end();) {
		if (onum == c) 
		{
			storage.erase(it);
			break;
		} else
			it++;
		c++;
	}
	mutexMap.unlock();
}

int JsonFileReceiverQueueService::init(
	const std::string &option, 
	void *data
)
{
	path = option;
	return load();
}

void JsonFileReceiverQueueService::flush()
{
	save();
}

void JsonFileReceiverQueueService::done()
{
	flush();
}

std::string JsonFileReceiverQueueService::toJsonString()
{
	std::stringstream ss;
	ss << "[";
	bool addSeparator(false);
	for (std::map<ReceiverQueueKey, ReceiverQueueValue, ReceiverQueueKeyCompare>::const_iterator it(storage.begin()); it != storage.end(); it++) {
		if (addSeparator)
			ss << ",";
		else
			addSeparator = true;
		ss << "{\"" 
			<< ATTR_NAMES[0] << "\":" << it->first.time.tv_sec << ",\"" 
			<< ATTR_NAMES[1] << "\":" << it->first.id << ",\"" 
			<< ATTR_NAMES[4] << "\":" << it->second.deviceId.toJsonString() << ",\"" 
			<< ATTR_NAMES[2] << "\":\"" << it->second.jsonPayload() << "\"" ;
		if (it->second.dbids.size()) {
			ss << ", \"" << ATTR_NAMES[3] << "\":[";
			bool isFirst = true;
			for (std::vector<int>::const_iterator itd(it->second.dbids.begin()); itd != it->second.dbids.end(); itd++) {
				if (isFirst)
					isFirst = false;
				else
					ss << ", ";
				ss << *itd;
			}
			ss << "]";
		}
		ss << "}";
	}
	ss << "]";
	return ss.str();
}

int JsonFileReceiverQueueService::count()
{
	return storage.size();
}

int JsonFileReceiverQueueService::pop(
	const int &dbid,
	ReceiverQueueEntry &value
)
{
	std::map<ReceiverQueueKey, ReceiverQueueValue, ReceiverQueueKeyCompare>::iterator it(storage.begin());
	if (it == storage.end())
		return ERR_CODE_QUEUE_EMPTY;
	value.set(it->first, it->second);

	mutexMap.lock();
	int remainDbCount = it->second.popDbId(dbid);
	if (remainDbCount == 0) {
		// remove itself
		storage.erase(it);
	}
	mutexMap.unlock();
	return remainDbCount >= 0 ? 0 : remainDbCount;
}

int JsonFileReceiverQueueService::peek(
	const int &dbid,
	ReceiverQueueEntry &value
)
{
	std::map<ReceiverQueueKey, ReceiverQueueValue, ReceiverQueueKeyCompare>::iterator it(storage.begin());
	if (it == storage.end())
		return ERR_CODE_QUEUE_EMPTY;
	if (!it->second.hasDbId(dbid))
		return ERR_CODE_NO_DATABASE;
	value.set(it->first, it->second);
	return 0;
}
