#include <algorithm>
#include <sstream>

#include "receiver-queue-service.h"
#include "base64/base64.h"
#include "errlist.h"
#include "utildate.h"
#include "utillora.h"

#define CLEAR_COUNT	2048

ReceiverQueueKey::ReceiverQueueKey()
	: time({0, 0}), id(0)
{

}

ReceiverQueueKey::ReceiverQueueKey
(
	const ReceiverQueueKey &value
)
	: time(value.time), id(value.id)
{

}

void ReceiverQueueKey::clear()
{
	time = {0, 0};
	id = 0;
}

void ReceiverQueueKey::setProperties
(
	std::map<std::string, std::string> &retval
)
{
	retval["time"] = std::to_string(time.tv_sec);
	retval["timestamp"] = time2string(time.tv_sec);// timeval2string(time);
	retval["id"] = std::to_string(id);
}

ReceiverQueueValue::ReceiverQueueValue
(
	const ReceiverQueueValue &value
)
	: deviceId(value.deviceId), payload(value.payload), dbids(value.dbids)
{
	memmove(addr, value.addr, sizeof(DEVADDR)); 
}

ReceiverQueueValue::ReceiverQueueValue()
	: payload("")
{

}

std::string ReceiverQueueValue::jsonPayload() const
{
	return base64_encode(payload, false);
}

