#include <regex>

#include "gateway-list.h"
#include "errlist.h"
#include "utillora.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#pragma clang diagnostic pop

#include "utilstring.h"

GatewayList::GatewayList()
	: errmessage(""), filename("")
{

}

GatewayList::GatewayList(
	const GatewayList &value
)
{
	errmessage = value.errmessage;
	filename = value.filename;
	gateways = value.gateways;
}

GatewayList::GatewayList(
	const std::string &afilename
)
	: errmessage(""), filename(afilename)
{
	parse(file2string(filename.c_str()));
}

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

std::string GatewayList::getAddress(
	uint64_t gatewayId
) {
	std::map<uint64_t, GatewayStat>::iterator it(gateways.find(gatewayId));
	if (it != gateways.end())
		return it->second.addr;
	else
		return "";
}

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
			errmessage = strerror_client(r);
		} else {
			if (gatewayId)
				stat.gatewayId = gatewayId;
			gateways[stat.gatewayId] = stat;
		}
	}
	return 0;
}

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

void GatewayList::parse(
	const std::string &value
)
{
	rapidjson::Document doc;
	doc.Parse(value.c_str());
	parse(0, doc);
}

void GatewayList::save()
{
	string2file(filename, toJsonString());
}
