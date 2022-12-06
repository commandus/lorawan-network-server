#include "gateway-stat.h"
#include <sstream>
#include <iomanip>

#include "utildate.h"
#include "utilstring.h"
#include "errlist.h"

/**
 * Create empty gateway statistics
 */
GatewayStat::GatewayStat()
	: addr(""), socket(0), name(""), gatewayId(0), errcode(0),
	t(0),					// UTC time of pkt RX, us precision, ISO 8601 'compact' format
	lat(0.0),				// latitude
	lon(0.0),				// longitude
	alt(0),					// altitude, meters, integer
	rxnb(0),				// Number of radio packets received (unsigned integer)
	rxok(0),				// Number of radio packets received with a valid PHY CRC
	rxfw(0),				// Number of radio packets forwarded (unsigned integer)
	ackr(0.0),				// Percentage of upstream datagrams that were acknowledged
	dwnb(0),				// Number of downlink datagrams received (unsigned integer)
	txnb(0)					// Number of packets emitted (unsigned integer)
{
    memset(&sockaddr, 0, sizeof(sockaddr));
}

/**
 * Copy gateway statistics
 */
GatewayStat::GatewayStat(
	const GatewayStat &value
)
{
	addr = value.addr; 
	socket = value.socket; 
	memmove(&sockaddr, &value.socket, sockaddr.sin6_family ==  AF_INET6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in));
	name = value.name;
	gatewayId = value.gatewayId;
	errcode = value.errcode;
	t = value.t;
	lat = value.lat;
	lon = value.lon;
	alt = value.alt;
	rxnb = value.rxnb;
	rxok = value.rxok;
	rxfw = value.rxfw;
	ackr = value.ackr;
	dwnb = value.dwnb;
	txnb = value.txnb;
}

/**
 * 8-bit gateway Identifier
 */
bool GatewayStat::operator==(
	GatewayStat &rhs
) const {
	return gatewayId == rhs.gatewayId;
}

/**
 * Statistics property names
 */
static const char* STAT_NAMES[13] = {
	"gwid",	// 0 string id
	"addr",	// 1 string address
	"name",	// 2 string name
	"time", // 3 string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
	"lati", // 4 number 
	"long", // 5 number
	"alti", // 6 number
	"rxnb", // 7 number
	"rxok", // 8 number
	"rxfw", // 9 number
	"ackr", // 10 number
	"dwnb", // 11 number
	"txnb" // 12 number
};

/**
 * Serialize statistics
 */
void GatewayStat::toJSON(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
) const
{
	rapidjson::Value vId;
	std::stringstream ss;
	ss << std::hex << gatewayId;
	std::string ssr = ss.str();
	vId.SetString(ssr.c_str(), (rapidjson::SizeType) ssr.size(), allocator);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[0])), vId, allocator);

	rapidjson::Value vaddr(rapidjson::kStringType);
	vaddr.SetString(addr.c_str(), (rapidjson::SizeType) addr.size(), allocator);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[1])), vaddr, allocator);

	rapidjson::Value vname(rapidjson::kStringType);
	vname.SetString(name.c_str(), (rapidjson::SizeType) name.size(), allocator);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[2])), vname, allocator);

	std::string dt = ltimeString(t, -1, "%FT%T%Z");
	rapidjson::Value v1(rapidjson::kStringType);
	v1.SetString(dt.c_str(), (rapidjson::SizeType) dt.size(), allocator);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[3])), v1, allocator);

	rapidjson::Value v2(lat);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[4])), v2, allocator);

	rapidjson::Value v3(lon);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[5])), v3, allocator);

	rapidjson::Value v4(alt);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[6])), v4, allocator);

	rapidjson::Value v5(rxnb);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[7])), v5, allocator);

	rapidjson::Value v6(rxok);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[8])), v6, allocator);

	rapidjson::Value v7(rxfw);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[9])), v7, allocator);

	rapidjson::Value v8(ackr);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[10])), v8, allocator);

	rapidjson::Value v9(dwnb);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[11])), v9, allocator);

	rapidjson::Value v10(txnb);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[12])), v10, allocator);
}

/**
 * Parse JSON
 */
