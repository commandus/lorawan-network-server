#include "gateway-list.h"
#include "errlist.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#pragma clang diagnostic pop

#include "utilstring.h"

GatewayList::GatewayList()
{

}

GatewayList::GatewayList(
	const GatewayList &value
)
{
	filename = value.filename;
	gateways = value.gateways;
}

GatewayList::GatewayList(
	const std::string &afilename
)
	: filename(afilename)
{
	parse(file2string(afilename.c_str()));
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
		if (stat.parse(value[i])) {
			if (gatewayId)
				stat.gatewayId = gatewayId;
			gateways[stat.gatewayId] = stat;
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
