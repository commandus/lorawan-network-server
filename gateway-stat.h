#ifndef GATEWAY_STAT_H_
#define GATEWAY_STAT_H_	1

#include <time.h>
#include <inttypes.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

/* 
 * PUSH_DATA (0) stat Section 4
 * {"stat":{"time":"2021-02-24 04:54:01 GMT","lati":62.02774,"long":129.72883,"alti":348,"rxnb":0,"rxok":0,"rxfw":0,"ackr":0.0,"dwnb":0,"txnb":0}}
 */

class GatewayStat {
public:
	std::string name;
	uint64_t gatewayId;
	std::string addr;
	int errcode;
	time_t t;					// UTC time of pkt RX, us precision, ISO 8601 'expanded' format e.g. 2021-02-24 04:54:01 GMT
	double lat;					// GPS latitude of the gateway in degree (float, N is +)
	double lon;					// GPS longitude of the gateway in degree (float, E is +)
	uint32_t alt;				// altitude, meters, integer
	size_t rxnb;				// Number of radio packets received (unsigned integer)
	size_t rxok;				// Number of radio packets received with a valid PHY CRC
	size_t rxfw;				// Number of radio packets forwarded (unsigned integer)
	double ackr;				// Percentage of upstream datagrams that were acknowledged
	size_t dwnb;				// Number of downlink datagrams received (unsigned integer)
	size_t txnb;				// Number of packets emitted (unsigned integer)

	GatewayStat();
	GatewayStat(const GatewayStat &value);
	bool operator==(GatewayStat &rhs) const;

	void toJSON(rapidjson::Value &value, rapidjson::Document::AllocatorType& allocator) const;
	int parse(
		rapidjson::Value &value
	);
	std::string toJsonString() const;
	std::string toString() const;
};

#endif
