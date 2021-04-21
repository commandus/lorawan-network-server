#ifndef CONFIG_JSON_H
#define CONFIG_JSON_H 1

#include <string>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

typedef enum {
	IDENTITY_STORAGE_FILE_JSON = 1,
	IDENTITY_STORAGE_DIR_TEXT = 2,
	IDENTITY_STORAGE_LMDB = 3
} IDENTITY_STORAGE;

class ServerConfig {
	public:
		std::vector<std::string> listenAddressIPv4;
		std::vector<std::string> listenAddressIPv6;
		int readBufferSize;
		int verbosity;
		bool daemonize;
		std::string identityStorageName;
		IDENTITY_STORAGE storageType;
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
