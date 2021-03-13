#ifndef GATEWAY_LIST_H_
#define GATEWAY_LIST_H_	1

#include <vector>
#include <map>
#include <inttypes.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

#include "gateway-stat.h"

class GatewayList {
	private:
		std::string filename;
	public:
		std::map<uint64_t, GatewayStat> gateways;
		GatewayList();
		GatewayList(const std::string &filename);
		GatewayList(const GatewayList &value);
		int put(uint64_t gatewayId, const std::string &value);
		bool update(const GatewayStat &value);
		int parse(uint64_t gatewayId, rapidjson::Value &value);
		void parse(const std::string &value);
		void toJSON(rapidjson::Value &value, rapidjson::Document::AllocatorType& allocator) const;
		std::string toJsonString() const;
		void save();
};

#endif
