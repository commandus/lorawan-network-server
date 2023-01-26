#include "payload-insert.h"
#include <sstream>

#include "db-any.h"
#include "logger-huffman/logger-parse.h"
#include "logger-loader.h"
#include "log-intf.h"
#include "errlist.h"

#ifdef _MSC_VER
#define EXPORT_PLUGIN_FUNC extern "C" __declspec(dllexport)
#else
#define EXPORT_PLUGIN_FUNC extern "C"
#endif

std::string payloadCreate(
    const std::string &message,
    int outputFormat,
    int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties
)
{
    return loggerSQLCreateTable1(sqlDialect);
}

EXPORT_PLUGIN_FUNC void *pluginInit(
    DatabaseByConfig *dbByConfig,
    const std::map<std::string, std::vector <std::string> > &params,
    LogIntf *log,
    int verbosity   // always 0
)
{
    LoggerHuffmanEnv *env = new LoggerHuffmanEnv;
    bool hasLoggerKosaPacketsLoader = false;
    // set database to load from
    std::string loggerDbName;
    std::map<std::string, std::vector <std::string> >::const_iterator pDb = params.find("logger-huffman-database-name");
    if (pDb != params.end()) {
        if (!pDb->second.empty())
            loggerDbName = pDb->second[0];
    }

    // get passport dirs
    std::vector <std::string> passportDirs;
    std::map<std::string, std::vector <std::string> >::const_iterator pPd = params.find("logger-huffman-passport");
    if (pPd != params.end()) {
        passportDirs = pPd->second;
    }

    if (!loggerDbName.empty()) {
        DatabaseNConfig *kldb = nullptr;
        if (dbByConfig)
            kldb = dbByConfig->find(loggerDbName);
        if (kldb) {
            env->loader.setDatabase(kldb->db);
            int r = kldb->open();
            if (r == ERR_CODE_NO_DATABASE) {
                hasLoggerKosaPacketsLoader = false;
            } else {
                hasLoggerKosaPacketsLoader = true;
            }
        }
    }
    if (hasLoggerKosaPacketsLoader) {
        std::stringstream sskldb;
        sskldb << MSG_INIT_LOGGER_HUFFMAN << loggerDbName;
        if (log)
            log->onInfo(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, sskldb.str());
    } else {
        if (log) {
            std::stringstream ss;
            ss << ERR_INIT_LOGGER_HUFFMAN_DB << " \"" << loggerDbName << "\"";
            log->onInfo(nullptr, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_INIT_LOGGER_HUFFMAN_DB,
            ss.str());
        }
    }
    env->env = initLoggerParser(passportDirs,
       [log](void *lenv, int level, int moduleCode, int errorCode, const std::string &message) {
            if (log)
                log->onInfo(nullptr, level, moduleCode, errorCode, message);
        },
        &env->loader);
    if (!env->env) {
        if (log)
            log->onInfo(nullptr, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER,
                ERR_INIT_LOGGER_HUFFMAN_PARSER);
        exit(ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER);
    }
    return env;
}

EXPORT_PLUGIN_FUNC void pluginDone(
    void *env
)
{
    if (env) {
        LoggerHuffmanEnv *e = (LoggerHuffmanEnv *) env;
        doneLoggerParser(e->env);
        delete e;
    }
}

EXPORT_PLUGIN_FUNC void afterInsert(
    void *env
)
{
    if (env) {
        LoggerHuffmanEnv *e = (LoggerHuffmanEnv *) env;
        loggerRemoveCompletedOrExpired(e->env);
    }
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
EXPORT_PLUGIN_FUNC int payload2InsertClauses(
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
    std::string s = loggerSQLInsertRaw(sqlDialect, data, properties);
    int r = 0;
    if (!s.empty()) {
        retClauses.push_back(s);
        if (env) {
            LoggerHuffmanEnv *e = (LoggerHuffmanEnv *) env;
            r = loggerSQLInsertPackets(e->env, retClauses, sqlDialect, properties, nullValueString);
        }
    }
    return r;
}

EXPORT_PLUGIN_FUNC int payloadPrepare(
    void *env,
    uint32_t addr,
    const std::string &payload
) {
    if (env) {
        LoggerHuffmanEnv *e = (LoggerHuffmanEnv *) env;
        return loggerParsePacket(e->env, addr, payload);
    }
    return 0;
}
