/**
 * @brief LoRaWAN 1.0.x packet decoder utility
 * @file lora-print.cpp
 * Copyright (c) 2021 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <iostream>
#include <cstring>
#include <vector>
#include <map>

#include "argtable3/argtable3.h"
#include "base64/base64.h"

#include "utilstring.h"

#include "errlist.h"
#include "utildate.h"

#include "db-any.h"

#include "pkt2/str-pkt2.h"

#include "identity-service-file-json.h"
#include "identity-service-dir-txt.h"
#include "utilidentity.h"
#include "lora-encrypt.h"
#include "database-config-json.h"
#include "utilfile.h"
#include "config-filename.h"
#include "log-intf.h"

#ifdef ENABLE_LMDB
#include "identity-service-lmdb.h"
#endif			

#define DEF_IDENTITY_STORAGE_NAME	"identity.json"
#define DEF_IDENTITY_STORAGE_TYPE	"json"

const std::string programName = "lora-print";
#define DEF_CONFIG_FILE_NAME ".lora-print.json"

static DatabaseByConfig *dbByConfig = nullptr;
static PayloadInsertPlugins plugins;

class LoraPrintConfiguration {
public:
	std::string command;				    // json|sql
	std::string protoPath;				    // proto file directory. Default 'proto'
    std::string pluginsPath;			    // plugin file directory. Default 'plugins'
	std::string dbConfig;				    // Default dbs.json'
	std::vector<std::string> dbName;	    // database names
    std::vector <std::string> extraDbName;  // passport
	std::string payload;				    // hex-string
	std::string messageType;
	// identity service
	std::string identityStorageName;
	IDENTITY_STORAGE identityStorageType;
    DEVEUI devEUI;                          // used for getting key to decipher Join Accept frame
    uint32_t devAddr;                       // default 42
	int outputFormat;					    // 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv header, 12- tab header
	int verbosity;						    // verbosity level
};

class PrintError: public LogIntf {
public:
    int verbosity;
    void logMessage(void *env, int level, int moduleCode, int errorCode, const std::string &message) override {
        if (level > verbosity)
            return;
        std::cerr << logLevelString(level) << " "
            << (errorCode ? std::to_string(errorCode) + ": " : " ")
            << message
            << std::endl;
    }
};

PrintError printError;

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
	LoraPrintConfiguration *config,
	int argc,
	char *argv[])
{
	struct arg_str *a_command, *a_proto_path, *a_plugins_path, *a_dbconfig, *a_dbname, *a_extra_dbname,
        *a_message_type, *a_payload_hex,
        *a_payload_base64, *a_dev_eui, *a_dev_addr, *a_identityStorageName, *a_identityStorageType;
	struct arg_int *a_output_format;
	struct arg_lit *a_payload_json, *a_verbosity, *a_help;
	struct arg_end *a_end;

	void *argtable[] = {
		a_command = arg_str0(nullptr, nullptr, "<command>", "json|sql. Default json"),

		a_payload_hex = arg_str0("x", "hex", "<hex-string>", "LoraWAN packet to decode, hexadecimal string."),
		a_payload_base64 = arg_str0("6", "base64", "<base64>", "same, base64 encoded."),
        a_payload_json = arg_lit0("j", "json", "Read JSON from stdin"),

        a_dev_eui = arg_str0("e", "eui", "<hex>", "Device EUI, used to decipher Join Accept frame"),
        a_dev_addr = arg_str0("a", "addr", "<hex>", "Device address. Default 2a (decimal 42"),

		a_proto_path = arg_str0("p", "proto", "<path>", "proto files directory. Default 'proto'"),
        a_plugins_path = arg_str0("l", "plugins", "<path>", "plugin directory. Default 'plugins'"),
		a_message_type = arg_str0("m", "message", "<pkt.msg>", "force nessage type packet and name"),

		a_identityStorageName = arg_str0("i", "id-name", "<name>", "default " DEF_IDENTITY_STORAGE_NAME),
		a_identityStorageType = arg_str0("y", "id-type", "json|txt|lmdb", "default " DEF_IDENTITY_STORAGE_TYPE),

		a_output_format = arg_int0("f", "format", "<0..8, 11..12>", "0- json(default), 1- csv, 2- tab, 3- sql, 4- sql, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv header, 12- tab header"),

		a_dbconfig = arg_str0("c", "dbConfig", "<file>", "database config file name. Default 'dbs.json'"),
		a_dbname = arg_strn("d", "db", "<db-name>", 0, 100, "database name, Default all"),
        a_extra_dbname = arg_strn("D", "extradb", "<path>", 0, 100, "Extra plugins options, Default none"),

		a_verbosity = arg_litn("v", "verbose", 0, 7, "Set verbosity level (-vvvvvvv: debug)"),
		a_help = arg_lit0("?", "help", "Show this help"),
		a_end = arg_end(20)
	};

	int nerrors;

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		// arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_PARAM_INVALID;
	}
	// Parse the command line as defined by argtable[]
	nerrors = arg_parse(argc, argv, argtable);

	config->command = "";
	if (!nerrors) {
		if (a_command->count)
			config->command = *a_command->sval;
		if (a_proto_path->count)
			config->protoPath = *a_proto_path->sval;
		else
			config->protoPath = "proto";
        if (a_plugins_path->count)
            config->pluginsPath = *a_plugins_path->sval;
        else
            config->pluginsPath = "plugins";
		if (a_dbconfig->count)
			config->dbConfig = *a_dbconfig->sval;
		else
			config->dbConfig = "dbs.json";

		if (a_message_type->count)
			config->messageType = *a_message_type->sval;
		else
			config->messageType = "";

        if (a_dev_eui->count) {
            string2DEVEUI(config->devEUI, *a_dev_eui->sval);
        } else {
            memset(&config->devEUI, '\0', sizeof(DEVEUI));
        }
        if (a_dev_addr->count) {
            DEVADDR a;
            string2DEVADDR(a, *a_dev_addr->sval);
            DEVADDRINT ai(a);
            config->devAddr = ai.a;
        } else {
            config->devAddr = 42;
        }
		for (int i = 0; i < a_dbname->count; i++) {
			config->dbName.push_back(a_dbname->sval[i]);
		}
        for (int i = 0; i < a_extra_dbname->count; i++) {
            config->extraDbName.push_back(a_extra_dbname->sval[i]);
        }

		config->payload = "";
		if (a_payload_hex->count)
			config->payload = hex2string(*a_payload_hex->sval);
		if (a_payload_base64->count) {
			try {
				config->payload = base64_decode(*a_payload_base64->sval, false);
			}
			catch (const std::exception& e) {
                printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_INVALID_BASE64, ERR_INVALID_BASE64);
				nerrors++;
			}
		}
        if (a_payload_json->count) {
            SEMTECH_PREFIX_GW prefix;
            prefix.token = 0;
            prefix.tag = 0;
            prefix.version = 2;
            memset(prefix.mac, 1, sizeof(DEVEUI));
            std::string json;
            std::cin >> json;
            config->payload = std::string((const char *) &prefix, sizeof(SEMTECH_PREFIX_GW)) + json;
        }
		config->verbosity = a_verbosity->count;
	}

	config->identityStorageName = "";
	if (a_identityStorageName->count) {
		config->identityStorageName = *a_identityStorageName->sval;
	}
	if (config->identityStorageName.empty())
		config->identityStorageName = DEF_IDENTITY_STORAGE_NAME;
	
	std::string sidentityStorageType = "";
	if (a_identityStorageType->count) {
		sidentityStorageType = *a_identityStorageType->sval;
	}
	config->identityStorageType = string2storageType(sidentityStorageType);

	config->outputFormat = 0;
	if (a_output_format->count) {
		config->outputFormat = *a_output_format->ival;
	}
	if (config->outputFormat < 0 || config->outputFormat > 12 ) {
        printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_WRONG_PARAM, ERR_WRONG_PARAM);
        nerrors++;
	}

	if (config->command.empty())
		config->command = "json";

	if (config->payload.empty()) {
        printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_NO_PAYLOAD, ERR_NO_PAYLOAD);
		nerrors++;
	}

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, programName.c_str());
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_LORA_PRINT_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

void doInsert(
	LoraPrintConfiguration *config,
	DatabaseByConfig *databaseByConfig,
	const std::string &binData,
	const std::map<std::string, std::string> *properties
)
{
    databaseByConfig->prepare(config->devAddr, binData);
	for (std::vector<std::string>::const_iterator it(config->dbName.begin()); it != config->dbName.end(); it++) {
		DatabaseNConfig *db = databaseByConfig->find(*it);
		if (!db) {
            std::stringstream ss;
            ss << ERR_DB_DATABASE_NOT_FOUND << *it;
            printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_DB_DATABASE_NOT_FOUND, ss.str());
			exit(ERR_CODE_DB_DATABASE_NOT_FOUND);
		}
        std::vector<std::string> clauses;
        db->insertClauses(clauses, config->messageType, INPUT_FORMAT_BINARY, binData, properties);
        std::string s;
        for (std::vector<std::string>::const_iterator it(clauses.begin()); it != clauses.end(); it++) {
            s += *it;
            s += " ";
        }
        std::cout << s << std::endl;
	}
}

/**
 * Print LoraWan packet
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv header, 12- tab header
 */ 
