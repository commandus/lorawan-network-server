#ifndef CONFIG_JSON_H
#define CONFIG_JSON_H 1

#include <string>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class ServerConfig {
	public:
		std::vector<std::string> listenAddressIPv4;
		std::vector<std::string> listenAddressIPv6;
		int readBufferSize;
		int verbosity;
		bool daemonize;
		std::string identityStorageName;
		void clear();
		ServerConfig();
		int parse(
			rapidjson::Value &value
		);
		void toJson(
			rapidjson::Value &value,
			rapidjson::Document::AllocatorType& allocator
		);
};

class Configuration {
	public:
		std::string configFileName;
		std::string gatewaysFileName;
		ServerConfig serverConfig;
		Configuration();
		Configuration(const char* value);
		int parse(
			const char* json
		);
		void clear();
		std::string toString();
};

#endif
