#include <regex>
#include <sstream>

#include "gateway-list.h"
#include "errlist.h"
#include "utillora.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#pragma clang diagnostic pop

#include "utilstring.h"
#include "platform.h"

/**
 * Create empty gateway list
 */
GatewayList::GatewayList()
	: errmessage(""), filename("")
{

}

/**
 * Copy gateways
 */
GatewayList::GatewayList(
	const GatewayList &value
)
{
	errmessage = value.errmessage;
	filename = value.filename;
	gateways = value.gateways;
}

/**
 * Loads gateways fron JSON file
 */
GatewayList::GatewayList(
	const std::string &afilename
)
	: errmessage(""), filename(afilename)
{
	parse(file2string(filename.c_str()));
}

/**
 * Serialize gateway list to JSON
 */
void GatewayList::toJSON(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
) const
{
	for (std::map<uint64_t, GatewayStat>::const_iterator it(gateways.begin()); it != gateways.end(); it++) {
		rapidjson::Value v;
		v.SetObject();
		it->second.toJSON(v, allocator);
		value.PushBack(v, allocator);
	}
}

/**
 * Add gateway to the list frm JSON string
 */
int GatewayList::put(
	uint64_t gatewayId,
	const std::string &value
)
{
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator(doc.GetAllocator());
	doc.Parse(value.c_str());
	return parse(gatewayId, doc);
}

/**
 * Return 'config' gateway address
 * It is different than 'current' address extracted from last successful PULL request
 * @param gatewayId gateway identifier
 */
std::string GatewayList::getAddress(
	uint64_t gatewayId
) {
	std::map<uint64_t, GatewayStat>::iterator it(gateways.find(gatewayId));
	if (it != gateways.end())
		return it->second.addr;
	else
		return "";
}

/**
 * Update list from statistic packet
 */
bool GatewayList::update(
	const GatewayStat &value
)
{
	std::map<uint64_t, GatewayStat>::iterator it(gateways.find(value.gatewayId));
	// save anyway
	gateways[value.gatewayId] = value;
	// indicate it is new one
	return it != gateways.end();
}

/**
 * Parse JSON
 */
int GatewayList::parse(
	uint64_t gatewayId,
	rapidjson::Value &value
)
{
	if (!value.IsArray())
		return ERR_CODE_INVALID_JSON;
	for (int i = 0; i < value.Size(); i++) {
		GatewayStat stat;
		if (int r = stat.parse(value[i])) {
			errmessage = strerror_lorawan_ns(r);
		} else {
			if (gatewayId)
				stat.gatewayId = gatewayId;
			gateways[stat.gatewayId] = stat;
		}
	}
	return 0;
}

/**
 * Return gateway list subset filters by gateway identifiers
 * @param list gateway identifiers. You can use wildcards '*', '?'.
 * @param useRegex true- list contains wildcards. Symbols '*', '?' are replaced with '.*' and '.' 
 * regular expression symbol respectively.
 * @return 0- success, < 0- error code
 */
int GatewayList::parseIdentifiers(
	std::vector<uint64_t> &retval,
	const std::vector<std::string> &list,
	bool useRegex
) const
{
	for (std::vector<std::string>::const_iterator it(list.begin()); it != list.end(); it++) {
		if (isHex(*it)) {
			// identifier itself
			uint64_t v = str2gatewayId(it->c_str());
			if (gateways.find(v) == gateways.end()) {
				return false;
			} else {
				retval.push_back(v);
			}
		} else {
			// can contain regex "*"
			try {
				std::string re;
				if (useRegex)
					re = *it;
				else
					re = replaceAll(replaceAll(*it, "*", ".*"), "?", ".");
				std::regex rex(re, std::regex_constants::grep);
				for (std::map<uint64_t, GatewayStat>::const_iterator itg(gateways.begin()); itg != gateways.end(); itg++) {
					std::string s2 = uint64_t2string(itg->first);
					if (std::regex_search(s2, rex))
						retval.push_back(itg->first);
				}
		    }
    		catch (const std::regex_error& e) {
				return ERR_CODE_INVALID_REGEX;
				break;
			}
		}
	}
	return 0;
}

/**
 * Return gateway list subset filters by gateway name
 * @param list gateway names (case sensitive). You can use wildcards '*', '?'.
 * @param useRegex true- list contains wildcards. Symbols '*', '?' are replaced with '.*' and '.' 
 * regular expression symbol respectively.
 * @return 0- success, < 0- error code
 */
