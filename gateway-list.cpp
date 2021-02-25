#include "gateway-list.h"
#include "errlist.h"

#include "rapidjson/writer.h"

GatewayList::GatewayList()
{

}

GatewayList::GatewayList(
	const GatewayList &value
)
{
	statistics = value.statistics;
}

void GatewayList::toJSON(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
) const
{
	for (std::map<uint64_t, GatewayStat>::const_iterator it(statistics.begin()); it != statistics.end(); it++) {
		rapidjson::Value v;
		v.SetObject();
		it->second.toJSON(v, allocator);
		value.PushBack(v, allocator);
	}
}

int GatewayList::parse(
	const std::string &value
)
{
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator(doc.GetAllocator());
	doc.Parse(value.c_str());
	if (!doc.IsArray())
		return ERR_CODE_INVALID_JSON;
	return parse(0, doc);
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
			statistics[stat.gatewayId] = stat;
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
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	return std::string(buffer.GetString());
}
