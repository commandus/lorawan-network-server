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

#include <google/protobuf/message.h>

#include "argtable3/argtable3.h"
#include "base64/base64.h"

#include "platform.h"
#include "utilstring.h"

#include "errlist.h"
#include "utilstring.h"
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
	std::string proto_path;				// proto file directory. Default 'proto
	std::string dbConfigFileName;		// Default dbs.js'
	std::vector<std::string> dbname;	// database names

	std::string payload;				// hex-string

	int offset;							// list command, offset. Default 0.
	int limit;							// list command, limit size. Default 10
	std::vector<std::string> sort_asc;	// list command, sort by field ascending
	std::vector<std::string> sort_desc;	// list command, sort by field descending

	std::string message_type;

	// identity service
	std::string identityStorageName;
	int identityStorageType;
	std::string addr;					// for insert
	uint8_t fport;

	int verbosity;						// verbosity level
};

static void done()
{
	// destroy and free all
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
	struct arg_str *a_command, *a_proto_path, *a_dbconfigfilename, *a_dbname, *a_message_type, 
		*a_payload_hex, *a_payload_base64,
		*a_sort_asc, *a_sort_desc, *a_addr, *a_identityStorageName, *a_identityStorageType;
	struct arg_int *a_offset, *a_limit, *a_fport;
	struct arg_lit *a_verbosity, *a_help;
	struct arg_end *a_end;

	void *argtable[] = {
		a_command = arg_str0(NULL, NULL, "<command>", "print|list|create|insert. Default print"),
		a_proto_path = arg_str0("p", "proto", "<path>", "proto files directory. Default 'proto'"),
		a_dbconfigfilename = arg_str0("c", "dbconfig", "<file>", "database config file name. Default 'dbs.js'"),
		a_dbname = arg_strn("d", "dbname", "<database>", 0, 100, "database name, Default all"),

		a_message_type = arg_str0("m", "message", "<pkt.msg>", "Message type packet and name"),
		
		a_payload_hex = arg_str0("x", "hex", "<hex-string>", "print, insert command, payload data."),
		a_payload_base64 = arg_str0("6", "base64", "<base64>", "print, insert command, payload data."),

		a_offset = arg_int0("o", "offset", "<number>", "list command, offset. Default 0."),
		a_limit = arg_int0("l", "limit", "<number>", "list command, limit size. Default 10."),
		a_sort_asc = arg_strn("s", "asc", "<field-name>", 0, 100, "list command, sort by field ascending."),
		a_sort_desc = arg_strn("S", "desc", "<field-name>", 0, 100, "list command, sort by field descending."),
		
		a_addr = arg_str0("a", "addr", "<addr>", "insert, device network address"),
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
			config->proto_path = *a_proto_path->sval;
		else
			config->proto_path = "proto";

		if (a_dbconfigfilename->count)
			config->dbConfigFileName = *a_dbconfigfilename->sval;
		else
			config->dbConfigFileName = "dbs.js";

		if (a_message_type->count)
			config->message_type = *a_message_type->sval;
		else
			config->message_type = "";

		for (int i = 0; i < a_dbname->count; i++) {
			config->dbname.push_back(a_dbname->sval[i]);
		}

		config->payload = "";
		if (a_payload_hex->count)
			config->payload = *a_payload_hex->sval;
		if (a_payload_base64->count)
			try {
				config->payload = hexString(base64_decode(*a_payload_base64->sval, false));
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
			config->sort_asc.push_back(a_sort_asc->sval[i]);
		}
		for (int i = 0; i < a_sort_desc->count; i++) {
			config->sort_desc.push_back(a_sort_desc->sval[i]);
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

	if (config->message_type.empty()) {
		if (config->command == "create") {
			// try to get message_type from the payload
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

void doList
(
	void* env,
	Configuration *config,
	DatabaseByConfig *dbAny,
	const std::string &messageType
)
{
	for (std::vector<std::string>::const_iterator it(config->dbname.begin()); it != config->dbname.end(); it++) {
		
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

		std::string t(db->tableName(env, messageType));
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

		if (config->sort_asc.size() || config->sort_desc.size()) {
			ss << " ORDER BY ";
		}
		bool isFirst = true;
		for (std::vector<std::string>::const_iterator it(config->sort_asc.begin()); it != config->sort_asc.end(); it++)  {
			if (isFirst)
				isFirst = false;
			else
				ss << ", ";
			ss << quote << *it << quote;
		}

		for (std::vector<std::string>::const_iterator it(config->sort_desc.begin()); it != config->sort_desc.end(); it++)  {
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
	}
}

void doCreate
(
	void* env,
	Configuration *config,
	DatabaseByConfig *dbAny,
	const std::string &messageType,
	int verbosity
)
{
	for (std::vector<std::string>::const_iterator it(config->dbname.begin()); it != config->dbname.end(); it++) {
		
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
			std::cout << db->createClause(env, messageType) << std::endl;
		}
		r = db->createTable(env, messageType);
		if (r) {
			std::cerr << ERR_DB_CREATE << r << " database " << *it << ": " << db->db->errmsg << std::endl;
			std::cerr << "SQL statement: " << db->createClause(env, messageType) << std::endl;
		}
		r = db->close();
	}
}

void doInsert
(
	void* env,
	Configuration *config,
	DatabaseByConfig *dbAny,
	const std::string &messageType,
	const std::string &hexData,
	const std::map<std::string, std::string> &props,
	int verbosity
)
{
	for (std::vector<std::string>::const_iterator it(config->dbname.begin()); it != config->dbname.end(); it++) {
		
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
		db->config->setProperties(properties, props);

		if (verbosity) {
            std::vector<std::string> clauses;
            db->insertClauses(clauses, env, messageType, 1, hexData, &properties);
            std::string s;
            for (std::vector<std::string>::const_iterator it(clauses.begin()); it != clauses.end(); it++) {
                s += *it;
                s += " ";
            }
            std::cout << s << std::endl;  // 1- INPUT_FORMAT_HEX
		}

		r = db->insert(env, messageType, 1, hexData, &properties);   // 1- INPUT_FORMAT_HEX

		if (r) {
			std::cerr << ERR_DB_INSERT << r << " database " << *it << ": " << db->db->errmsg << std::endl;
            std::vector<std::string> clauses;
            db->insertClauses(clauses, env, messageType, 1, hexData, &properties);
            std::string s;
            for (std::vector<std::string>::const_iterator it(clauses.begin()); it != clauses.end(); it++) {
                s += *it;
                s += " ";
            }
			std::cerr << "SQL statement: " << s << std::endl; // 1- INPUT_FORMAT_HEX
		}
		r = db->close();
	}
}

void doPrint
(
	void* env,
	Configuration *config,
	DatabaseByConfig *dbAny,
	const std::string &forceMessageType,
	const std::string &hexData
)
{
	std::cout << parsePacket(env, 1, 0, 0, hexData, forceMessageType, NULL, NULL, NULL) << std::endl;
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

	void* env = initPkt2(config.proto_path, 0);
	if (!env) {
		std::cerr << ERR_LOAD_PROTO << std::endl;
		exit(ERR_CODE_LOAD_PROTO);
	}

	ConfigDatabases configDatabases(config.dbConfigFileName);
	if (configDatabases.dbs.size() == 0) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
		donePkt2(env);
		exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	if (config.dbname.size() == 0) {
		// if not database is specified, use all of them
		for (std::vector<ConfigDatabase>::const_iterator it(configDatabases.dbs.begin()); it != configDatabases.dbs.end(); it++) {
			config.dbname.push_back(it->name);
		}
	}

	DatabaseByConfig dbAny(&configDatabases);

	if (config.command == "print")
		doPrint(env, &config, &dbAny, config.message_type, config.payload);
	if (config.command == "list")
		doList(env, &config, &dbAny, config.message_type);
	if (config.command == "create") {
		if (config.message_type.empty()) {
			google::protobuf::Message *m;
			parsePacket2ProtobufMessage((void**) &m, env, 1, config.payload, "", NULL, NULL, NULL);
			if (m)
				config.message_type = m->GetTypeName();
		}
		doCreate(env, &config, &dbAny, config.message_type, config.verbosity);
	}
	if (config.command == "insert") {
		IdentityService *identityService = NULL;
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
		identityService->init(config.identityStorageName, NULL);
		DEVADDR devaddr;
		string2DEVADDR(devaddr, config.addr);
		DeviceId deviceId;
		int r = identityService->get(deviceId, devaddr);
		if (r) {
			std::cerr << ERR_MESSAGE << ERR_CODE_INVALID_ADDRESS << ": " << ERR_INVALID_ADDRESS << std::endl;
			exit(ERR_CODE_INVALID_ADDRESS);
		}
		// set properties addr eui name activation (ABP|OTAA) class (A|B|C) name
		std::map<std::string, std::string> properties;
		time_t t(time(NULL));
		properties["addr"] = config.addr;						// addr network address string
		properties["fport"] = std::to_string(config.fport);		// application port number (1..223). 0- MAC, 224- test, 225..255- reserved
		properties["time"] = std::to_string(t);					// time (seconds since Unix epoch)
		properties["timestamp"] = time2string(t);				// timestamp string
		// TODO get it from command line
		properties["deveui"] = "3232323232323232";				// eui global end-device identifier in IEEE EUI64 address space
		properties["name"] = "device32";						// device name
		properties["activation"] = "ABP";						// (ABP|OTAA)
		properties["class"] = "C";								// A|B|C
 
		deviceId.setProperties(properties);

		doInsert(env, &config, &dbAny, config.message_type, config.payload, properties, config.verbosity);

		if (identityService)
			delete identityService;
	}

	donePkt2(env);
	return 0;
}