bool ReceiverQueueValue::setJsonPayload
(
	const std::string &jsonValue
)
{
	try {
		payload = base64_decode(jsonValue, true);
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}

void ReceiverQueueValue::clear()
{
	payload = "";
	dbids.clear();
}

/**
 * @param dbId database identifier
 * @return remaining database count, if error, return negative number
 */
int ReceiverQueueValue::popDbId
(
	int dbId
)
{
	for (std::vector<int>::iterator it(dbids.begin()); it != dbids.end(); it++)
	{
		if (*it == dbId) {
        	dbids.erase(it);
			return dbids.size();
		}
    }
	return ERR_CODE_NO_DATABASE;
}	

/**
 * @return true if database exists
 */
bool ReceiverQueueValue::hasDbId
(
	int dbid
)
{
	return (std::find(dbids.begin(), dbids.end(), dbid) != dbids.end());
}	

void ReceiverQueueValue::setProperties
(
	std::map<std::string, std::string> &retval
)
{
	deviceId.setProperties(retval);
	retval["addr"] = DEVADDRINT2string(addr);
	// application port number (1..223). 0- MAC, 224- test, 225..255- reserved
	retval["fport"] = std::to_string(fport);
}

bool ReceiverQueueKeyCompare::operator() (
	const ReceiverQueueKey& l,
	const ReceiverQueueKey& r
) const 
{
	if (l.time.tv_sec < r.time.tv_sec)
		return true;
	if (l.time.tv_sec > r.time.tv_sec)
		return false;

	if (l.time.tv_usec < r.time.tv_usec)
		return true;
	if (l.time.tv_usec > r.time.tv_usec)
		return false;

	return l.id < r.id;
}

ReceiverQueueEntry::ReceiverQueueEntry()
{

}

ReceiverQueueEntry::ReceiverQueueEntry
(
	const ReceiverQueueKey &akey,
	const ReceiverQueueValue &avalue
)
{
	key = akey;
	value = avalue;
}

void ReceiverQueueEntry::set(
	const ReceiverQueueKey &akey,
	const ReceiverQueueValue &avalue
) {
	key = akey;
	value = avalue;
}

void ReceiverQueueEntry::clear()
{
	key.clear();
	value.clear();
}

std::string ReceiverQueueEntry::toJsonString() const
{
	std::stringstream ss;
	ss << "{\"time\":" << key.time.tv_sec
		<< ",\"id\":" << key.id
		<< "\"deviceId\":" << value.deviceId.toJsonString()
		<< ",\"payload\":\"" << base64_encode(value.payload, false) << "\"";
	if (value.dbids.size()) {
		ss << ",\"" << "dbids\":[";
		bool isFirst = true;
		for (std::vector<int>::const_iterator itd(value.dbids.begin()); itd != value.dbids.end(); itd++) {
			if (isFirst)
			{
				isFirst = false;
			} else {
				ss << ",";
			}
			ss << *itd;
		}
		ss << "]";
	}
	ss << "}";
	return ss.str();
}

void ReceiverQueueEntry::setProperties
(
	std::map<std::string, std::string> &values,
	const std::map<std::string, std::string> &aliases

)
{
	// copy only bands lsted in aliases, and replace key to the alias name
	std::map<std::string, std::string> sessionProperties;
	this->key.setProperties(sessionProperties);
	this->value.setProperties(sessionProperties);
	for (std::map<std::string, std::string>::const_iterator it(aliases.begin()); it != aliases.end(); it++) {
		std::map<std::string, std::string>::const_iterator f = sessionProperties.find(it->first);
		if (f != sessionProperties.end()) {
			if (!it->second.empty()) {
				values[it->second] = f->second;
			}
		}
	}
}

ReceiverQueueService::ReceiverQueueService()
	: cnt(0)
{
}

ReceiverQueueService::~ReceiverQueueService()
{

}

int ReceiverQueueService::next()
{
	cnt++;
	return cnt;
}

void ReceiverQueueService::setDbs
(
	const std::vector<int> &values
)
{
	std::vector<int> d = dbs;
	for (std::vector<int>::iterator it(d.begin()); it != d.end();) {
		if (std::find(values.begin(), values.end(), *it) != values.end())
			d.erase(it);
		else
			it++;
	}
	// delete old ids not in new one
	clearDbs(d);
	dbs = values;
}

/**
 * Check does it received already
 */
bool ReceiverQueueService::isDuplicated
(
	const SemtechUDPPacket &packet,
	const time_t &received
)
{
	DEVADDRINT v = packet.getDeviceAddr();
	std::map<DEVADDRINT, FCNT_TIME, DEVADDRINTCompare>::const_iterator it(fcnts.find(v));
	if (it == fcnts.end()) {
		// device address not found
		FCNT_TIME ft = { packet.getRfmHeader()->fcnt, received };
		fcnts[v] = ft;
	} else {
		// address exists
		// TODO if addresses changed, need to restart service
		if (packet.getRfmHeader()->fcnt == fcnts[v].fcnt) {
			// duplicated, ignore time
			return true;
		}
	}
	return false;
}

/**
 * Clear oldest device from the memory
 */ 
void ReceiverQueueService::clearFcnts()
{
	// fcnts.clear();
	time_t t = time(NULL);
	t -= 5;	// 5s
	for (std::map<DEVADDRINT, FCNT_TIME, DEVADDRINTCompare>::const_iterator it(fcnts.begin()); it != fcnts.end();) {
		if (it->second.time <= t)
			it = fcnts.erase(it);
		else 
			it++;
	}
}

// brute-force push payload
void ReceiverQueueService::pushForce
(
	const DeviceId &deviceId,
	const std::string &payload,
	const timeval &time
)
{
	ReceiverQueueEntry e;
	e.key.id = next();
	e.key.time = time;
	memset(e.value.addr, 0, sizeof(DEVADDR));
	e.value.deviceId = deviceId;
	e.value.payload = payload;
	e.value.dbids = dbs;
	pushEntry(e);
}

bool ReceiverQueueService::push
(
	const SemtechUDPPacket &packet,
	const timeval &timeval
)
{
	if (isDuplicated(packet, timeval.tv_sec))
		return false;
	ReceiverQueueEntry e;
	e.key.id = next();
	e.key.time = timeval;
	memmove(e.value.addr, packet.getRfmHeader()->devaddr, sizeof(DEVADDR));
	e.value.deviceId = packet.devId;
	e.value.payload = packet.payload;
	e.value.dbids = dbs;
	e.value.fport = packet.header.fport;
	pushEntry(e);

	// garbage collector ;)
	if (e.key.id % CLEAR_COUNT == CLEAR_COUNT - 1)
		clearFcnts();
    return true;
}
