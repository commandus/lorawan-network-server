/**
 * @brief LoRaWAN 1.0.x packet decoder utility
 * @file lora-print.cpp
 * Copyright (c) 2021 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
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

#include "packet-queue.h"
#include "pkt2/str-pkt2.h"
#include "db-any.h"

#include "config-filename.h"

#include "identity-service-file-json.h"
#include "identity-service-dir-txt.h"
#include "utilidentity.h"

#ifdef ENABLE_LMDB
#include "identity-service-lmdb.h"
#endif			

#define DEF_IDENTITY_STORAGE_NAME	"identity.json"
#define DEF_IDENTITY_STORAGE_TYPE	"json"

const std::string programName = "lora-print";
#define DEF_CONFIG_FILE_NAME ".lora-print.json"

class LoraPrintConfiguration {
public:
	std::string command;				// json|sql
	std::string proto_path;				// proto files directory. Default 'proto
	std::string dbconfig;				// Default dbs.js'
	std::vector<std::string> dbname;	// database names
	std::string payload;				// hex-string
	std::string message_type;
	// identity service
	std::string identityStorageName;
	IDENTITY_STORAGE identityStorageType;
	int outputFormat;					// 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv header, 12- tab header
	int verbosity;						// verbosity level
};

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
	struct arg_str *a_command, *a_proto_path, *a_dbconfig, *a_dbname, *a_message_type, 
		*a_payload_hex, *a_payload_base64,
		*a_identityStorageName,
		*a_identityStorageType;
	struct arg_int *a_output_format;
	struct arg_lit *a_verbosity, *a_help;
	struct arg_end *a_end;

	void *argtable[] = {
		a_command = arg_str0(NULL, NULL, "<command>", "json|sql. Default json"),

		a_payload_hex = arg_str0("x", "hex", "<hex-string>", "LoraWAN packet to decode, hexadecimal string."),
		a_payload_base64 = arg_str0("6", "base64", "<base64>", "same, base64 encoded."),

		a_proto_path = arg_str0("p", "proto", "<path>", "proto files directory. Default 'proto'"),
		a_message_type = arg_str0("m", "message", "<pkt.msg>", "force nessage type packet and name"),

		a_identityStorageName = arg_str0("i", "id-name", "<name>", "default " DEF_IDENTITY_STORAGE_NAME),
		a_identityStorageType = arg_str0("y", "id-type", "json|txt|lmdb", "default " DEF_IDENTITY_STORAGE_TYPE),

		a_output_format = arg_int0("f", "format", "<0..8, 11..12>", "0- json(default), 1- csv, 2- tab, 3- sql, 4- sql, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv header, 12- tab header"),

		a_dbconfig = arg_str0("c", "dbconfig", "<file>", "database config file name. Default 'dbs.js'"),
		a_dbname = arg_strn("d", "dbname", "<database>", 0, 100, "database name, Default all"),

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

		config->payload = "";
		if (a_payload_hex->count)
			config->payload = hex2string(*a_payload_hex->sval);
		if (a_payload_base64->count) {
			try {
				config->payload = base64_decode(*a_payload_base64->sval, false);
			}
			catch (const std::exception& e) {
				std::cerr << ERR_INVALID_BASE64 << std::endl;
				nerrors++;
			}
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
		std::cerr << ERR_MESSAGE << ERR_CODE_WRONG_PARAM << ": " << ERR_WRONG_PARAM << std::endl;
		nerrors++;
	}

	if (config->command.empty())
		config->command = "json";

	if (config->payload.empty()) {
		std::cerr << ERR_MESSAGE << ERR_CODE_NO_PAYLOAD << ": " << ERR_NO_PAYLOAD << std::endl;
		nerrors++;
	}

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, programName.c_str());
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_PROTO_DB_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

void doInsert
(
	void* env,
	LoraPrintConfiguration *config,
	DatabaseByConfig *databaseByConfig,
	const std::string &messageType,
	const std::string &binData,
	const std::map<std::string, std::string> *properties
)
{
	for (std::vector<std::string>::const_iterator it(config->dbname.begin()); it != config->dbname.end(); it++) {
		
		DatabaseNConfig *db = databaseByConfig->find(*it);
		if (!db) {
			std::cerr << ERR_DB_DATABASE_NOT_FOUND << *it << std::endl;
			exit(ERR_CODE_DB_DATABASE_NOT_FOUND);
		}
		std::cout << db->insertClause(env, messageType, INPUT_FORMAT_BINARY, binData, properties) << std::endl;
	}
}

/**
 * Print LoraWan packet
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv header, 12- tab header
 */ 
void doPrint
(
	void* env,
	LoraPrintConfiguration *config,
	const std::string &forceMessageType,
	int outputFormat,
	const std::string &binData
)
{
	std::cout << parsePacket(env, INPUT_FORMAT_BINARY, outputFormat, 0, binData, forceMessageType, NULL, NULL, NULL) << std::endl;
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

	DatabaseByConfig databaseByConfig(&configDatabases);

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

	SEMTECH_PREFIX_GW dataprefix;
	GatewayStat gatewayStat;
	std::vector<SemtechUDPPacket> packets;

	int r = SemtechUDPPacket::parse(NULL, dataprefix, gatewayStat, packets, config.payload.c_str(), config.payload.size(), identityService);
	
	if (r) {
        std::cerr << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r) << std::endl;
	}

	if (gatewayStat.errcode == 0) {
		std::cout << gatewayStat.toJsonString() << std::endl;
	}
	for (std::vector<SemtechUDPPacket>::iterator it(packets.begin()); it != packets.end(); it++) {
		if (it->errcode) {
			std::cerr << ERR_MESSAGE << ERR_CODE_INVALID_PACKET << ": " << ERR_INVALID_PACKET << std::endl;
			continue;
		}
		std::cout << it->toJsonString() << std::endl;
	
		/*
		if (config.verbosity > 2) {
			
			if (it->hasApplicationPayload())
				std::cerr << "application payload " << (int) it->getHeader()->fport << std::endl;
			if (it->hasMACPayload())
				std::cerr << "MAC payload " << std::endl;

		}
		*/

		std::string payload = it->payload;
		if (config.command == "sql") {
			std::map<std::string, std::string> properties;
			// set properties addr eui name activation (ABP|OTAA) class (A|B|C) name
			// time  timestamp 
			time_t t(time(NULL));
			properties["addr"] = it->getDeviceAddrStr();							// addr network address string
			properties["fport"] = std::to_string((int) it->header.fport);			// application port number (1..223). 0- MAC, 224- test, 225..255- reserved
			properties["time"] = std::to_string(t);									// time (32 bit integer, seconds since Unix epoch)
			properties["timestamp"] = time2string(t);								// timestamp string
			properties["eui"] = it->getDeviceEUI();									// eui global end-device identifier in IEEE EUI64 address space
			properties["name"] = it->devId.name;									// device name
			properties["activation"] =  activation2string(it->devId.activation);	// (ABP|OTAA)
			properties["class"] = deviceclass2string(it->devId.deviceclass);		// A|B|C

			doInsert(env, &config, &databaseByConfig, config.message_type, payload, &properties);
		} else {
			doPrint(env, &config, config.message_type, config.outputFormat, payload);
		}

	}

	if (identityService)
		delete identityService;
	donePkt2(env);
	return 0;
}
