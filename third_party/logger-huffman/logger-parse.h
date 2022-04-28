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

class LoggerKosaCollection;

/**
 * Return CREATE table SQL clause in 
 * @param retClauses vector of CREATE statements
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @return count of statements, <0- error
 */
int sqlCreateTable(
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL
    
);

/**
 * Return CREATE table SQL clause
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @param separator  separator string default space
 * @return empty string if fails
 */
std::string sqlCreateTable1(
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL,
    const std::string &separator = " "
);

void *initLoggerParser(
    const std::string &passportDir,     ///< passport files root
    LOG_CALLBACK onLog                  ///< log callback
);

void *initLoggerParser(
    const std::vector<std::string> &passportDirs,     ///< passport files root
    LOG_CALLBACK onLog                  ///< log callback
);

void flushLoggerParser(void *env);
void doneLoggerParser(void *env);

void *getPassportDescriptor(void *env);
void *getLoggerKosaCollection(void *env);

/**
 * Return state of the desctiptor
 * @param env descriptor
 * @param format 0- Postgres, 1- MySQL, 2- Firebird, 3- SQLite 4- JSON, 5- text, 6- table
 */
std::string loggerParserState(void *env, int format);

int parsePacket(void *env, const std::string &packet);

/**
 * Return INSERT clause(s) in retClauses
 * @param env desciptor
 * @param retClauses vector of INSERT statements
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 * @return 0- success
 */
int sqlInsertPackets(
    void *env,
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL
);

/**
 * Remove completed or expired items
 * @param env descriptor
 */
void rmCompletedOrExpired(
    void *env
);

/**
 * Return INSERT clause(s) as one string
 * @param env desciptor
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 * @param separator  separator string default space
 * @return empty string if fails
 */
std::string sqlInsertPackets1(
    void *env,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL,
    const std::string &separator = " "
);

/**
 * Return INSERT raw data (as hex)
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 * @return empty string if fails
 */
std::string sqlInsertRaw(
    int sqlDialect,
    const std::string &value,
    const std::map<std::string, std::string> *extraValues = NULL
);

#endif
