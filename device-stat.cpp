#include "device-stat.h"
#include <sstream>
#include <iomanip>

#include "utildate.h"
#include "utilstring.h"
#include "errlist.h"

/**
 * Create empty gateway statistics
 */
DeviceStat::DeviceStat()
	: addr(0),		// network address
	t(0),				// last session time
	fcntup(0),			// Last frame count sent by end device
	fcntdown(0)			// Last frame count sent by network service
{

}

/**
 * Copy gateway statistics
 */
DeviceStat::DeviceStat(
	const DeviceStat &value
)
{
	addr = value.addr;			// network address
	t = value.t;				// last session time
	fcntup = value.fcntup;		// Last frame count sent by end device
	fcntdown = value.fcntdown;	///< Last frame count sent by network service
}

DeviceStat::DeviceStat
(
	const uint32_t &aaddr,
	const DEVICESTAT &value
)
	: addr(aaddr), t(value.t), fcntup(value.fcntup), fcntdown(value.fcntdown)
{

}

void DeviceStat::set
(
	const uint32_t &aaddr,
	const DEVICESTAT &value
)
{
	addr = aaddr;				// network address
	t = value.t;				// last session time
	fcntup = value.fcntup;		// Last frame count sent by end device
	fcntdown = value.fcntdown;	///< Last frame count sent by network service
}

/**
 * 8-bit gateway Identifier
 */
bool DeviceStat::operator==(
	DeviceStat &rhs
) const {
	return addr == rhs.addr;
}

/**
 * Statistics property names
 */
static const char* STAT_NAMES[13] = {
	"id",		// 0 string id (reserved)
	"addr",		// 1 string address
	"time", 	// 2 string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
	"fcntup", 	// 3 number 
	"fcntdown"	// 4 number
};

/**
 * Serialize device statistics
 */
void DeviceStat::toJSON(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
) const
{
	/*
	rapidjson::Value vId;
	std::stringstream ss;
	ss << std::hex << id;
	std::string ssr = ss.str();
	vId.SetString(ssr.c_str(), ssr.length(), allocator);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[0])), vId, allocator);
	*/

	std::string haddr = DEVADDR2string(*(DEVADDR*) &addr);
	rapidjson::Value v1(rapidjson::kStringType);
	v1.SetString(haddr.c_str(), haddr.length(), allocator);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[1])), v1, allocator);

	std::string dt = ltimeString(t, -1, "%F %T %Z");
	rapidjson::Value v2(rapidjson::kStringType);
	v1.SetString(dt.c_str(), dt.length(), allocator);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[2])), v2, allocator);

	rapidjson::Value v3(fcntup);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[3])), v3, allocator);

	rapidjson::Value v4(fcntdown);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[4])), v4, allocator);
}

/**
 * Parse JSON
 */
int DeviceStat::parse(
	rapidjson::Value &value
)
{
	int cnt = 0;
	/*
	if (value.HasMember(STAT_NAMES[0])) {
		rapidjson::Value &v = value[STAT_NAMES[0]];
		if (v.IsUint64()) {
			id = v.GetUint64();
		} else if (v.IsString()) {
			id = strtoull(v.GetString(), NULL, 16);
		}
		cnt++;
	}
	*/

	if (value.HasMember(STAT_NAMES[1])) {
		rapidjson::Value &v = value[STAT_NAMES[1]];
		if (v.IsString()) {
			string2DEVADDR(*(DEVADDR*) &addr, v.GetString());
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[2])) {
		rapidjson::Value &v = value[STAT_NAMES[3]];
		if (v.IsString()) {
			t = parseDate(v.GetString());
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[3])) {
		rapidjson::Value &v = value[STAT_NAMES[3]];
		if (v.IsUint()) {
			fcntup = v.GetUint();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[4])) {
		rapidjson::Value &v = value[STAT_NAMES[4]];
		if (v.IsUint()) {
			fcntdown = v.GetUint();
			cnt++;
		}
	}

	return (cnt > 0 ? 0 : ERR_CODE_NO_GATEWAY_STAT);	// means no any properties found
}

/**
 * Serialize JSON string
 */
std::string DeviceStat::toJsonString() const
{
		std::stringstream ss;
		ss << "{"
			// << "\"id\":\"" << std::hex << id << std::dec << "\", "
			<< "\"addr\":\"" << addr
			<< "\", \"time\":\"" << time2string(t)
			<< "\", \"fcntup\":" << fcntup
			<< ", \"fcntdown\":" << fcntdown
			<< "}";
		return ss.str();
}

/**
 * debug string
 */
std::string DeviceStat::toString() const
{
		std::stringstream ss;
		ss 
			// << " " << std::hex << id << std::dec
			<< addr
			<< " " << time2string(t)
			<< " fcntup: " << fcntup
			<< " fcntdown: " << fcntdown;
		return ss.str();
}