void doPrint(
	LoraPrintConfiguration *config,
    DatabaseByConfig *databaseByConfig,
    const uint32_t &addr,
	const std::string &binData,
    const std::map<std::string, std::string> *properties
)
{
    plugins.prepare(addr, binData);
    std::string nullValueString = "8888";

    databaseByConfig->prepare(config->devAddr, binData);
    for (std::vector<std::string>::const_iterator it(config->dbName.begin()); it != config->dbName.end(); it++) {
        DatabaseNConfig *db = databaseByConfig->find(*it);
        if (!db) {
            std::stringstream ss;
            ss << ERR_DB_DATABASE_NOT_FOUND << *it;
            printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_DB_DATABASE_NOT_FOUND, ss.str());
            exit(ERR_CODE_DB_DATABASE_NOT_FOUND);
        }

        int dialect = sqlDialectByName(db->config->type);

        std::stringstream ss;
        ss
            << "output format " << config->outputFormat
            << " message type \"" << config->messageType
            << "\" database \"" << *it
            << "\" type \"" << db->config->type
            << "\" dialect \"" << dialect
            << "\" payload \"" << hexString(binData);
        printError.logMessage(nullptr, LOG_DEBUG, LOG_ORA_PRINT, 0,ss.str());

        std::vector<std::string> clauses;
        plugins.insert(clauses, config->messageType, INPUT_FORMAT_BINARY, config->outputFormat,
            dialect, binData,
            &db->config->tableAliases, &db->config->fieldAliases, properties, nullValueString);

        for (std::vector<std::string>::const_iterator it(clauses.begin()); it != clauses.end(); it++) {
            std::cout << *it << std::endl;
        }
    }
    plugins.afterInsert();
}

