#ifndef GATEWAY_LIST_H_
#define GATEWAY_LIST_H_	1

#include <vector>
#include <map>
#include <inttypes.h>

#include "rapidjson/document.h"

#include "gateway-stat.h"

class GatewayList {
	public:
		std::map<uint64_t, GatewayStat> statistics;
		GatewayList();
		GatewayList(const GatewayList &value);
		void toJSON(rapidjson::Value &value, rapidjson::Document::AllocatorType& allocator) const;
		int parse(
			uint64_t gatewayId,
			rapidjson::Value &value
		);
		int parse(
			const std::string &value
		);
		std::string toJsonString() const;
};

#endif
