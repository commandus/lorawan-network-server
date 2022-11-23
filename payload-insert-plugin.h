#ifndef PAYLOAD_INSERT_PLUGIN_H_
#define PAYLOAD_INSERT_PLUGIN_H_	1

#include "payload-insert.h"
#include "log-intf.h"

class DatabaseByConfig;

class PayloadInsertPlugins
{
private:
    std::vector<void *> handles;
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

    int init(const std::string &protoPath, const std::string &dbName, const std::vector <std::string> &extras,
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
	int load(const std::string &pluginDirectory, const std::string &suffix = ".so");
	void unload();
};
#endif
