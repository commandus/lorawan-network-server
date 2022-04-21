#include "config-json.h"

#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#pragma clang diagnostic pop

#include "errlist.h"

#define DEF_BUFFER_SIZE	4096

static std::string messageQueueStorageType2String(MESSAGE_QUEUE_STORAGE value) {
	switch(value) {
		case MESSAGE_QUEUE_STORAGE_DIR_TEXT:
			return "txt";
		case MESSAGE_QUEUE_STORAGE_LMDB:
			return "lmdb";
		default:
			return "json";
	}
}

static MESSAGE_QUEUE_STORAGE string2messageQueueStorageType
(
	const std::string &value
) {
	if (value == "txt")
		return MESSAGE_QUEUE_STORAGE_DIR_TEXT;
	if (value == "lmdb")
		return MESSAGE_QUEUE_STORAGE_LMDB;
	return MESSAGE_QUEUE_STORAGE_JSON;
}

static int string2messageQueueDirFormat
(
	const std::string &value
) {
	if (value == "hex")
		return 1;
	if (value == "base64")
		return 2;
	return 0;	// bin
}

void ServerConfig::clear()
{
	listenAddressIPv4.clear();
	listenAddressIPv6.clear();
	readBufferSize = DEF_BUFFER_SIZE;
	verbosity = 0;
	controlFPort = 0;
	daemonize = false;
	logGWStatisticsFileName = "";
	logDeviceStatisticsFileName = "";
}

ServerConfig::ServerConfig() 
	: readBufferSize(DEF_BUFFER_SIZE), verbosity(0), controlFPort(0), daemonize(false),
      identityStorageName(""), deviceHistoryStorageName(""),
      regionalSettingsStorageName(""), regionalSettingsChannelPlanName(""),
      queueStorageName(""), storageType(IDENTITY_STORAGE_FILE_JSON),
      gwStatStorageType(GW_STAT_NONE), deviceStatStorageType(DEVICE_STAT_NONE),
      messageQueueType(MESSAGE_QUEUE_STORAGE_JSON), messageQueueDirFormat(0),
      logGWStatisticsFileName(""), logDeviceStatisticsFileName("")
{
}

