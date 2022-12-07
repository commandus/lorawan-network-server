#include "config-json.h"

#include <iostream>
#include <sstream>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "errlist.h"
#include "utilstring.h"

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
			for (rapidjson::SizeType i = 0; i < addresses.Size(); i++) {
				rapidjson::Value &address = addresses[i];
				if (address.IsString())
					listenAddressIPv4.push_back(address.GetString());
			}
		}
	}
	if (value.HasMember("listenAddressIPv6")) {
		rapidjson::Value &addresses = value["listenAddressIPv6"];
		if (addresses.IsArray()) {
			for (rapidjson::SizeType i = 0; i < addresses.Size(); i++) {
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
		address.SetString(it->c_str(), (rapidjson::SizeType) it->size(), allocator);
		addressesIPv4.PushBack(address, allocator);
	}
	value.AddMember("listenAddressIPv4", addressesIPv4, allocator);

	rapidjson::Value addressesIPv6;
	addressesIPv6.SetArray();
	for (std::vector<std::string>::const_iterator it(listenAddressIPv6.begin()); it != listenAddressIPv6.end(); it++) {
		rapidjson::Value address;
		address.SetString(it->c_str(), (rapidjson::SizeType) it->size(), allocator);
		addressesIPv6.PushBack(address, allocator);
	}
	value.AddMember("listenAddressIPv6", addressesIPv6, allocator);

	rapidjson::Value n;
	n.SetString(identityStorageName.c_str(), (rapidjson::SizeType) identityStorageName.size(), allocator);
	value.AddMember("identityStorageName", n, allocator);

	rapidjson::Value nds;
	nds.SetString(deviceHistoryStorageName.c_str(), (rapidjson::SizeType) deviceHistoryStorageName.size(), allocator);
	value.AddMember("deviceHistoryStorageName", nds, allocator);

    rapidjson::Value nrs;
    nrs.SetString(regionalSettingsStorageName.c_str(), (rapidjson::SizeType) regionalSettingsStorageName.size(), allocator);
    value.AddMember("regionalSettingsStorageName", nrs, allocator);

    rapidjson::Value nrscp;
    nrs.SetString(regionalSettingsChannelPlanName.c_str(), (rapidjson::SizeType) regionalSettingsChannelPlanName.size(), allocator);
    value.AddMember("regionalSettingsChannelPlanName", nrscp, allocator);

	rapidjson::Value nq;
	nq.SetString(queueStorageName.c_str(), (rapidjson::SizeType) queueStorageName.size(), allocator);
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
	vstorageType.SetString(s.c_str(),(rapidjson::SizeType) s.size(), allocator);
	value.AddMember("storageType", vstorageType, allocator);

    rapidjson::Value vgwStatStorageType;
    std::string s0(gwStatStorageType2String(gwStatStorageType));
    vgwStatStorageType.SetString(s0.c_str(), (rapidjson::SizeType) s0.size(), allocator);
    value.AddMember("gwStatStorageType", vgwStatStorageType, allocator);

    rapidjson::Value vlogGWStatisticsFileName;
    vlogGWStatisticsFileName.SetString(logGWStatisticsFileName.c_str(), (rapidjson::SizeType) logGWStatisticsFileName.size());
    value.AddMember("logGWStatisticsFileName", vlogGWStatisticsFileName, allocator);

	rapidjson::Value vdeviceStatStorageType;
	std::string s1(deviceStatStorageType2String(deviceStatStorageType));
	vdeviceStatStorageType.SetString(s1.c_str(), (rapidjson::SizeType) s1.size(), allocator);
	value.AddMember("deviceStatStorageType", vdeviceStatStorageType, allocator);

    rapidjson::Value vlogDeviceStatisticsFileName;
    vlogDeviceStatisticsFileName.SetString(logDeviceStatisticsFileName.c_str(), (rapidjson::SizeType) logDeviceStatisticsFileName.size());
    value.AddMember("logDeviceStatisticsFileName", vlogDeviceStatisticsFileName, allocator);

	rapidjson::Value vMessageQueuestorageType;
	std::string s2(messageQueueStorageType2String(messageQueueType));
	vMessageQueuestorageType.SetString(s2.c_str(), (rapidjson::SizeType) s2.size(), allocator);
	value.AddMember("messageQueueStorageType", vMessageQueuestorageType, allocator);

    rapidjson::Value vmessageQueueDirFormat;
    std::string s4;
    switch (messageQueueDirFormat) {
        case 1:
            s = "hex";
            break;
        case 2:
            s = "base64";
            break;
        default:
            s = "bin";
    }
    vmessageQueueDirFormat.SetString(s.c_str(), (rapidjson::SizeType) s.size(), allocator);
    value.AddMember("messageQueueDirFormat", vmessageQueueDirFormat, allocator);

    rapidjson::Value vnetid;
	std::string sni = netid.toString();
	vnetid.SetString(sni.c_str(), (rapidjson::SizeType) sni.size(), allocator);
	value.AddMember("netId", vnetid, allocator);
}

int Configuration::parse(
	const char* json
) {
	int r = 0;
	if (!json)
		return ERR_CODE_INVALID_JSON;
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseCommentsFlag>(json);
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
        if (doc.HasMember("pluginsParams")) {
            rapidjson::Value &pp =  doc["pluginsParams"];
            if (pp.IsArray()) { // json array of array, first element is parameter name, next are values
                for (rapidjson::SizeType i = 0; i < pp.Size(); i++) {
                    rapidjson::Value &pp1 = pp[i];
                    if (pp1.IsArray()) {
                        size_t sz = pp1.Size();
                        if (sz < 0)
                            continue;   // no map name
                        rapidjson::Value &jn = pp1[0];
                        std::string pn;
                        if (jn.IsString()) {
                            pn = jn.GetString();
                        }
                        for (int j = 1; j < sz; j++) {
                            rapidjson::Value &jv = pp1[j];
                            if (!jv.IsString())
                                continue;
                            std::string pv = jv.GetString();
                            pluginsParams[pn].push_back(pv);
                        }
                    }
                }
            }
        }
        if (doc.HasMember("pluginsPath")) {
            rapidjson::Value &v =  doc["pluginsPath"];
            if (v.IsString())
                pluginsPath = v.GetString();
        }
		if (doc.HasMember("gatewayPort")) {
			rapidjson::Value &gwp =  doc["gatewayPort"];
			if (gwp.IsInt())
				gatewayPort = gwp.GetInt();
		}
#ifdef ENABLE_LISTENER_USB
        if (doc.HasMember("embeddedGatewayConfig")) {
			rapidjson::Value &gwc =  doc["embeddedGatewayConfig"];
			if (gwc.IsString()) {
				std::string path = gwc.GetString();
                if (!path.empty()) {
                    std::string v = file2string(path.c_str());
                    if (!v.empty()) {
                        gatewayConfig.parseString(v);
                    }
                }
            }
		}
#endif
	} else
		return ERR_CODE_INVALID_JSON;
	return r;
}

