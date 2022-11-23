#include "payload-insert.h"
#include "pkt2/str-pkt2.h"
#include "log-intf.h"

extern "C" void *pluginInit(
    void *dbByConfig,   // DatabaseByConfig*
    const std::string &protoPath,
    const std::string &dbName,
    const std::vector <std::string> &extras,
    LogIntf *log,
    int verbosity   // always 0
)
{
    return initPkt2(protoPath, verbosity);
}

extern "C" void pluginDone(
    void *env
)
{
    donePkt2(env);
}

std::string payloadCreate(
    void *env,
    const std::string &message,
    int outputFormat,
    int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties
)
{
    return createTableSQLClause(env, message, OUTPUT_FORMAT_SQL, sqlDialect,
        tableAliases, fieldAliases, properties);
}

/**
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
extern "C" int payload2InsertClauses(
    std::vector<std::string> &retClauses,
    void *env,
    const std::string &message,
    int inputFormat,
    int outputFormat,
    int sqlDialect, // SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::string &data,
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties,
    const std::string &nullValueString
)
{
    std::string s = parsePacket(env, inputFormat, outputFormat, sqlDialect, data, message,
        tableAliases, fieldAliases, properties);
    if (!s.empty())
        retClauses.push_back(s);
    return 0;
}

// payloadPrepare
