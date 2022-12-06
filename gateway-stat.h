#ifndef GATEWAY_STAT_H_
#define GATEWAY_STAT_H_	1

#include <string>
#include <time.h>
#include <inttypes.h>
#ifdef _MSC_VER
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#define SOCKET int
#include <netinet/in.h>
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/document.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

/** 
 * Gateway stat keep current gateway address got from the last PULL request.
 * There are also addr member to keep gateway address read from configuration file. This addr must never used.
 * Gateway identified by 8 bit identifier.
 * 
 * PUSH_DATA (0) stat Section 4
 * {"stat":{"time":"2021-02-24 04:54:01 GMT","lati":62.02774,"long":129.72883,"alti":348,"rxnb":0,"rxok":0,"rxfw":0,"ackr":0.0,"dwnb":0,"txnb":0}}
 */
class GatewayStat {
public:
	std::string name;
	uint64_t gatewayId;
	// host name or address of the gateway
	std::string addr;
	// socket
	SOCKET socket;
	// Gateway send PULL_DATA packet to inform network server what gateway current address and port are (possibly over NAT)
	struct sockaddr_in6 sockaddr;
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