int ServerConfig::parse(
	rapidjson::Value &value
) {
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
	if (value.HasMember("deviceHistoryStorageName")) {
		rapidjson::Value &jn = value["deviceHistoryStorageName"];
		if (jn.IsString()) {
            deviceHistoryStorageName = jn.GetString();
		}
	}
    if (value.HasMember("regionalSettingsStorageName")) {
        rapidjson::Value &jn = value["regionalSettingsStorageName"];
        if (jn.IsString()) {
            regionalSettingsStorageName = jn.GetString();
        }
    }
    if (value.HasMember("regionalSettingsChannelPlanName")) {
        rapidjson::Value &jn = value["regionalSettingsChannelPlanName"];
        if (jn.IsString()) {
            regionalSettingsChannelPlanName = jn.GetString();
        }
    }

	if (value.HasMember("queueStorageName")) {
		rapidjson::Value &jn = value["queueStorageName"];
		if (jn.IsString()) {
			queueStorageName = jn.GetString();
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
	if (value.HasMember("controlFPort")) {
		rapidjson::Value &cFPort =  value["controlFPort"];
		if (cFPort.IsInt())
			controlFPort = cFPort.GetInt();
	}
	if (value.HasMember("daemonize")) {
		rapidjson::Value &daemon =  value["daemonize"];
		if (daemon.IsBool())
			daemonize = daemon.GetBool();
	}
	if (value.HasMember("storageType")) {
		rapidjson::Value &vstorageType =  value["storageType"];
		if (vstorageType.IsString())
			storageType = string2storageType(vstorageType.GetString());
	}

    if (value.HasMember("gwStatStorageType")) {
        rapidjson::Value &vgwStatStorageType =  value["gwStatStorageType"];
        if (vgwStatStorageType.IsString())
            gwStatStorageType = string2gwStatStorageType(vgwStatStorageType.GetString());
    }

	if (value.HasMember("deviceStatStorageType")) {
		rapidjson::Value &vdeviceStatStorageType =  value["deviceStatStorageType"];
		if (vdeviceStatStorageType.IsString())
			deviceStatStorageType = string2deviceStatStorageType(vdeviceStatStorageType.GetString());
	}

	if (value.HasMember("messageQueueStorageType")) {
		rapidjson::Value &vstorageType =  value["messageQueueStorageType"];
		if (vstorageType.IsString())
			messageQueueType = string2messageQueueStorageType(vstorageType.GetString());
	}

	// 0- bin, 1- hex, 2- base64
	if (value.HasMember("messageQueueDirFormat")) {
		rapidjson::Value &vmessageQueueDirFormat =  value["messageQueueDirFormat"];
		if (vmessageQueueDirFormat.IsString())
			messageQueueDirFormat = string2messageQueueDirFormat(vmessageQueueDirFormat.GetString());
		if (vmessageQueueDirFormat.IsInt())
			messageQueueDirFormat = vmessageQueueDirFormat.GetInt() & 3;
	}

	if (value.HasMember("logGWStatisticsFileName")) {
		rapidjson::Value &jn = value["logGWStatisticsFileName"];
		if (jn.IsString()) {
			logGWStatisticsFileName = jn.GetString();
		}
	}

	if (value.HasMember("logDeviceStatisticsFileName")) {
		rapidjson::Value &jn = value["logDeviceStatisticsFileName"];
		if (jn.IsString()) {
			logDeviceStatisticsFileName = jn.GetString();
		}
	}

	if (value.HasMember("netId")) {
		rapidjson::Value &vnetid = value["netId"];
		if (vnetid.IsString()) {
			netid.set(vnetid.GetString());
		} else {
			if (vnetid.IsNumber()) {
				netid.set(vnetid.GetInt());
			}
		}
	}
	return LORA_OK;
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

	rapidjson::Value nds;
	nds.SetString(deviceHistoryStorageName.c_str(), deviceHistoryStorageName.size(), allocator);
	value.AddMember("deviceHistoryStorageName", nds, allocator);

    rapidjson::Value nrs;
    nrs.SetString(regionalSettingsStorageName.c_str(), regionalSettingsStorageName.size(), allocator);
    value.AddMember("regionalSettingsStorageName", nrs, allocator);

    rapidjson::Value nrscp;
    nrs.SetString(regionalSettingsChannelPlanName.c_str(), regionalSettingsChannelPlanName.size(), allocator);
    value.AddMember("regionalSettingsChannelPlanName", nrscp, allocator);

	rapidjson::Value nq;
	nq.SetString(queueStorageName.c_str(), queueStorageName.size(), allocator);
	value.AddMember("queueStorageName", nq, allocator);

	rapidjson::Value rbs;
	rbs.SetInt(readBufferSize);
	value.AddMember("readBufferSize", rbs, allocator);

	rapidjson::Value verbose;
	verbose.SetInt(verbosity);
	value.AddMember("verbosity", verbose, allocator);

	rapidjson::Value cFPort;
	cFPort.SetInt(controlFPort);
	value.AddMember("controlFPort", cFPort, allocator);

	rapidjson::Value deamon;
	deamon.SetBool(daemonize);
	value.AddMember("daemonize", deamon, allocator);

	rapidjson::Value vstorageType;
	std::string s(storageType2String(storageType));
	vstorageType.SetString(s.c_str(), s.size(), allocator);
	value.AddMember("storageType", vstorageType, allocator);

    rapidjson::Value vgwStatStorageType;
    std::string s0(gwStatStorageType2String(gwStatStorageType));
    vgwStatStorageType.SetString(s0.c_str(), s0.size(), allocator);
    value.AddMember("gwStatStorageType", vgwStatStorageType, allocator);

	rapidjson::Value vdeviceStatStorageType;
	std::string s1(deviceStatStorageType2String(deviceStatStorageType));
	vdeviceStatStorageType.SetString(s1.c_str(), s1.size(), allocator);
	value.AddMember("deviceStatStorageType", vdeviceStatStorageType, allocator);

	rapidjson::Value vMessageQueuestorageType;
	std::string s2(messageQueueStorageType2String(messageQueueType));
	vMessageQueuestorageType.SetString(s2.c_str(), s2.size(), allocator);
	value.AddMember("messageQueueStorageType", vMessageQueuestorageType, allocator);

	rapidjson::Value lgwsfn;
	lgwsfn.SetString(logGWStatisticsFileName.c_str(), logGWStatisticsFileName.size(), allocator);
	value.AddMember("logGWStatisticsFileName", lgwsfn, allocator);

	rapidjson::Value ldsfn;
	ldsfn.SetString(logGWStatisticsFileName.c_str(), logGWStatisticsFileName.size(), allocator);
	value.AddMember("logDeviceStatisticsFileName", ldsfn, allocator);

	rapidjson::Value vnetid;
	std::string sni = netid.toString();
	vnetid.SetString(sni.c_str(), sni.size(), allocator);
	value.AddMember("netId", vnetid, allocator);
}

int Configuration::parse(
	const char* json
) {
	int r = 0;
	if (!json)
		return ERR_CODE_INVALID_JSON;
	rapidjson::Document doc;
	doc.Parse(json);
	gatewayPort = 4242;
	if (doc.IsObject()) {
		if (doc.HasMember("server")) {
			rapidjson::Value &server = doc["server"];
			r |= serverConfig.parse(server);
		}
		if (doc.HasMember("ws")) {
			rapidjson::Value &ws = doc["ws"];
			r |= wsConfig.parse(ws);
		} else {
			wsConfig.enabled = false;
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
		if (doc.HasMember("databaseConfigFileName")) {
			rapidjson::Value &dbcfn =  doc["databaseConfigFileName"];
			if (dbcfn.IsString())
				databaseConfigFileName = dbcfn.GetString();
		}
        if (doc.HasMember("databaseExtraConfigFileNames")) {
            rapidjson::Value &dbcefns =  doc["databaseExtraConfigFileNames"];
            if (dbcefns.IsArray()) {
                for (int i = 0; i < dbcefns.Size(); i++) {
                    rapidjson::Value &dbcefn = dbcefns[i];
                    if (dbcefn.IsString()) {
                        databaseExtraConfigFileNames.push_back(dbcefn.GetString());
                    }
                }
            }
        }
		if (doc.HasMember("protoPath")) {
			rapidjson::Value &vpp =  doc["protoPath"];
			if (vpp.IsString())
				protoPath = vpp.GetString();
		}
		if (doc.HasMember("gatewayPort")) {
			rapidjson::Value &gwp =  doc["gatewayPort"];
			if (gwp.IsInt())
				gatewayPort = gwp.GetInt();
		}
	} else
		return ERR_CODE_INVALID_JSON;
	return r;
}

Configuration::Configuration() 
	: configFileName(""), gatewaysFileName(""), 
	databaseConfigFileName(""), protoPath(""), gatewayPort(4242)
{
}

Configuration::Configuration(
	const char* value
)
	: configFileName(""), gatewaysFileName("")
{
	parse(value);
}

void Configuration::clear() {
	configFileName = "";
	gatewaysFileName = "";
	databaseConfigFileName = "";
    databaseExtraConfigFileNames.clear();
	protoPath = "";
	serverConfig.clear();
}

std::string Configuration::toString() {
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	rapidjson::Document doc;
	doc.SetObject();
	
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	rapidjson::Value cfn;
	cfn.SetString(configFileName.c_str(), configFileName.length(), allocator);
	doc.AddMember("configFileName", cfn, allocator);

	rapidjson::Value gfn;
	gfn.SetString(gatewaysFileName.c_str(), gatewaysFileName.length(), allocator);
	doc.AddMember("gatewaysFileName", gfn, allocator);

	rapidjson::Value server;
	serverConfig.toJson(server, allocator);
	doc.AddMember("server", server, allocator);

	rapidjson::Value ws;
	wsConfig.toJson(ws, allocator);
	doc.AddMember("ws", ws, allocator);

	rapidjson::Value dbcfn;
	dbcfn.SetString(databaseConfigFileName.c_str(), databaseConfigFileName.size(), allocator);
	doc.AddMember("databaseConfigFileName", dbcfn, allocator);

    rapidjson::Value dbecfns;
    dbecfns.SetArray();
    for (std::vector<std::string>::const_iterator it(databaseExtraConfigFileNames.begin()); it != databaseExtraConfigFileNames.end(); it++) {
        rapidjson::Value fn;
        fn.SetString(it->c_str(), it->size(), allocator);
        dbecfns.PushBack(fn, allocator);
    }
    doc.AddMember("databaseExtraConfigFileNames", dbecfns, allocator);

	rapidjson::Value pp;
	pp.SetString(protoPath.c_str(), protoPath.size(), allocator);
	doc.AddMember("protoPath", pp, allocator);

	rapidjson::Value gwp;
	gwp.SetInt(gatewayPort);
	doc.AddMember("gatewayPort", gwp, allocator);

	doc.Accept(writer);
	return std::string(buffer.GetString());
}

std::string Configuration::toDescriptionTableString() const {
	std::stringstream ss;
	for (std::vector<std::string>::const_iterator it(serverConfig.listenAddressIPv4.begin()); it != serverConfig.listenAddressIPv4.end(); it++) {
		ss << "\t" << *it << std::endl;
	}
	for (std::vector<std::string>::const_iterator it(serverConfig.listenAddressIPv6.begin()); it != serverConfig.listenAddressIPv6.end(); it++) {
		ss << "\t" << *it << std::endl;
	}
	return ss.str();
}

/**
 * @brief web service configuration.
 * 
 * Example:
 * "ws": {
 * 		"enabled": true,
 *		"port": 5002,
 *		"html": "/home/andrei/src/lorawan-ws-angular/lorawan-ws-angular/dist/lorawan-ws-angular",
 *		"defaultDatabase": "sqlite-logger",
 *		"databases": []
 *	}
 */
WebServiceConfig::WebServiceConfig()
	: enabled(true), port(DEF_WS_PORT), html(""), defaultDatabase(""),
	threadCount(2),	connectionLimit(1024), flags(0)
{

}

void WebServiceConfig::clear()
{
	enabled = true;
	port = DEF_WS_PORT;
	html = "";
	defaultDatabase = "";
	databases.clear();
	threadCount = 2;
	connectionLimit = 1024;
	flags = 0;
}

int WebServiceConfig::parse(
	rapidjson::Value &value
)
{
	enabled = true;
	if (value.HasMember("enabled")) {
		rapidjson::Value &venabled = value["enabled"];
		if (venabled.IsBool()) {
			enabled = venabled.GetBool();
		}
	}

	port = DEF_WS_PORT;
	if (value.HasMember("port")) {
		rapidjson::Value &vport = value["port"];
		if (vport.IsInt()) {
			port = vport.GetInt();
		}
	}

	html = "";
	if (value.HasMember("html")) {
		rapidjson::Value &vhtml = value["html"];
		if (vhtml.IsString()) {
			html = vhtml.GetString();
		}
	}

	defaultDatabase = "";
	if (value.HasMember("defaultDatabase")) {
		rapidjson::Value &vdefaultDatabase = value["defaultDatabase"];
		if (vdefaultDatabase.IsString()) {
			defaultDatabase = vdefaultDatabase.GetString();
		}
	}

	databases.clear();
	if (value.HasMember("databases")) {
		rapidjson::Value &vdatabases = value["databases"];
		if (vdatabases.IsArray()) {
			for (int i = 0; i < vdatabases.Size(); i++) {
				rapidjson::Value &n = vdatabases[i];
				if (n.IsString())
					databases.push_back(n.GetString());
			}
		}
	}

	threadCount = 2;
	if (value.HasMember("threadCount")) {
		rapidjson::Value &vthreadCount = value["threadCount"];
		if (vthreadCount.IsInt()) {
			threadCount = vthreadCount.GetInt();
		}
	}

	connectionLimit = 1024;
	if (value.HasMember("connectionLimit")) {
		rapidjson::Value &vconnectionLimit = value["connectionLimit"];
		if (vconnectionLimit.IsInt()) {
			connectionLimit = vconnectionLimit.GetInt();
		}
	}

	flags = 0;
	if (value.HasMember("flags")) {
		rapidjson::Value &vflags = value["flags"];
		if (vflags.IsInt()) {
			flags = vflags.GetInt();
		}
	}

	return LORA_OK;
}

void WebServiceConfig::toJson(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
)
{
	value.SetObject();

	rapidjson::Value vEnabled;
	vEnabled.SetBool(enabled);
	vEnabled.AddMember("enabled", vEnabled, allocator);

	rapidjson::Value vPort;
	vPort.SetInt(port);
	vPort.AddMember("port", vPort, allocator);

	rapidjson::Value vHtml;
	vHtml.SetString(html.c_str(), html.size(), allocator);
	value.AddMember("html", vHtml, allocator);

	rapidjson::Value vDefaultDatabase;
	vDefaultDatabase.SetString(defaultDatabase.c_str(), defaultDatabase.size(), allocator);
	value.AddMember("defaultDatabase", vDefaultDatabase, allocator);

	rapidjson::Value vdatabases;
	vdatabases.SetArray();
	for (std::vector<std::string>::const_iterator it(databases.begin()); it != databases.end(); it++) {
		rapidjson::Value name;
		name.SetString(it->c_str(), it->size(), allocator);
		vdatabases.PushBack(name, allocator);
	}
	value.AddMember("databases", vdatabases, allocator);

	rapidjson::Value vThreadCount;
	vThreadCount.SetInt(threadCount);
	vThreadCount.AddMember("threadCount", vThreadCount, allocator);

	rapidjson::Value vConnectionLimit;
	vConnectionLimit.SetInt(connectionLimit);
	vConnectionLimit.AddMember("connectionLimit", vConnectionLimit, allocator);

	rapidjson::Value vFlags;
	vFlags.SetInt(flags);
	vFlags.AddMember("flags", vFlags, allocator);
}
