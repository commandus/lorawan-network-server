#ifndef CONFIG_JSON_H
#define CONFIG_JSON_H 1

#include <string>
#include <vector>

#include "utilidentity.h"
#ifdef ENABLE_LISTENER_USB
#include "gateway-file-json.h"
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/document.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "gateway-stat-service-abstract.h"
#include "device-stat-service-abstract.h"
#include "device-history-service-abstract.h"


typedef enum {
	MESSAGE_QUEUE_STORAGE_JSON = 1,
	MESSAGE_QUEUE_STORAGE_DIR_TEXT = 2,
	MESSAGE_QUEUE_STORAGE_LMDB = 3
} MESSAGE_QUEUE_STORAGE;

#define DEF_WS_PORT	5002

class ServerConfig {
	public:
		std::vector<std::string> listenAddressIPv4;
		std::vector<std::string> listenAddressIPv6;
		int readBufferSize;
		int verbosity;
		uint8_t controlFPort; // 0- no remote control allowed, 1 .. 223
		bool daemonize;
		std::string identityStorageName;
		std::string queueStorageName;
		std::string deviceHistoryStorageName;
        std::string regionalSettingsStorageName;
        std::string regionalSettingsChannelPlanName;
		IDENTITY_STORAGE storageType;
        GW_STAT_STORAGE gwStatStorageType;
		DEVICE_STAT_STORAGE deviceStatStorageType;
		MESSAGE_QUEUE_STORAGE messageQueueType;
		int messageQueueDirFormat;					// 0- bin, 1- hex, 2- base64
		std::string logGWStatisticsFileName;
		std::string logDeviceStatisticsFileName;
		NetId netid;

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

/**
 * @brief web service configuration.
 * 
 * Example:
 * "ws": {
 * 		"enabled": true,
 *		"port": 5002,
 *		"html": "/home/andrei/src/lorawan-ws-angular/lorawan-ws-angular/dist/lorawan-ws-angular",
 *		"default-database": "sqlite-logger",
 *		"databases": []
 *	}
 */
class WebServiceConfig {
	public:
		bool enabled;						///< enable or disable wev service on start. Default- enabled
		int port;							///< default port is 5002
		std::string html;					///< web root directory. If not set, only JSON API available
		std::string defaultDatabase;		///< default (no-name) database
		std::vector<std::string> databases;	///< named databases (db=<database name> parameter is required in JSON API)

		int threadCount;					///< If <=0, default 2
		int connectionLimit;				///< If <=0, default 1024
		uint32_t flags;						///< Default 0

        // JWT
        std::string jwtIssuer;
        std::string jwtSecret;
        std::string userPasswordListFileName;
		
		void clear();
		WebServiceConfig();
		int parse(
			rapidjson::Value &value
		);
		void toJson(
			rapidjson::Value &value,
			rapidjson::Document::AllocatorType& allocator
		);
        std::string toString();
};

class Configuration {
	public:
		std::string configFileName;
		std::string gatewaysFileName;
		std::string databaseConfigFileName;
        std::string pluginsPath;
		std::map<std::string, std::vector<std::string> > pluginsParams; ///< e.g. logger passport directories
		int gatewayPort;
		ServerConfig serverConfig;
		WebServiceConfig wsConfig;
#ifdef ENABLE_LISTENER_USB
        GatewayConfigFileJson gatewayConfig;
#endif
		Configuration();
		Configuration(const char* value);
		int parse(
			const char* json
		);
		void clear();
		std::string toString();
        std::string toDescriptionTableString() const;
};

#endif