int GatewayStat::parse(
	rapidjson::Value &value
)
{
	int cnt = 0;

	if (value.HasMember(STAT_NAMES[0])) {
		rapidjson::Value &v = value[STAT_NAMES[0]];
		if (v.IsUint64()) {
			gatewayId = v.GetUint64();
		} else if (v.IsString()) {
			gatewayId = strtoull(v.GetString(), NULL, 16);
		}
		cnt++;
	}

	if (value.HasMember(STAT_NAMES[1])) {
		rapidjson::Value &v = value[STAT_NAMES[1]];
		if (v.IsString()) {
			addr = v.GetString();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[2])) {
		rapidjson::Value &v = value[STAT_NAMES[2]];
		if (v.IsString()) {
			name = v.GetString();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[3])) {
		rapidjson::Value &v = value[STAT_NAMES[3]];
		if (v.IsString()) {
			t = parseDate(v.GetString());
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[4])) {
		rapidjson::Value &v = value[STAT_NAMES[4]];
		if (v.IsDouble()) {
			lat = v.GetDouble();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[5])) {
		rapidjson::Value &v = value[STAT_NAMES[5]];
		if (v.IsDouble()) {
			lon = v.GetDouble();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[6])) {
		rapidjson::Value &v = value[STAT_NAMES[6]];
		if (v.IsInt()) {
			alt = v.GetInt();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[7])) {
		rapidjson::Value &v = value[STAT_NAMES[7]];
		if (v.IsInt()) {
			rxnb = v.GetInt();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[8])) {
		rapidjson::Value &v = value[STAT_NAMES[8]];
		if (v.IsInt()) {
			rxok = v.GetInt();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[9])) {
		rapidjson::Value &v = value[STAT_NAMES[9]];
		if (v.IsInt()) {
			rxfw = v.GetInt();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[10])) {
		rapidjson::Value &v = value[STAT_NAMES[10]];
		if (v.IsInt()) {
			ackr = v.GetInt();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[11])) {
		rapidjson::Value &v = value[STAT_NAMES[11]];
		if (v.IsInt()) {
			dwnb = v.GetInt();
			cnt++;
		}
	}

	if (value.HasMember(STAT_NAMES[12])) {
		rapidjson::Value &v = value[STAT_NAMES[12]];
		if (v.IsInt()) {
			txnb = v.GetInt();
			cnt++;
		}
	}
	return (cnt > 0 ? 0 : ERR_CODE_NO_GATEWAY_STAT);	// means no properties found
}

/**
 * Serialize JSON string
 */
std::string GatewayStat::toJsonString() const
{
		std::stringstream ss;
		ss << "{"
			<< "\"gwid\":\"" << std::hex << gatewayId << std::dec
			<< "\", \"addr\":\"" << addr
			<< "\", \"name\":\"" << name
			<< "\", \"time\":\"" << time2string(t)
			<< "\", \"lati\":" << std::fixed << std::setprecision(5) << lat
			<< ", \"long\":" << std::fixed << std::setprecision(5) << lon
			<< ", \"alti\":" << alt
			<< ", \"rxnb\":" << rxnb
			<< ", \"rxok\":" << rxok
			<< ", \"rxfw\":" << rxfw
			<< ", \"ackr\":" << std::fixed << std::setprecision(1) << ackr
			<< ", \"dwnb\":" << dwnb
			<< ", \"txnb\":" << txnb
			<< "}";
		return ss.str();
}

/**
 * debug string
 */
std::string GatewayStat::toString() const
{
		std::stringstream ss;
		ss 
			<< time2string(t)
			<< " " << std::hex << gatewayId << std::dec
			<< " " << addr
			<< " " << name
			<< " (" << std::fixed << std::setprecision(5) << lat
			<< ", " << std::fixed << std::setprecision(5) << lon
			<< ", " << alt
			<< "), rxnb: " << rxnb
			<< ", rxok: " << rxok
			<< ", rxfw: " << rxfw
			<< ", ackr: " << std::fixed << std::setprecision(1) << ackr
			<< ", dwnb: " << dwnb
			<< ", txnb: " << txnb;
		return ss.str();
}
