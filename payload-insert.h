#ifndef PAYLOAD_INSERT_H_
#define PAYLOAD_INSERT_H_	1

#include <vector>
#include <map>
#include <string>
#include "log-intf.h"

class DatabaseByConfig;

/**
 * Optional initialize function
 * @return env
 */
typedef void *(*pluginInitFunc)(
    DatabaseByConfig *dbByConfig,
    const std::map<std::string, std::vector <std::string> > &params,
    LogIntf *log,
    int rfu
);

/**
 * Optional destroy function
 * @return env
 */
typedef void(*pluginDoneFunc)(
    void * env
);

#define SQL_POSTGRESQL 0
#define SQL_MYSQL 1
#define SQL_FIREBIRD 2
#define SQL_SQLITE 3

/**
 * lorawan-network-server loads shared libraries (.so) in the "payload-plugins" folder.
 * If library has function named "payload2InsertClauses", this function adds to the parsers chain.
 * 
 * Function must have exported name "payload2InsertClauses".
 * 
 * Function must have signature as declared below otherwise lorawan-network-server crash.
 * 
 * @return count of added to the retClauses "INSERT" clauses. Return <0 if data is not parseable.
 * If return value is <0, next loaded function would be called.
 * If return value is 0 or >0, chain of loaded function is cancelled.
 * @param retClauses return INSERT clauses(usually one)
 * @param env
 * @param message name of preferred handler (message type name). Default "".
 * @param inputFormat: 0- binary (always) 1- hex (never used)
 * @param outputFormat 0- json 3- sql
 * @param sqlDialect SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
 * @param data: payload
 * @param tableAliases optional aliases (not used)
 * @param fieldAliases optional aliases (not used)
 * @param properties: LoRaWAN metadata properties
 * @param nullValueString: magic number "8888" by default. You can set it to "NULL".
 *
 * Property keys are:
 * activation (ABP|OTAA)
 * class A|B|C
 * deveui global end-device identifier in IEEE EUI64 address space
 * appeui
 * appKey
 * nwkKey
 * devNonce
 * joinNonce
 * name device name
 * version LoRaWAN version
 * addr network address string
 * fport application port number (1..223). 0- MAC, 224- test, 225..255- reserved as decimal number string
 * id packet id
 * time (32 bit integer, seconds since Unix epoch) as decimal number string
 * timestamp string
 * id number of packet received by the server as decimal number string
 */
typedef int (*payload2InsertClausesFunc)(
    std::vector<std::string> &retClauses,
    void *env,
    const std::string &message,
    int inputFormat,
    int outputFormat,
    int sqlDialect,
    const std::string &data,
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties,
	const std::string &nullValueString
);

typedef int (*payloadPrepareFunc)(
    void *env,
    uint32_t addr,
    const std::string &payload
);

/**
 * Return CREATE TABLE clauses
 * Optional call.
 */
typedef int (*payloadCreateFunc)(
    std::string &retVal,
    void *env,
    const std::string &message,
    int outputFormat,
    int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties
);

typedef void (*payloadAfterInsertFunc)(void *env);


class Payload2InsertPluginInitFuncs {
public:
    void *env;
    pluginInitFunc init;
    pluginDoneFunc done;
    payloadPrepareFunc prepare;
    payload2InsertClausesFunc insert;
    payloadAfterInsertFunc afterInsert;
    payloadCreateFunc create;
};

#endif
