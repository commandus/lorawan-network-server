#include "config-json.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

void ServerConfig::clear() {
	listenAddress = "";
	verbosity = 0;
	daemonize = false;
}

int ServerConfig::parse(
	rapidjson::Value &value
) {
	clear();
	if (value.HasMember("listenAddress")) {
		rapidjson::Value &address =  value["listenAddress"];
		if (address.IsString())
			listenAddress = address.GetString();
	}
	if (value.HasMember("verbosity")) {
		rapidjson::Value &verbose =  value["verbosity"];
		if (verbose.IsString())
			verbosity = verbose.GetInt();
	}
	if (value.HasMember("deamonize")) {
		rapidjson::Value &daemon =  value["deamonize"];
		if (daemon.IsString())
			daemonize = daemon.GetBool();
	}
	return 0;
}

void ServerConfig::toJson(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
) {
	value.SetObject();
	
	rapidjson::Value address;
	address.SetString(listenAddress.c_str(), listenAddress.length(), allocator);
	value.AddMember("listenAddress", address, allocator);

	rapidjson::Value verbose;
	verbose.SetInt(verbosity);
	value.AddMember("verbosity", verbose, allocator);

	rapidjson::Value deamon;
	deamon.SetBool(verbosity);
	value.AddMember("deamonize", deamon, allocator);

	rapidjson::Value &daemon =  value[""];
	if (daemon.IsString())
		daemonize = daemon.GetBool();
}

int Configuration::parse(
	const char* json
) {
	int r = 0;
	if (!json)
		return 1;
	rapidjson::Document d;
	d.Parse(json);
	if (d.IsObject() && d.HasMember("server")) {
		rapidjson::Value &server = d["server"];
		r |= serverConfig.parse(server);
	}
	return r;
}


Configuration::Configuration() {
}

Configuration::Configuration(
	const char* value
) {
	parse(value);
}

void Configuration::clear() {
	serverConfig.clear();
}

std::string Configuration::toString() {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	rapidjson::Document doc;
	rapidjson::Value server;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	serverConfig.toJson(server, allocator);
	doc.AddMember("server", server, allocator);
	doc.Accept(writer);
	return std::string(buffer.GetString());
}
