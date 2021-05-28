#ifndef STR_PKT2_H
#define STR_PKT2_H     1

#include <string>
#include <map>

#include "database-config.h"

#define INPUT_FORMAT_BINARY		0
#define INPUT_FORMAT_HEX		1

#define OUTPUT_FORMAT_JSON		0
#define OUTPUT_FORMAT_CSV		1
#define OUTPUT_FORMAT_TAB		2
#define OUTPUT_FORMAT_SQL		3
#define OUTPUT_FORMAT_SQL2		4
#define OUTPUT_FORMAT_PBTEXT	5
#define OUTPUT_FORMAT_DEBUG		6
#define OUTPUT_FORMAT_HEX		7
#define OUTPUT_FORMAT_BIN		8

/**
 * Initialize packet declarations
 * @param proto_path path to the catalog with protobuf decraration files
 * @param verbosity if 1, 2 or 3 print out to the stderr errors parsing declarations
 * @return structure of the packet declaratuions to be passed to the parsePacket()
 */
void* initPkt2(
	const std::string &proto_path,
	int verbosity
);

/**
 * Destroy and free packet declarations
 * @param env packet declaratuions
 */
void donePkt2(void *env);

/**
 * Parse packet by declaration
 * @param env packet declaratuions
 * @param inputFormat 0- binary, 1- hex string
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin 
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param packet data
 * @param forceMessage "" If specifed, try only message type
 * @return empty string if fails
 */
std::string parsePacket(
	void *env, 
	int inputFormat,
	int outputFormat,
	int sqlDialect,
	const std::string &packet,
	const std::string &forceMessage,
	const std::map<std::string, std::string> *tableAliases = NULL,
	const std::map<std::string, std::string> *fieldAliases = NULL
);

/**
 * Parse packet by declaration
 * @param retMessage return message of google::protobuf::Message type
 * @param env packet declaratuions
 * @param inputFormat 0- binary, 1- hex string
 * @param packet data
 * @param forceMessage "" If specifed, try only message type
 * @return empty string if fails
 */
bool parsePacket2ProtobufMessage(
	void **retMessage,
	void *env, 
	int inputFormat,
	const std::string &packet,
	const std::string &forceMessage,
	const std::map<std::string, std::string> *tableAliases = NULL,
	const std::map<std::string, std::string> *fieldAliases = NULL
);

/**
 * Return field name list string
 * @param env contains options_cache
 * @param message_type
 * @param delimiter "\t" or ", "
 * @return field name list
 */
std::string headerFields(
	void *env, 
	const std::string &message_type,
	const std::string &delimiter
);

/**
 * Return CREATE table SQL clause
 * @param env packet declaratuions
 * @param messageName Protobuf full type name (including packet)
 * @param outputFormat 3- sql, 4- Sql
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param tableAliases <Protobuf full type name>=<alias (SQL table name)>
 * @param fieldAliases <Protobuf message fiekd name>=<alias (SQL column name)>
 * @return empty string if fails
 */
std::string createTableSQLClause(
	void *env, 
	const std::string &messageName,
	int outputFormat,
	int sqlDialect,
	const std::map<std::string, std::string> *tableAliases = NULL,
	const std::map<std::string, std::string> *fieldAliases = NULL
);

#endif