Configuration::Configuration() 
	: gatewayPort(4242)
{
}

Configuration::Configuration(
	const char* value
)
{
	parse(value);
}

void Configuration::clear() {
	configFileName = "";
	gatewaysFileName = "";
	databaseConfigFileName = "";
    pluginsParams.clear();
    pluginsPath = "";
	serverConfig.clear();
}

std::string Configuration::toString() {
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	rapidjson::Document doc;
	doc.SetObject();
	
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	rapidjson::Value cfn;
	cfn.SetString(configFileName.c_str(), (rapidjson::SizeType) configFileName.size(), allocator);
	doc.AddMember("configFileName", cfn, allocator);

	rapidjson::Value gfn;
	gfn.SetString(gatewaysFileName.c_str(), (rapidjson::SizeType) gatewaysFileName.size(), allocator);
	doc.AddMember("gatewaysFileName", gfn, allocator);

	rapidjson::Value server;
	serverConfig.toJson(server, allocator);
	doc.AddMember("server", server, allocator);

	rapidjson::Value ws;
	wsConfig.toJson(ws, allocator);
	doc.AddMember("ws", ws, allocator);

	rapidjson::Value dbcfn;
	dbcfn.SetString(databaseConfigFileName.c_str(), (rapidjson::SizeType) databaseConfigFileName.size(), allocator);
	doc.AddMember("databaseConfigFileName", dbcfn, allocator);

    rapidjson::Value jPluginParams;
    jPluginParams.SetArray();
    for (std::map<std::string, std::vector<std::string> >::const_iterator it(pluginsParams.begin()); it != pluginsParams.end(); it++) {
        rapidjson::Value jPluginParam;
        jPluginParam.SetArray();
        // add name
        rapidjson::Value jParamName;
        jParamName.SetString(it->first.c_str(), (rapidjson::SizeType) it->first.size(), allocator);
        jPluginParam.PushBack(jParamName, allocator);
        for (std::vector<std::string>::const_iterator itv(it->second.begin()); itv != it->second.end(); it++) {
            rapidjson::Value jParamValue;
            jParamValue.SetString(itv->c_str(), (rapidjson::SizeType) itv->size(), allocator);
            jPluginParam.PushBack(jParamValue, allocator);
        }
        jPluginParams.PushBack(jPluginParam, allocator);
    }
    doc.AddMember("pluginsParams", jPluginParams, allocator);

    rapidjson::Value psp;
    psp.SetString(pluginsPath.c_str(), (rapidjson::SizeType) pluginsPath.size(), allocator);
    doc.AddMember("pluginsPath", psp, allocator);

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
      threadCount(2), connectionLimit(1024), flags(0), jwtIssuer(""), jwtSecret(""),
      userPasswordListFileName("")
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
    jwtIssuer = "";
    jwtSecret = "";
    userPasswordListFileName = "";
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

    jwtIssuer = "";
    if (value.HasMember("issuer")) {
        rapidjson::Value &vissuer = value["issuer"];
        if (vissuer.IsString()) {
            jwtIssuer = vissuer.GetString();
        }
    }

    jwtSecret = "";
    if (value.HasMember("secret")) {
        rapidjson::Value &vsecret = value["secret"];
        if (vsecret.IsString()) {
            jwtSecret = vsecret.GetString();
        }
    }

    userPasswordListFileName = "";
    if (value.HasMember("userListFileName")) {
        rapidjson::Value &vjwtUserListFileName = value["userListFileName"];
        if (vjwtUserListFileName.IsString()) {
            userPasswordListFileName = vjwtUserListFileName.GetString();
        }
    }

    return LORA_OK;
}

