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

#include <google/protobuf/message.h>

#include "argtable3/argtable3.h"

#include "platform.h"
#include "utilstring.h"

#include "errlist.h"
#include "utilstring.h"
#include "utildate.h"

#include "pkt2/str-pkt2.h"
#include "db-any.h"

#include "config-filename.h"

const std::string progname = "proto-db";
#define DEF_CONFIG_FILE_NAME ".proto-db.json"

class Configuration {
public:
	std::string command;				// print|list|create|insert
	std::string proto_path;				// proto files directory. Default 'proto
	std::string dbconfig;				// Default dbs.js'
	std::vector<std::string> dbname;	// database names

	std::string payload;				// hex-string

	int offset;							// list command, offset. Default 0.
	int limit;							// list command, limit size. Default 10
	std::vector<std::string> sort_asc;	// list command, sort by field ascending
	std::vector<std::string> sort_desc;	// list command, sort by field descending

	std::string message_type;
	
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
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
	Configuration *config,
	int argc,
	char *argv[])
{
	struct arg_str *a_command, *a_proto_path, *a_dbconfig, *a_dbname, *a_message_type, *a_payload_hex,
		*a_sort_asc, *a_sort_desc;
	struct arg_int *a_offset, *a_limit;
	struct arg_lit *a_verbosity, *a_help;
	struct arg_end *a_end;

	void *argtable[] = {
		a_command = arg_str0(NULL, NULL, "<command>", "print|list|create|insert. Default print"),
		a_proto_path = arg_str0("p", "proto", "<path>", "proto files directory. Default 'proto'"),
		a_dbconfig = arg_str0("c", "dbconfig", "<file>", "database config file name. Default 'dbs.js'"),
		a_dbname = arg_strn("d", "dbname", "<database-name>", 0, 100, "database name, Default all"),

		a_message_type = arg_str0("m", "message", "<packet.message>", "Message type packet and name"),
		
		a_payload_hex = arg_str0("x", "hex", "<hex-string>", "print, insert command, payload data."),

		a_offset = arg_int0("o", "offset", "<number>", "list command, offset. Default 0."),
		a_limit = arg_int0("l", "limit", "<number>", "list command, limit size. Default 10."),
		a_sort_asc = arg_strn("s", "asc", "<field-name>", 0, 100, "list command, sort by field ascending."),
		a_sort_desc = arg_strn("S", "desc", "<field-name>", 0, 100, "list command, sort by field descending."),
		
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

		if (a_dbconfig->count)
			config->dbconfig = *a_dbconfig->sval;
		else
			config->dbconfig = "dbs.js";

		if (a_message_type->count)
			config->message_type = *a_message_type->sval;
		else
			config->message_type = "";

		for (int i = 0; i < a_dbname->count; i++) {
			config->dbname.push_back(a_dbname->sval[i]);
		}

		if (a_payload_hex->count)
			config->payload = *a_payload_hex->sval;
		else
			config->payload = "";

		if (a_offset->count)
			config->offset = *a_offset->ival;
		else
			config->offset = 0;

		if (a_limit->count)
			config->limit = *a_offset->ival;
		else
			config->limit = 10;

		for (int i = 0; i < a_sort_asc->count; i++) {
			config->sort_asc.push_back(a_sort_asc->sval[i]);
		}
		for (int i = 0; i < a_sort_desc->count; i++) {
			config->sort_desc.push_back(a_sort_desc->sval[i]);
		}

		if (a_limit->count)
			config->limit = *a_offset->ival;
		else
			config->limit = 10;

		config->verbosity = a_verbosity->count;
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
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
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
			ss << "SELECT ";
			if (config->limit) {
				ss << " FIRST " << config->limit;
			}
			if (config->offset) {
				ss << " SKIP " << config->offset;
			}
			ss << " \"imei\", \"version\", \"status\",  \"status\", \"recvno\",  \"sentno\", \"recvtime\", \"gps_time\", \"iridium_latitude\", \"iridium_longitude\", \"gps_latitude\", \"gps_longitude\" ";
		} else {
			ss << "SELECT * ";
		}
		ss << " FROM " << quote << t << quote;

		if (config->sort_asc.size() || config->sort_desc.size()) {
			ss << " ORDER BY ";
		}
		bool isFirst = true;
		for (std::vector<std::string>::const_iterator it(config->sort_asc.begin()); it != config->sort_asc.end(); it++)  {
			if (isFirst)
				isFirst = false;
			else
				ss << ", ";
			ss << *it;
		}

		for (std::vector<std::string>::const_iterator it(config->sort_desc.begin()); it != config->sort_desc.end(); it++)  {
			if (isFirst)
				isFirst = false;
			else
				ss << ", ";
			ss << *it << " DESC";
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
	const std::string &hexData
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

		r = db->insert(env, messageType, INPUT_FORMAT_HEX, hexData);

		if (r) {
			std::cerr << ERR_DB_INSERT << r << " database " << *it << ": " << db->db->errmsg << std::endl;
			std::cerr << "SQL statement: " << db->insertClause(env, messageType, INPUT_FORMAT_HEX, hexData) << std::endl;
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
	std::cout << parsePacket(env, 1, 0, 0, hexData, forceMessageType, NULL, NULL) << std::endl;
}

int main(
	int argc,
	char *argv[])
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

	ConfigDatabases configDatabases(config.dbconfig);
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
			parsePacket2ProtobufMessage((void**) &m, env, 1, config.payload, "", NULL, NULL);
			if (m)
				config.message_type = m->GetTypeName();
			doCreate(env, &config, &dbAny, config.message_type);
		}
	}
	if (config.command == "insert")
		doInsert(env, &config, &dbAny, config.message_type, config.payload);
	donePkt2(env);
	return 0;
}
