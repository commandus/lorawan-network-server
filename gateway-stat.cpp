#include "gateway-stat.h"
#include <sstream>
#include <iomanip>

#include "utildate.h"
#include "utilstring.h"
#include "errlist.h"

GatewayStat::GatewayStat()
: gatewayId(0), errcode(0),
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

}

GatewayStat::GatewayStat(
	const GatewayStat &value
)
{
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

static const char* STAT_NAMES[11] = {
	"stat",	// 0 array name
	"time", // 1 string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
	"lati", // 2 number 
	"long", // 3 number
	"alti", // 4 number
	"rxnb", // 5 number
	"rxok", // 6 number
	"rxfw", // 7 number
	"ackr", // 8 number
	"dwnb", // 9 number
	"txnb", // 10 number
};

void GatewayStat::toJSON(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
) const
{
	rapidjson::Value vId;
	std::stringstream ss;
	ss << std::hex << gatewayId;
	std::string ssr = ss.str();
	vId.SetString(ssr.c_str(), ssr.length(), allocator);
	value.AddMember("gwid", vId, allocator);

	std::string dt = ltimeString(t, -1, "%F %T %Z");
	rapidjson::Value v1(rapidjson::kStringType);
	v1.SetString(dt.c_str(), dt.length(), allocator);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[1])), v1, allocator);

	rapidjson::Value v2(lat);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[2])), v2, allocator);

	rapidjson::Value v3(lon);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[3])), v3, allocator);

	rapidjson::Value v4(alt);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[4])), v4, allocator);

	rapidjson::Value v5(rxnb);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[5])), v5, allocator);

	rapidjson::Value v6(rxok);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[6])), v6, allocator);

	rapidjson::Value v7(rxfw);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[7])), v7, allocator);

	rapidjson::Value v8(ackr);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[8])), v8, allocator);

	rapidjson::Value v9(dwnb);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[9])), v9, allocator);

	rapidjson::Value v10(txnb);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(STAT_NAMES[10])), v10, allocator);
}

int GatewayStat::parse(
	rapidjson::Value &value
)
{
	errcode = ERR_CODE_NO_GATEWAY_STAT;	// means no "stat" object is recognized

	if (value.HasMember("gwid")) {
		rapidjson::Value &v = value["gwid"];
		if (v.IsUint64()) {
			gatewayId = v.GetUint64();
		} else if (v.IsString()) {
			gatewayId = strtoull(v.GetString(), NULL, 16);
		}
	}

	if (value.HasMember(STAT_NAMES[1])) {
		rapidjson::Value &v = value[STAT_NAMES[1]];
		if (v.IsString()) {
			t = parseDate(v.GetString());
			errcode = 0;
		}
	}

	if (value.HasMember(STAT_NAMES[2])) {
		rapidjson::Value &v = value[STAT_NAMES[2]];
		if (v.IsDouble()) {
			lat = v.GetDouble();
		}
	}

	if (value.HasMember(STAT_NAMES[3])) {
		rapidjson::Value &v = value[STAT_NAMES[3]];
		if (v.IsDouble()) {
			lon = v.GetDouble();
		}
	}

	if (value.HasMember(STAT_NAMES[4])) {
		rapidjson::Value &v = value[STAT_NAMES[4]];
		if (v.IsInt()) {
			alt = v.GetInt();
		}
	}

	if (value.HasMember(STAT_NAMES[5])) {
		rapidjson::Value &v = value[STAT_NAMES[5]];
		if (v.IsInt()) {
			rxnb = v.GetInt();
		}
	}

	if (value.HasMember(STAT_NAMES[6])) {
		rapidjson::Value &v = value[STAT_NAMES[6]];
		if (v.IsInt()) {
			rxok = v.GetInt();
		}
	}

	if (value.HasMember(STAT_NAMES[7])) {
		rapidjson::Value &v = value[STAT_NAMES[7]];
		if (v.IsInt()) {
			rxfw = v.GetInt();
		}
	}

	if (value.HasMember(STAT_NAMES[8])) {
		rapidjson::Value &v = value[STAT_NAMES[8]];
		if (v.IsInt()) {
			ackr = v.GetInt();
		}
	}

	if (value.HasMember(STAT_NAMES[9])) {
		rapidjson::Value &v = value[STAT_NAMES[9]];
		if (v.IsInt()) {
			dwnb = v.GetInt();
		}
	}

	if (value.HasMember(STAT_NAMES[10])) {
		rapidjson::Value &v = value[STAT_NAMES[10]];
		if (v.IsInt()) {
			txnb = v.GetInt();
		}
	}
	return errcode;
}

std::string GatewayStat::toJsonString() const
{
		std::stringstream ss;
		ss << "{"
			<< "\"gwid\":\"" << std::hex << gatewayId << std::dec
			<< "\", \"time\":\"" << time2string(t) << "\""
			<< ",\"lati\":" << std::fixed << std::setprecision(5) << lat
			<< ",\"long\":" << std::fixed << std::setprecision(5) << lon
			<< ",\"alti\":" << alt
			<< ",\"rxnb\":" << rxnb
			<< ",\"rxok\":" << rxok
			<< ",\"rxfw\":" << rxfw
			<< ",\"ackr\":" << std::fixed << std::setprecision(1) << ackr
			<< ",\"dwnb\":" << dwnb
			<< ",\"txnb\":" << txnb
			<< "}";
		return ss.str();
}

std::string GatewayStat::toString() const
{
		std::stringstream ss;
		ss << std::hex << gatewayId << std::dec
			<< " " << time2string(t)
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
