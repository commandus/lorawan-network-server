#ifndef CONFIG_JSON_H
#define CONFIG_JSON_H 1

#include <string>
#include <vector>

#include "utilidentity.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#include "gateway-stat-service-abstract.h"
#include "device-stat-service-abstract.h"
#include "device-history-service-abstract.h"

#pragma clang diagnostic pop

typedef enum {
	MESSAGE_QUEUE_STORAGE_JSON = 1,
	MESSAGE_QUEUE_STORAGE_DIR_TEXT = 2,
	MESSAGE_QUEUE_STORAGE_LMDB = 3
} MESSAGE_QUEUE_STORAGE;

class ServerConfig {
	public:
		std::vector<std::string> listenAddressIPv4;
		std::vector<std::string> listenAddressIPv6;
		int readBufferSize;
		int verbosity;
		uint8_t controlFPort; // 0- no remote conttrol allowed, 1 .. 223
		bool daemonize;
		std::string identityStorageName;
		std::string queueStorageName;
		std::string deviceHistoryStorageName;
		IDENTITY_STORAGE storageType;
        GW_STAT_STORAGE gwStatStorageType;
		DEVICE_STAT_STORAGE deviceStatStorageType;
		MESSAGE_QUEUE_STORAGE messageQueueType;
		int messageQueueDirFormat;					// 0- bin, 1- hex, 2- base64
		std::string logGWStatisticsFileName;
		std::string logDeviceStatisticsFileName;

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
		std::string databaseConfigFileName;
		std::string protoPath;
		int gatewayPort;
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
