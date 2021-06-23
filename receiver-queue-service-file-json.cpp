#include <fstream>
#include <regex>
#include <iostream>
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
#define ATTRS_COUNT	4
static const char *ATTR_NAMES[ATTRS_COUNT] = {
	"time", 		// time value
	"id",			// identifier
	"payload",		// payload
	"dbids"			// database identifiers
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
		JsonFileReceiverQueueService *service;
		ReceiverQueueEntry entry;
	public:
		MessageQueueJsonHandler(JsonFileReceiverQueueService *svc)
			: idx(-1), service(svc)
		{
		}

		bool Null() {
			return true; 
		}

		bool Bool(bool b) {
			return true;
		}

		void putUInt(unsigned int value) {
			switch(idx) {
				case 0:	// time
					std::cerr << "int time: " << value << std::endl;
					entry.key.time.tv_sec = value;
					entry.key.time.tv_usec = 0;
					break;
				case 1:	// id
					std::cerr << "int id: " << value << std::endl;
					entry.key.id = value;
					break;
				case 3:	// db id
					std::cerr << "int db id: " << value << std::endl;
					entry.value.dbids.push_back(value);
					break;
			};
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
					entry.value.payload = base64_decode(s, true);
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
			return true; 
		}

		bool Key(const char* str, rapidjson::SizeType length, bool copy) { 
			idx = getAttrByName(str);
			return true;
		}
		bool EndObject(rapidjson::SizeType memberCount)
		{
			if (true)
				service->pushEntry(entry);
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
	// TODO
	storage.clear();
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

int JsonFileReceiverQueueService::save()
{
	std::fstream os;
	os.open(path, std::ios::out);
	os << "[";
	bool addSeparator(false);
	for (std::map<ReceiverQueueKey, ReceiverQueueValue>::const_iterator it = storage.begin(); it != storage.end(); it++) {
		if (addSeparator)
			os << ",";
		os << "{\"" 
			<< ATTR_NAMES[0] << "\":" << it->first.time.tv_sec << ",\"" 
			<< ATTR_NAMES[1] << "\":" << it->first.id << ",\"" 
			<< ATTR_NAMES[2] << "\":\"" << it->second.jsonPayload() << "\"" ;
		if (it->second.dbids.size()) {
			os << ",\"" << ATTR_NAMES[3] << "\":[";
			bool isFirst = true;
			for (std::vector<int>::const_iterator itd(it->second.dbids.begin()); itd != it->second.dbids.end(); itd++) {
				if (isFirst)
				{
					isFirst = false;
				} else {
					os << ", ";
				}
				os << *itd;
			}
			os << "]";
		}
		os << "}";
		addSeparator = true;
	}
	os << "]";
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
		size = UINT64_MAX;
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
	// TODO
	mutexMap.lock();
	// storage.erase(onum);
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
			<< ATTR_NAMES[2] << "\":\"" << it->second.jsonPayload() << "\"" ;
		if (it->second.dbids.size()) {
			ss << ", \"" << ATTR_NAMES[3] << "\":[";
			bool isFirst = true;
			for (std::vector<int>::const_iterator itd(it->second.dbids.begin()); itd != it->second.dbids.end(); itd++) {
				if (isFirst)
				{
					isFirst = false;
				} else  {
					ss << ", ";
				}
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
	int remainDbCount = it->second.popDbId(dbid);
	if (remainDbCount == 0) {
		// remove itself
		storage.erase(it);
	}
}