static ConfigDatabasesIntf *configDatabases = nullptr;
static IdentityService *identityService = nullptr;

void done() {

    if (identityService) {
        delete identityService;
        identityService = nullptr;
    }
    if (configDatabases) {
        delete configDatabases;
        configDatabases = nullptr;
    }
    plugins.done();
    plugins.unload();
}

int main(
	int argc,
	char *argv[])
{
	LoraPrintConfiguration config;
	// load config file and get macGwConfig from the command line
	if (parseCmd(&config, argc, argv) != 0) {
		exit(ERR_CODE_COMMAND_LINE);
	}
    printError.verbosity = config.verbosity;

    if (!config.protoPath.empty()) {
        if (!util::fileExists(config.protoPath)) {
            config.protoPath = getDefaultConfigFileName(argv[0], config.protoPath);;
        }
    }
    if (!config.pluginsPath.empty()) {
        if (!util::fileExists(config.pluginsPath)) {
            config.pluginsPath = getDefaultConfigFileName(argv[0], config.pluginsPath);;
        }
    }
    if (config.pluginsPath.empty()) {
        printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_INIT_PLUGINS_FAILED, MSG_NO_PLUGINS_LOADED);
        exit(ERR_CODE_INIT_PLUGINS_FAILED);
    }

    configDatabases = new ConfigDatabasesJson(config.dbConfig);
	if (configDatabases->dbs.empty()) {
        printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_LOAD_DATABASE_CONFIG, ERR_LOAD_DATABASE_CONFIG);
        done();
        exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	if (config.dbName.size() == 0) {
		// if not database is specified, use all of them
		for (std::vector<ConfigDatabase>::const_iterator it(configDatabases->dbs.begin()); it != configDatabases->dbs.end(); it++) {
			config.dbName.push_back(it->name);
		}
	}

	DatabaseByConfig databaseByConfig(configDatabases, nullptr);
	// Start identity service
	switch (config.identityStorageType) {
		case IDENTITY_STORAGE_LMDB:
#ifdef ENABLE_LMDB
			identityService = new LmdbIdentityService();
#endif			
			break;
		case IDENTITY_STORAGE_DIR_TEXT:
			identityService = new DirTxtIdentityService();
			break;
		default:
			identityService = new JsonFileIdentityService();
	}
	identityService->init(config.identityStorageName, nullptr);

    // Plugins load & init
    int r = plugins.load(config.pluginsPath);
    if (r <= 0) {
        std::stringstream ss;
        ss << ERR_LOAD_PLUGINS_FAILED << "plugins directory: \"" << config.pluginsPath << "\"";
        printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_LOAD_PLUGINS_FAILED, ss.str());
        exit(ERR_CODE_LOAD_PLUGINS_FAILED);
    }
    std::string dbName;
    if (config.dbName.size())
        dbName = config.dbName[0];
    dbByConfig = new DatabaseByConfig(configDatabases, &plugins);
    r = plugins.init(config.protoPath, dbName, config.extraDbName, &printError, 0);
    if (r) {
        std::stringstream ss;
        ss << ERR_INIT_PLUGINS_FAILED
            << "plugins directory: \"" << config.pluginsPath << "\""
            << ", proto directory: \"" << config.protoPath << "\""
            << ", database name: \"" << dbName << "\"";
        for (std::vector<std::string>::const_iterator it(config.extraDbName.begin()); it != config.extraDbName.end(); it++) {
            ss << " extra db: " << *it;
        }
        printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_INIT_PLUGINS_FAILED, ss.str());
        exit(ERR_CODE_INIT_PLUGINS_FAILED);
    }

    std::stringstream ss;
    ss
       << "plugins directory: \"" << config.pluginsPath << "\""
       << ", proto directory: \"" << config.protoPath << "\""
       << ", database name: \"" << dbName << "\"";
    for (std::vector<std::string>::const_iterator it(config.extraDbName.begin()); it != config.extraDbName.end(); it++) {
        ss << " extra db: " << *it;
    }
    printError.logMessage(nullptr, LOG_DEBUG, LOG_ORA_PRINT, ERR_CODE_INIT_PLUGINS_FAILED, ss.str());

    // parse packet
    SEMTECH_PREFIX_GW dataPrefix;
	GatewayStat gatewayStat;
	std::vector<SemtechUDPPacket> packets;

	r = SemtechUDPPacket::parse(nullptr, dataPrefix,
        gatewayStat, packets, config.payload.c_str(), config.payload.size(), identityService);

    switch (r) {
        case ERR_CODE_IS_JOIN:
            if (packets.size()) {
                switch (packets[0].header.header.macheader.f.mtype) {
                    case MTYPE_JOIN_REQUEST:
                        {
                            JOIN_REQUEST_FRAME *joinRequestFrame = packets[0].getJoinRequestFrame();
                            if (joinRequestFrame) {
                                std::cout << "Join request "
                                          << JOIN_REQUEST_FRAME2string(*joinRequestFrame)
                                          << std::endl;
                            }
                        }
                        break;
                    case MTYPE_JOIN_ACCEPT:
                        {
                            JOIN_ACCEPT_FRAME *joinAcceptFrame = packets[0].getJoinAcceptFrame();
                            if (isDEVEUIEmpty(config.devEUI)) {
                                printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_INIT_PLUGINS_FAILED, "Device EUI missed. Provide -e <EUI> option.");
                                done();
                                exit(ERR_CODE_PARAM_INVALID);
                            }

                            NetworkIdentity ni;
                            int ir = identityService->getNetworkIdentity(ni, config.devEUI);
                            if (ir) {
                                std::stringstream ss;
                                ss << "Device EUI " << DEVEUI2string(config.devEUI) << " not found.";
                                printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_INIT_PLUGINS_FAILED, ss.str());
                                done();
                                exit(ERR_CODE_PARAM_INVALID);
                            }

                            const KEY128 *key = &ni.nwkKey;
                            if (joinAcceptFrame) {
                                encryptJoinAcceptResponse(*joinAcceptFrame, *key);
                                std::cout << "Join accept "
                                          << JOIN_ACCEPT_FRAME2string(*joinAcceptFrame)
                                          << std::endl;
                                break;
                            }

                            JOIN_ACCEPT_FRAME_CFLIST *joinAcceptCFListFrame = packets[0].getJoinAcceptCFListFrame();
                            if (joinAcceptCFListFrame) {
                                encryptJoinAcceptCFListResponse(*joinAcceptCFListFrame, *key);
                                std::cout << "Join accept CFList "
                                          << JOIN_ACCEPT_FRAME_CFLIST2string(*joinAcceptCFListFrame)
                                          << std::endl;
                                break;
                            }
                            std::cerr << "Invalid Join Accept frame "
                                << hexString(packets[0].payload) << " (" << packets[0].payload.size() << " bytes)"
                                << ". Expected size is " << sizeof(JOIN_ACCEPT_FRAME_CFLIST) << " or " << sizeof(JOIN_ACCEPT_FRAME)
                                << std::endl;
                        }
                        break;
                }
            }
            break;
        default:
            if (r)
                printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, r, strerror_lorawan_ns(r));
	}

	if (gatewayStat.errcode == 0) {
		std::cerr << gatewayStat.toJsonString() << std::endl;
	}

    if (r) {
        done();
        exit(r);
    }

    if (config.verbosity > 2) {
        std::cerr << packets.size() << " packets" << std::endl;
    }

	for (std::vector<SemtechUDPPacket>::iterator it(packets.begin()); it != packets.end(); it++) {
		if (it->errcode) {
            printError.logMessage(nullptr, LOG_ERR, LOG_ORA_PRINT, ERR_CODE_INVALID_PACKET, ERR_INVALID_PACKET);
            continue;
		}
		if (config.verbosity > 2) {
    		std::cerr << it->toJsonString() << std::endl;
			if (it->hasApplicationPayload())
				std::cerr << "application payload " << (int) it->getHeader()->fport << std::endl;
			if (it->hasMACPayload())
				std::cerr << "MAC payload " << std::endl;
		}

		std::string payload = it->payload;
        // set properties addr eui name activation (ABP|OTAA) class (A|B|C) name
        // time  timestamp
        time_t t(time(nullptr));
        std::map<std::string, std::string> properties;
        properties["addr"] = it->getDeviceAddrStr();							    // addr network address string
        properties["fport"] = std::to_string((int) it->header.fport);			// application port number (1..223). 0- MAC, 224- test, 225..255- reserved
        properties["time"] = std::to_string(t);									// time 32-bit integer (seconds since Unix epoch)
        properties["timestamp"] = time2string(t);								// timestamp string
        properties["eui"] = it->getDeviceEUI();									    // eui global end-device identifier in IEEE EUI64 address space
        properties["name"] = it->devId.name;									    // device name
        properties["activation"] =  activation2string(it->devId.activation);	// (ABP|OTAA)
        properties["class"] = deviceclass2string(it->devId.deviceclass);		// A|B|C

        if (config.command == "sql") {
			doInsert(&config, &databaseByConfig, payload, &properties);
		} else {
            DEVADDRINT a = it->getDeviceAddr();
			doPrint(&config, &databaseByConfig, a.a, payload, &properties);
		}
	}
    done();
	return 0;
}