int GatewayList::parseNames(
	std::vector<uint64_t> &retval,
	const std::vector<std::string> &list,
	bool useRegex
) const
{
	for (std::vector<std::string>::const_iterator it(list.begin()); it != list.end(); it++) {
		if (isHex(*it)) {
			// identifier itself
			uint64_t v = str2gatewayId(it->c_str());
			if (gateways.find(v) == gateways.end()) {
				return false;
			} else {
				retval.push_back(v);
			}
		} else {
			// can contain regex "*"
			try {
				std::string re;
				if (useRegex)
					re = *it;
				else
					re = replaceAll(replaceAll(*it, "*", ".*"), "?", ".");
				std::regex rex(re, std::regex_constants::grep);
				for (std::map<uint64_t, GatewayStat>::const_iterator itg(gateways.begin()); itg != gateways.end(); itg++) {
					if (std::regex_search(itg->second.name, rex))
						retval.push_back(itg->first);
				}
		    }
    		catch (const std::regex_error& e) {
				return ERR_CODE_INVALID_REGEX;
				break;
			}
		}
	}
	return 0;
}

/**
 * @return JSON string
 */
std::string GatewayList::toJsonString() const
{
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
	doc.SetArray();
	toJSON(doc, allocator);
	
	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	return std::string(buffer.GetString());
}

/**
 * Parse JSON string
 */
void GatewayList::parse(
	const std::string &value
)
{
	rapidjson::Document doc;
	doc.Parse(value.c_str());
	parse(0, doc);
}

/**
 * Save to JSON file
 */
void GatewayList::save()
{
	string2file(filename, toJsonString());
}

/**
 * Check does gateway exists
 * @param gwid gateway identifier
 * @return true is gateway exists
 */
bool GatewayList::has(
	const DEVEUI &gwid
) const
{
	uint64_t k = *(uint64_t *) &gwid;
	return (gateways.find(k) != gateways.end());
}

/**
 * Check does gateway exists
 * @param gwid gateway identifier
 * @return true is gateway exists
 */
bool GatewayList::has(
	const uint64_t gwid
) const
{
	return (gateways.find(gwid) != gateways.end());
}

/**
 * Set gateway socket address
 * Gateway send PULL request from random port number each minute. 
 * This port number is opened for communication until next PULL request.
 * Network server must remember this port and then send packet to this port number.
 * Please note gateway address can be different from assigned address in configuration file because of NAT.
 * @param gwid gateway identifier
 * @param gwAddress gateway socket address to be set(update)
 * @return true if gateway with gwid identifier found and successfully updated
 */
bool GatewayList::setSocketAddress
(
	const DEVEUI &gwid,
	int socket,
	const struct sockaddr_in *gwAddress
)
{
	uint64_t k = *(uint64_t *) &gwid;
	std::map<uint64_t, GatewayStat>::iterator it(gateways.find(k));
	if (it == gateways.end())
		return false;
	memmove(&it->second.sockaddr, gwAddress,
		(gwAddress->sin_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)));
	it->second.socket = socket;
	return true;
}

/**
 * Set gateway socket address into parameter value from gwAddress parameter and copy name and geolocation
 * from config file
 * Gateway send PULL request from random port number each minute. 
 * This port number is opened for communication until next PULL request.
 * Network server must remember this port and then send packet to this port number.
 * Please note gateway address can be different from assigned address in configuration file because of NAT.
 * @param gwAddress gateway socket address to be set(update)
 * @param value Set gateway stat socket address, name and geolocation (if gateway found).
 * @return true if found
 */
bool GatewayList::copyId
(
	GatewayStat &value,
	const struct sockaddr *gwAddress
) const
{
	uint64_t k = *(uint64_t *) &value.gatewayId;
	std::map<uint64_t, GatewayStat>::const_iterator it(gateways.find(k));
	if (it == gateways.end())
		return false;
	value.name = it->second.name;
	value.lat = it->second.lat;
	value.lon = it->second.lon;
	value.addr = UDPSocket::addrString(gwAddress);
	return true;
}

std::string GatewayList::toDescriptionTableString() const {
    std::stringstream ss;
    ss << std::endl;
    for (std::map<uint64_t, GatewayStat>::const_iterator it(gateways.begin()); it != gateways.end(); it++) {
        ss << "\t" << it->second.name
            << "\t" << gatewayId2str(it->second.gatewayId)
            << "\t" << it->second.addr
            << std::endl;
    }
    return ss.str();
}