std::string WebServiceConfig::toString()
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    toJson(doc, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return std::string(buffer.GetString());
}

void WebServiceConfig::toJson(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator
)
{
	value.SetObject();

	rapidjson::Value vEnabled;
	vEnabled.SetBool(enabled);
	value.AddMember("enabled", vEnabled, allocator);

	rapidjson::Value vPort;
	vPort.SetInt(port);
	value.AddMember("port", vPort, allocator);

	rapidjson::Value vHtml;
	vHtml.SetString(html.c_str(), (rapidjson::SizeType) html.size(), allocator);
	value.AddMember("html", vHtml, allocator);

	rapidjson::Value vDefaultDatabase;
	vDefaultDatabase.SetString(defaultDatabase.c_str(), (rapidjson::SizeType) defaultDatabase.size(), allocator);
	value.AddMember("defaultDatabase", vDefaultDatabase, allocator);

	rapidjson::Value vdatabases;
	vdatabases.SetArray();
	for (std::vector<std::string>::const_iterator it(databases.begin()); it != databases.end(); it++) {
		rapidjson::Value name;
		name.SetString(it->c_str(), (rapidjson::SizeType) it->size(), allocator);
		vdatabases.PushBack(name, allocator);
	}
	value.AddMember("databases", vdatabases, allocator);

	rapidjson::Value vThreadCount;
	vThreadCount.SetInt(threadCount);
	value.AddMember("threadCount", vThreadCount, allocator);

	rapidjson::Value vConnectionLimit;
	vConnectionLimit.SetInt(connectionLimit);
    value.AddMember("connectionLimit", vConnectionLimit, allocator);

	rapidjson::Value vFlags;
	vFlags.SetInt(flags);
    value.AddMember("flags", vFlags, allocator);

    rapidjson::Value vIssuer;
    vIssuer.SetString(jwtIssuer.c_str(), (rapidjson::SizeType) jwtIssuer.size(), allocator);
    value.AddMember("issuer", vIssuer, allocator);

    rapidjson::Value vSecret;
    vSecret.SetString(jwtSecret.c_str(), (rapidjson::SizeType) jwtSecret.size(), allocator);
    value.AddMember("secret", vSecret, allocator);

    rapidjson::Value vUserListFileName;
    vUserListFileName.SetString(userPasswordListFileName.c_str(), (rapidjson::SizeType) userPasswordListFileName.size(), allocator);
    value.AddMember("userListFileName", vUserListFileName, allocator);
}
