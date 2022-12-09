/**
 * Simple LoRaWAN network server PKT2 plugin
 * MQTT plugin parse binary packet by protobuf description
 * LoRaWAN network server's configuration file (lorawan-network-server.json)
 * must provide parameters:
 *
 * "pluginsParams": [
 *      ["mqtt-pkt2-proto-dir", "proto"]
 *  ]
 */

#include <sstream>

#include "payload-insert.h"
#include "pkt2/str-pkt2.h"
#include "log-intf.h"
#include "errlist.h"

LogIntf *errLog = nullptr;

extern "C" void *pluginInit(
    void *dbByConfig,   // DatabaseByConfig*
    const std::map<std::string, std::vector <std::string> > &params,
    LogIntf *log,
    int verbosity   // always 0
)
{
    std::map<std::string, std::vector<std::string> >::const_iterator pit = params.find("mqtt-pkt2-proto-dir");
    std::string protoPath;
    if (pit != params.end()) {
        if (!pit->second.empty() && !pit->second[0].empty()) {
            protoPath = pit->second[0];
        }
    }

    errLog = log;
    if (errLog) {
        std::stringstream ss;
        ss << "Plugin pkt2 initialized. Proto path: " << protoPath;
        errLog->logMessage(nullptr, LOG_DEBUG, LOG_PLUGIN_PKT2, 0, ss.str());
    }
    return initPkt2(protoPath, verbosity);
}

extern "C" void pluginDone(
    void *env
)
{
    donePkt2(env);
}

extern "C" int payloadCreate(
    std::string &retVal,
    void *env,
    const std::string &message,
    int outputFormat,
    int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties
)
{
    retVal = createTableSQLClause(env, message, OUTPUT_FORMAT_SQL, sqlDialect,
        tableAliases, fieldAliases, properties);
    return 0;
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
    if (errLog)
        errLog->logMessage(nullptr, LOG_DEBUG, LOG_PLUGIN_PKT2, 0, s);
            
    if (!s.empty())
        retClauses.push_back(s);
    return 0;
}

// payloadPrepare
