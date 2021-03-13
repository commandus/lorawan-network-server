#include "config-json.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#pragma clang diagnostic pop

#include "errlist.h"

#define DEF_BUFFER_SIZE	4096

void ServerConfig::clear() {
	listenAddressIPv4.clear();
	listenAddressIPv6.clear();
	readBufferSize = DEF_BUFFER_SIZE;
	verbosity = 0;
	daemonize = false;
}

ServerConfig::ServerConfig() 
	: readBufferSize(DEF_BUFFER_SIZE), verbosity(0), daemonize(false), identityStorageName("")
{

}

int ServerConfig::parse(
	rapidjson::Value &value
) {
	clear();
	if (value.HasMember("listenAddressIPv4")) {
		rapidjson::Value &addresses = value["listenAddressIPv4"];
		if (addresses.IsArray()) {
			for (int i = 0; i < addresses.Size(); i++) {
				rapidjson::Value &address = addresses[i];
				if (address.IsString())
					listenAddressIPv4.push_back(address.GetString());
			}
		}
	}
	if (value.HasMember("listenAddressIPv6")) {
		rapidjson::Value &addresses = value["listenAddressIPv6"];
		if (addresses.IsArray()) {
			for (int i = 0; i < addresses.Size(); i++) {
				rapidjson::Value &address = addresses[i];
				if (address.IsString())
					listenAddressIPv6.push_back(address.GetString());
			}
		}
	}
	if (value.HasMember("identityStorageName")) {
		rapidjson::Value &jn = value["identityStorageName"];
		if (jn.IsString()) {
			identityStorageName = jn.GetString();
		}
	}
	if (value.HasMember("readBufferSize")) {
		rapidjson::Value &rbs =  value["readBufferSize"];
		if (rbs.IsInt())
			readBufferSize = rbs.GetInt();
			if (readBufferSize <= 0)
				readBufferSize = DEF_BUFFER_SIZE;
	}
	if (value.HasMember("verbosity")) {
		rapidjson::Value &verbose =  value["verbosity"];
		if (verbose.IsInt())
			verbosity = verbose.GetInt();
	}
	if (value.HasMember("daemonize")) {
		rapidjson::Value &daemon =  value["daemonize"];
		if (daemon.IsBool())
			daemonize = daemon.GetBool();
	}
	return 0;
}

void ServerConfig::toJson(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
) {
	value.SetObject();

	rapidjson::Value addressesIPv4;
	addressesIPv4.SetArray();
	for (std::vector<std::string>::const_iterator it(listenAddressIPv4.begin()); it != listenAddressIPv4.end(); it++) {
		rapidjson::Value address;
		address.SetString(it->c_str(), it->size(), allocator);
		addressesIPv4.PushBack(address, allocator);

	}
	value.AddMember("listenAddressIPv4", addressesIPv4, allocator);

	rapidjson::Value addressesIPv6;
	addressesIPv6.SetArray();
	for (std::vector<std::string>::const_iterator it(listenAddressIPv6.begin()); it != listenAddressIPv6.end(); it++) {
		rapidjson::Value address;
		address.SetString(it->c_str(), it->size(), allocator);
		addressesIPv6.PushBack(address, allocator);

	}
	value.AddMember("listenAddressIPv6", addressesIPv6, allocator);

	rapidjson::Value n;
	n.SetString(identityStorageName.c_str(), identityStorageName.size(), allocator);
	value.AddMember("identityStorageName", n, allocator);

	rapidjson::Value rbs;
	rbs.SetInt(readBufferSize);
	value.AddMember("readBufferSize", rbs, allocator);

	rapidjson::Value verbose;
	verbose.SetInt(verbosity);
	value.AddMember("verbosity", verbose, allocator);

	rapidjson::Value deamon;
	deamon.SetBool(daemonize);
	value.AddMember("daemonize", deamon, allocator);
}

int Configuration::parse(
	const char* json
) {
	int r = 0;
	if (!json)
		return ERR_CODE_INVALID_JSON;
	rapidjson::Document doc;
	doc.Parse(json);
	if (doc.IsObject()) {
		if (doc.HasMember("server")) {
			rapidjson::Value &server = doc["server"];
			r |= serverConfig.parse(server);
		}
		if (doc.HasMember("configFileName")) {
			rapidjson::Value &cfn =  doc["configFileName"];
			if (cfn.IsString())
				configFileName = cfn.GetString();
		}
		if (doc.HasMember("gatewaysFileName")) {
			rapidjson::Value &gfn =  doc["gatewaysFileName"];
			if (gfn.IsString())
				gatewaysFileName = gfn.GetString();
		}
	}
	return r;
}

Configuration::Configuration() 
	: configFileName(""), gatewaysFileName("")
{
}

Configuration::Configuration(
	const char* value
) {
	parse(value);
}

void Configuration::clear() {
	configFileName = "";
	gatewaysFileName = "";
	serverConfig.clear();
}

std::string Configuration::toString() {
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Value server;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	rapidjson::Value cfn;
	cfn.SetString(configFileName.c_str(), configFileName.length(), allocator);
	doc.AddMember("configFileName", cfn, allocator);

	rapidjson::Value gfn;
	gfn.SetString(gatewaysFileName.c_str(), gatewaysFileName.length(), allocator);
	doc.AddMember("gatewaysFileName", gfn, allocator);

	serverConfig.toJson(server, allocator);
	doc.AddMember("server", server, allocator);

	doc.Accept(writer);
	return std::string(buffer.GetString());
}
