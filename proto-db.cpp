/**
 * @brief Database helper utility
 * @file proto-db.cpp
 * Copyright (c) 2021 andrey.ivanov@ikfia.ysn.ru Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <iostream>
#include <signal.h>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <map>

#include "database-config-json.h"

#include "argtable3/argtable3.h"
#include "base64/base64.h"

#include "utilstring.h"

#include "errlist.h"
#include "utildate.h"
#include "utilidentity.h"

#include "db-any.h"

#include "pkt2/str-pkt2.h"

#include "identity-service-file-json.h"
#include "identity-service-dir-txt.h"

#include "config-filename.h"

const std::string programName = "proto-db";
#define DEF_CONFIG_FILE_NAME ".proto-db.json"
#define DEF_IDENTITY_STORAGE_NAME	"identity.json"
#define DEF_IDENTITY_STORAGE_TYPE	"json"

class Configuration {
public:
	std::string command;				// print|list|create|insert
	std::string protoPath;				// proto file directory. Default 'proto
    std::string pluginsPath;            // plugin directory. Default 'plugins'
    std::vector<std::string> extraDbName;

    std::string dbConfigFileName;		// Default dbs.json'
	std::vector<std::string> dbName;	// database names

	std::string payload;				// hex or base64 string

	int offset;							// list command, offset. Default 0.
	int limit;							// list command, limit size. Default 10
	std::vector<std::string> sortAsc;	// list command, sort by field ascending
	std::vector<std::string> sortDesc;	// list command, sort by field descending

	std::string messageType;

	// identity service
	std::string identityStorageName;
	int identityStorageType;
	std::string addr;					// for insert
	uint8_t fport;

	int verbosity;						// verbosity level
};

class PrintError: public LogIntf {
public:
    int verbosity;
    PrintError() : verbosity(0) {};
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

static PayloadInsertPlugins plugins;

static void done()
{
	// destroy and free all
    plugins.done();
    plugins.unload();

    exit(0);
}

static void stop()
{
}

void signalHandler(int signal)
{
	switch (signal)
	{
	case SIGINT:
		std::cerr << MSG_INTERRUPTED << std::endl;
		stop();
		done();
		break;
	default:
		break;
	}
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = &signalHandler;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
}
#endif

/**
 * @brief Parse command line
 * @return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
	Configuration *config,
	int argc,
	char *argv[])
{
	struct arg_str *a_command, *a_proto_path, *a_plugins_path, *a_extra_dbname, *a_dbconfigfilename, *a_dbname, *a_message_type,
		*a_payload_hex, *a_payload_base64, *a_sort_asc, *a_sort_desc, *a_addr, *a_identityStorageName,
        *a_identityStorageType;
	struct arg_int *a_offset, *a_limit, *a_fport;
	struct arg_lit *a_verbosity, *a_help;
	struct arg_end *a_end;

	void *argtable[] = {
		a_command = arg_str0(nullptr, nullptr, "<command>", "print|list|create|insert. Default print"),
		a_proto_path = arg_str0("p", "proto", "<path>", "proto files directory. Default 'proto'"),
        a_plugins_path = arg_str0("l", "plugins", "<path>", "plugin directory. Default 'plugins'"),
        a_extra_dbname = arg_strn("D", "extradb", "<path>", 0, 100, "Extra plugins options, Default none"),
        a_dbconfigfilename = arg_str0("c", "dbConfig", "<file>", "database config file name. Default 'dbs.json'"),
		a_dbname = arg_strn("d", "dbName", "<database>", 0, 100, "database name, Default all"),

		a_message_type = arg_str0("m", "message", "<pkt.msg>", "Message type packet and name"),
		
		a_payload_hex = arg_str0("x", "hex", "<hex-string>", "print, insert command, payload data."),
		a_payload_base64 = arg_str0("6", "base64", "<base64>", "print, insert command, payload data."),

		a_offset = arg_int0("o", "offset", "<number>", "list command, offset. Default 0."),
		a_limit = arg_int0("l", "limit", "<number>", "list command, limit size. Default 10."),
		a_sort_asc = arg_strn("s", "asc", "<field-name>", 0, 100, "list command, sort by field ascending."),
		a_sort_desc = arg_strn("S", "desc", "<field-name>", 0, 100, "list command, sort by field descending."),
		
		a_addr = arg_str0("a", "addr", "<hex>", "insert, device network address"),
		a_fport = arg_int0("f", "fport", "<0.233>", "port field. 0- MAC. Default 1.."),
		a_identityStorageName = arg_str0("i", "id-name", "<name>", "default " DEF_IDENTITY_STORAGE_NAME),
		a_identityStorageType = arg_str0("y", "id-type", "json|txt|lmdb", "default " DEF_IDENTITY_STORAGE_TYPE),

		a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level"),
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
        for (int i = 0; i < a_extra_dbname->count; i++) {
            config->extraDbName.emplace_back(a_extra_dbname->sval[i]);
        }

		if (a_dbconfigfilename->count)
			config->dbConfigFileName = *a_dbconfigfilename->sval;
		else
			config->dbConfigFileName = "dbs.json";

		if (a_message_type->count)
			config->messageType = *a_message_type->sval;
		else
			config->messageType = "";

		for (int i = 0; i < a_dbname->count; i++) {
			config->dbName.push_back(a_dbname->sval[i]);
		}

		config->payload = "";
		if (a_payload_hex->count)
			config->payload = hex2string(*a_payload_hex->sval);
		if (a_payload_base64->count)
			try {
				config->payload = base64_decode(*a_payload_base64->sval, false);
			}
			catch (const std::exception& e) {
				std::cerr << ERR_INVALID_BASE64 << std::endl;
				nerrors++;
			}
		if (a_offset->count)
			config->offset = *a_offset->ival;
		else
			config->offset = 0;

		if (a_limit->count)
			config->limit = *a_limit->ival;
		else
			config->limit = 10;

		for (int i = 0; i < a_sort_asc->count; i++) {
			config->sortAsc.push_back(a_sort_asc->sval[i]);
		}
		for (int i = 0; i < a_sort_desc->count; i++) {
			config->sortDesc.push_back(a_sort_desc->sval[i]);
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

		config->addr = "";
		if (a_addr->count)
			config->addr = *a_addr->sval;

		if (a_fport->count)
			config->fport = *a_fport->ival;
		else
			config->fport = 1;

		config->verbosity = a_verbosity->count;
	}

	if (config->command == "insert") {
		if (config->addr.empty()) {
			std::cerr << ERR_MESSAGE << ERR_CODE_INVALID_ADDRESS << ": " << ERR_INVALID_ADDRESS << std::endl;
			nerrors++;
		}
	}

	if (config->fport > 233) {
		std::cerr << ERR_MESSAGE << ERR_CODE_INVALID_FPORT << ": " << ERR_INVALID_FPORT << std::endl;
		nerrors++;
	}

	if (config->command.empty())
		config->command = "print";

	if (config->payload.empty()) {
		if ((config->command == "print") || (config->command == "insert")) {
			std::cerr << ERR_MESSAGE << ERR_CODE_NO_PAYLOAD << ": " << ERR_NO_PAYLOAD << std::endl;
			nerrors++;
		}
	}

	if (config->messageType.empty()) {
		if (config->command == "create") {
			// try to get messageType from the payload
			if (config->payload.empty()) {
				nerrors++;
				std::cerr << ERR_MESSAGE << ERR_CODE_NO_MESSAGE_TYPE << ": " << ERR_NO_MESSAGE_TYPE << std::endl;
				std::cerr << ERR_MESSAGE << ERR_CODE_NO_PAYLOAD << ": " << ERR_NO_PAYLOAD << std::endl;
			}
		}
	}

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, programName.c_str());
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_PROTO_DB_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

void doList(
	Configuration *config,
	DatabaseByConfig *dbAny,
	const std::string &messageType
)
{
	for (std::vector<std::string>::const_iterator it(config->dbName.begin()); it != config->dbName.end(); it++) {
		DatabaseNConfig *db = dbAny->find(*it);
		if (!db) {
			std::cerr << ERR_DB_DATABASE_NOT_FOUND << *it << std::endl;
			exit(ERR_CODE_DB_DATABASE_NOT_FOUND);
		}

		int r = db->open();
		if (r) {
			std::cerr << ERR_DB_DATABASE_OPEN << r << std::endl;
			exit(ERR_CODE_DB_DATABASE_OPEN);
		}

		std::string t(db->tableName(messageType));
		std::stringstream ss;

		std::string quote;
		if (db->db->type == "mysql")
			quote = "`";
		else
			quote = "\"";

		if (db->db->type == "firebird") {
			ss << "SELECT";
			if (config->limit) {
				ss << " FIRST " << config->limit;
			}
			if (config->offset) {
				ss << " SKIP " << config->offset;
			}
			ss << " * ";
		} else {
			ss << "SELECT * ";
		}
		ss << "FROM " << quote << t << quote;

		if (config->sortAsc.size() || config->sortDesc.size()) {
			ss << " ORDER BY ";
		}
		bool isFirst = true;
		for (std::vector<std::string>::const_iterator it(config->sortAsc.begin()); it != config->sortAsc.end(); it++)  {
			if (isFirst)
				isFirst = false;
			else
				ss << ", ";
			ss << quote << *it << quote;
		}

		for (std::vector<std::string>::const_iterator it(config->sortDesc.begin()); it != config->sortDesc.end(); it++)  {
			if (isFirst)
				isFirst = false;
			else
				ss << ", ";
			ss << quote << *it << quote << " DESC";
		}

		if (db->db->type != "firebird") {
			if (config->limit) {
				ss << " LIMIT " << config->limit;
			}
			if (config->offset) {
				ss << " OFFSET " << config->offset;
			}
		}

		std::string selectClause = ss.str();
		if (config->verbosity >= 3)
		{
			std::cerr << "Clause: " << std::endl << selectClause << std::endl;
		}
		std::vector<std::vector<std::string>> vals;

		r = db->select(vals, selectClause);
		if (r) {
			std::cerr << ERR_DB_SELECT << r << " database " << *it << ": " << db->db->errmsg << std::endl;
			std::cerr << "SQL statement: " << selectClause << std::endl;
		}

		for (std::vector<std::vector<std::string>>::const_iterator it(vals.begin()); it != vals.end(); it++)
		{
			for (std::vector<std::string>::const_iterator it2(it->begin()); it2 != it->end(); it2++) {
				std::cout << *it2 << "|";
			}
			std::cout << std::endl;
		}
		r = db->close();
        if (r) {
            std::cerr << ERR_DB_DATABASE_CLOSE << r << " database " << *it << ": " << db->db->errmsg << std::endl;
        }
	}
}

void doCreate
(
	Configuration *config,
	DatabaseByConfig *dbAny,
	const std::string &messageType,
	int verbosity
)
{
	for (std::vector<std::string>::const_iterator it(config->dbName.begin()); it != config->dbName.end(); it++) {
		DatabaseNConfig *db = dbAny->find(*it);
		if (!db) {
			std::cerr << ERR_DB_DATABASE_NOT_FOUND << *it << std::endl;
			exit(ERR_CODE_DB_DATABASE_NOT_FOUND);
		}

		int r = db->open();
		if (r) {
			std::cerr << ERR_DB_DATABASE_OPEN << r << std::endl;
			exit(ERR_CODE_DB_DATABASE_OPEN);
		}

		if (verbosity) {
			std::cout << db->createClause(messageType) << std::endl;
		}

		r = db->createTable(messageType);
		if (r) {
			std::cerr << ERR_DB_CREATE << r << " database " << *it << ": " << db->db->errmsg << std::endl;
			std::cerr << "SQL statement: " << db->createClause(messageType) << std::endl;
		}
		r = db->close();
        if (r) {
            std::cerr << ERR_DB_DATABASE_CLOSE << r << " database " << *it << ": " << db->db->errmsg << std::endl;
        }
	}
}

void doInsert(
	Configuration *config,
	DatabaseByConfig *dbAny,
	const std::string &messageType,
	const std::string &payload,
	const std::map<std::string, std::string> &props,
	int verbosity
)
{
    DEVADDR a;
    string2DEVADDR(a, config->addr);
    DEVADDRINT ai(a);
    dbAny->prepare(ai.a, payload);
	for (std::vector<std::string>::const_iterator it(config->dbName.begin()); it != config->dbName.end(); it++) {
		DatabaseNConfig *db = dbAny->find(*it);
		if (!db) {
			std::cerr << ERR_DB_DATABASE_NOT_FOUND << *it << std::endl;
			exit(ERR_CODE_DB_DATABASE_NOT_FOUND);
		}

		int r = db->open();
		if (r) {
			std::cerr << ERR_DB_DATABASE_OPEN << r << std::endl;
			exit(ERR_CODE_DB_DATABASE_OPEN);
		}
		std::map<std::string, std::string> properties;
		db->setProperties(properties, props);

		if (verbosity) {
            std::vector<std::string> clauses;
            db->insertClauses(clauses, messageType, INPUT_FORMAT_BINARY, payload, &properties);
            std::string s;
            for (std::vector<std::string>::const_iterator it(clauses.begin()); it != clauses.end(); it++) {
                s += *it;
                s += " ";
            }
            std::cout << s << std::endl;
		}
		r = db->insert(messageType, INPUT_FORMAT_BINARY, payload, &properties);   // 1- INPUT_FORMAT_HEX

		if (r) {
			std::cerr << ERR_DB_INSERT << r << " database " << *it << ": " << db->db->errmsg << std::endl;
            std::vector<std::string> clauses;
            db->insertClauses(clauses, messageType, INPUT_FORMAT_BINARY, payload, &properties);
            std::string s;
            for (std::vector<std::string>::const_iterator it(clauses.begin()); it != clauses.end(); it++) {
                s += *it;
                s += " ";
            }
			std::cerr << "SQL statement: " << s << std::endl;
		}
		r = db->close();
        if (r) {
            std::cerr << ERR_DB_DATABASE_CLOSE << r << " database " << *it << ": " << db->db->errmsg << std::endl;
        }
	}
}

void doPrint(
	Configuration *config,
	DatabaseByConfig *databaseByConfig,
	const std::string &forceMessageType,
	const std::string &payload,
    const std::map<std::string, std::string> &props,
    int outputFormat
)
{
    const uint32_t addr = 0;
    plugins.prepare(addr, payload);
    std::string nullValueString = "8888";

    databaseByConfig->prepare(addr, payload);
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
        ss << "output format " << outputFormat << " " << getOutputFormatName(outputFormat)
           << " message type \"" << config->messageType
           << "\" database \"" << *it
           << "\" type \"" << db->config->type
           << "\" dialect \"" << dialect
           << "\" payload \"" << hexString(payload);
        printError.logMessage(nullptr, LOG_DEBUG, LOG_ORA_PRINT, 0,ss.str());

        std::vector<std::string> clauses;
        std::map<std::string, std::string> validProperties;
        db->setProperties(validProperties, props);
        plugins.insert(clauses, config->messageType, INPUT_FORMAT_BINARY, outputFormat,
    dialect, payload,
        &db->config->tableAliases, &db->config->fieldAliases, &validProperties, nullValueString);

        for (std::vector<std::string>::const_iterator it(clauses.begin()); it != clauses.end(); it++) {
            std::cout << *it << std::endl;
        }
    }

    plugins.afterInsert();
}

int main(
	int argc,
	char *argv[]
)
{
#ifdef _MSC_VER
#else
	setSignalHandler();
#endif
	Configuration config;
	// load config file and get macGwConfig from the command line
	if (parseCmd(&config, argc, argv) != 0) {
		exit(ERR_CODE_COMMAND_LINE);
	}
    ConfigDatabasesIntf *configDatabases = new ConfigDatabasesJson(config.dbConfigFileName);

	if (configDatabases->dbs.empty()) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
        delete configDatabases;
		exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	if (config.dbName.size() == 0) {
		// if not database is specified, use all of them
		for (std::vector<ConfigDatabase>::const_iterator it(configDatabases->dbs.begin()); it != configDatabases->dbs.end(); it++) {
			config.dbName.push_back(it->name);
		}
	}

    // Plugins load & init
    int r = plugins.load(config.pluginsPath);
    if (r <= 0) {
        std::stringstream ss;
        ss << ERR_LOAD_PLUGINS_FAILED << "plugins directory: \"" << config.pluginsPath << "\"";
        std::cerr << ss.str() << std::endl;
        exit(ERR_CODE_LOAD_PLUGINS_FAILED);
    }
    std::string dbName;
    if (!config.dbName.empty())
        dbName = config.dbName[0];
    DatabaseByConfig dbAny(configDatabases, &plugins);

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

    // set properties addr eui name activation (ABP|OTAA) class (A|B|C) name
    std::map<std::string, std::string> properties;
    time_t t(time(nullptr));
    properties["addr"] = config.addr;						    // addr network address string
    properties["fport"] = std::to_string(config.fport);		// application port number (1..223). 0- MAC, 224- test, 225..255- reserved
    properties["time"] = std::to_string(t);					// time (seconds since Unix epoch)
    properties["timestamp"] = time2string(t);				// timestamp string
    // TODO get it from command line
    properties["deveui"] = "3232323232323232";				    // eui global end-device identifier in IEEE EUI64 address space
    properties["name"] = "device32";						    // device name
    properties["activation"] = "ABP";						    // (ABP|OTAA)
    properties["class"] = "C";								    // A|B|C

    if (config.command == "print")
		doPrint(&config, &dbAny, config.messageType, config.payload, properties, OUTPUT_FORMAT_JSON);
	if (config.command == "list")
		doList(&config, &dbAny, config.messageType);
	if (config.command == "create") {
		doCreate(&config, &dbAny, config.messageType, config.verbosity);
	}
	if (config.command == "insert") {
		IdentityService *identityService = nullptr;
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
		DEVADDR devaddr;
		string2DEVADDR(devaddr, config.addr);
		DeviceId deviceId;
		r = identityService->get(deviceId, devaddr);
		if (r) {
			std::cerr << ERR_MESSAGE << ERR_CODE_INVALID_ADDRESS << ": " << ERR_INVALID_ADDRESS << std::endl;
            delete configDatabases;
			exit(ERR_CODE_INVALID_ADDRESS);
		}
		deviceId.setProperties(properties);

		doInsert(&config, &dbAny, config.messageType, config.payload, properties, config.verbosity);

		if (identityService)
			delete identityService;
	}
    if (configDatabases)
        delete configDatabases;
	return 0;
}
