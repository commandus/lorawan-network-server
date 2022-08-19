#ifndef LOGGER_PARSE_H_
#define LOGGER_PARSE_H_ 1

#include <vector>
#include <map>

#include <functional>

typedef std::function<void(
    void *env,
    int level,
    int modulecode,
    int errorcode,
    const std::string &message)>
LOG_CALLBACK;

class LoggerKosaCollector;

/**
 * Return CREATE table SQL clause in 
 * @param retClauses vector of CREATE statements
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @return count of statements, <0- error
 */
int loggerSQLCreateTable(
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = nullptr
);

/**
 * Return CREATE table SQL clause
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @param separator  separator string default space
 * @return empty string if fails
 */
std::string loggerSQLCreateTable1(
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = nullptr,
    const std::string &separator = " "
);

/**
 * Initialize one passport directory or file
 * @param passportDir passport root directory
 * @param onLog callback function to report an errors
 * @param loggerKosaPacketsLoader packet loader LoggerKosaPacketsLoader class object pointer
 * @return "passport" descriptor
 */
void *initLoggerParser(
    const std::string &passportDir,     ///< passport files root, can be empty
    LOG_CALLBACK onLog = nullptr,                 ///< log callback
    void *loggerKosaPacketsLoader = nullptr
);

/**
 * Initialize more than one passport directory(or files)
 * @param passportDirs list of passport files or directories
 * @param onLog callbac function to report an errors
 * @param loggerKosaPacketsLoader packet loader LoggerKosaPacketsLoader class object pointer
 * @return "passport" descriptor
 */
void *initLoggerParser(
    const std::vector<std::string> &passportDirs,     ///< passport files roots list or passport files
    LOG_CALLBACK onLog = nullptr,                               ///< log callback
    void *loggerKosaPacketsLoader = nullptr
);

void flushLoggerParser(void *env);
void doneLoggerParser(void *env);

void *getPassportDescriptor(void *env);
void *getLoggerKosaCollector(void *env);

/**
 * Return passports as text or json
 * @param env logger descriptor
 * @param format 1- text, 2- JSON, 3- table, 0-nothing
 * @param retVal if NULL, nothing return
 * @param year 0- any, >1- filter by year
 * @param plume 0- any, >1- filter by plume
 * @param offset 0..
 * @param size limit
 * @return passports as text or json
 */
size_t lsPassports(
    void *env,
    int format,
    std::vector<std::string> *retVal,
    int year,
    int plume,
    size_t offset,
    size_t count
);

/**
 * Return state of the desctiptor
 * @param env descriptor
 * @param format 0- Postgres, 1- MySQL, 2- Firebird, 3- SQLite 4- JSON, 5- text, 6- table
 */
std::string loggerParserState(void *env, int format);

int loggerParsePacket(void *env, uint32_t addr, const std::string &packet);
int loggerParsePackets(void *env, uint32_t addr, const std::vector<std::string> &packets);

/**
 * Return INSERT clause(s) in retClauses
 * @param env "passport" descriptor
 * @param retClauses vector of INSERT statements
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 * @param nullValueString default "NULL"
 * @return 0- success
 */
int loggerSQLInsertPackets(
    void *env,
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL,
    const std::string &nullValueString = "NULL"
);

/**
 * Remove completed or expired items
 * @param env descriptor
 */
void loggerRemoveCompletedOrExpired(
    void *env
);

/**
 * loggerSQLInsertPackets wrapper returns INSERT clause(s) as one string
 * @param env descriptor
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 * @param nullValueString default "NULL"
 * @param separator  separator string default space
 * @return empty string if fails
 */
std::string loggerSQLInsertPackets1(
    void *env,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL,
    const std::string &nullValueString = "NULL",
    const std::string &separator = " "
);

/**
 * Received packet can be saved in the "raw" table for reference
 * Return INSERT raw data (as hex)
 * @param sqlDialect 0..3
 * @param value data
 * @param extraValues  <optional field name>=value
 * @return empty string if fails
 */
std::string loggerSQLInsertRaw(
    int sqlDialect,
    const std::string &value,
    const std::map<std::string, std::string> *extraValues = NULL
);

/**
 * Return SQL SELECT statement returning packets as hex strings separated by space.
 * Execute generated SQL query an get first field string value from the first row.
 * This value pass to the loggerParseSQLBaseMeasurement().
 * Get parsed values and construct plume measurements.
 * @param sqlDialect SQL dialect number
 * @param addr LoRaWAN device address 4 bytes long integer
 * @return SQL SELECT statement returning packets as hex strings separated by space9
 */
std::string loggerBuildSQLBaseMeasurementSelect(
    int sqlDialect,
    uint32_t addr
);

/**
 * Read hex strings, return binary strings.
 * Before loggerParseSQLBaseMeasurement() call loggerBuildSQLBaseMeasurementSelect() and get hex strings.
 * @param retClauses return binary packet(s)
 * @param value
 * @return SQL SELECT statement returning packets as hex strings separated by space9
 */
bool loggerParseSQLBaseMeasurement(
    std::vector <std::string> &retClauses,
    const std::string &value
);

#endif
