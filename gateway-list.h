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
#include "utillora.h"
#include "udp-socket.h"

/**
 * Gateways list of gateways identifiers, addresses and statistics
 */
class GatewayList {
	private:
		std::string filename;
	public:
		std::map<uint64_t, GatewayStat> gateways;
		std::string errmessage;
		GatewayList();
		GatewayList(const std::string &filename);
		GatewayList(const GatewayList &value);
		int put(uint64_t gatewayId, const std::string &value);
		std::string getAddress(uint64_t gatewayId);
		bool update(const GatewayStat &value);
		int parse(uint64_t gatewayId, rapidjson::Value &value);
		void parse(const std::string &value);
		void toJSON(rapidjson::Value &value, rapidjson::Document::AllocatorType& allocator) const;
		std::string toJsonString() const;
		void save();
		bool validate(const std::vector<uint64_t> &list) const;
		int parseIdentifiers(
			std::vector<uint64_t> &retval,
			const std::vector<std::string> &value,
			bool useRegex
		) const;
		int parseNames(
			std::vector<uint64_t> &retval,
			const std::vector<std::string> &value,
			bool useRegex
		) const;
		bool setSocketAddress(
			const DEVEUI  &gwid,
			const struct sockaddr_in *gwAddress
		);
		bool has(
			const DEVEUI &gwid
		) const;
		bool has(
			const uint64_t gwid
		) const;
		// Set identifier, name and geolocation if found. Return true if found
		bool copyId(
			GatewayStat &value,
			const struct sockaddr *gwAddress
		) const;
};

#endif
