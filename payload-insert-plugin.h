#ifndef PAYLOAD_INSERT_PLUGIN_H_
#define PAYLOAD_INSERT_PLUGIN_H_	1

#ifdef _MSC_VER
#include <Windows.h>
#define PLUGIN_FILE_NAME_SUFFIX ".dll"
#else
#define PLUGIN_FILE_NAME_SUFFIX ".so"
typedef void * HINSTANCE;
#endif

#include "payload-insert.h"
#include "log-intf.h"

/**
 * Get output format name
 * @param index output format number 0- json, 1- csv, 2- tab, 3- sql, 4- sql2, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv_header, 12- tab_header 13- insert
 * @return output format name, "" if invalid
 */
std::string getOutputFormatName(int index);

/**
 * Get output format index
 * @param name output format number 0- json, 1- csv, 2- tab, 3- sql, 4- sql2, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv_header, 12- tab_header 13- insert
 * @return -1- inavalid name, 0..13:  output format index
 */
int getOutputFormatNumber(const std::string &name);


class DatabaseByConfig;

class PayloadInsertPlugins
{
private:
    std::vector<HINSTANCE> handles;
    std::vector<Payload2InsertPluginInitFuncs> funcs;
protected:
    int push(const std::string &file);
public:
    DatabaseByConfig *dbByConfig;
	std::vector <Payload2InsertPluginInitFuncs> plugins;
	PayloadInsertPlugins();
	// load plugins
	PayloadInsertPlugins(const std::string &pluginDirectory);
	// unload plugins
	~PayloadInsertPlugins();

    int init(const std::map<std::string, std::vector <std::string> > &params,
        LogIntf *logCallback, int rfu);
    void done();

    void prepare(uint32_t addr, const std::string &data);
    /**
     * Call appropriate callback from the loaded .so libraries.
     * @return count of added to the retClauses "INSERT" clauses. Return <0 if data is not parseable.
     * If return value is <0, next loaded function would be called.
     * If return value is 0 or >0, chain of loaded function is cancelled.
     */
    int insert(
        std::vector<std::string> &retClauses,
        const std::string &message,
        int inputFormat,
        int outputFormat,
        int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
        const std::string &data,
        const std::map<std::string, std::string> *tableAliases,
        const std::map<std::string, std::string> *fieldAliases,
        const std::map<std::string, std::string> *properties,
        const std::string &nullValueString
    );
    // clean up after successful insert
    void afterInsert();
    std::string create(
        const std::string &message,
        int outputFormat,
        int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
        const std::map<std::string, std::string> *tableAliases,
        const std::map<std::string, std::string> *fieldAliases,
        const std::map<std::string, std::string> *properties
    );

    // load plugins
	int load(const std::string &pluginDirectory, const std::string &suffix = PLUGIN_FILE_NAME_SUFFIX);
	void unload();
};
#endif
