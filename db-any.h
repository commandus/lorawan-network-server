#ifndef DB_ANY_H_
#define DB_ANY_H_	1

#include "db-intf.h"
#include "pkt2/database-config.h"
#ifdef ENABLE_PKT2
#include "pkt2/str-pkt2.h"
#endif

class DatabaseNConfig
{
public:
	const ConfigDatabase *config;
	DatabaseIntf* db;
    std::string lastErroneousStatement;

	DatabaseNConfig(const ConfigDatabase *aConfig);
	~DatabaseNConfig();
	std::string tableName(void *env, const std::string &message) const;
	std::string selectClause(void *env, const std::string &message);
	std::string createClause(void *env, const std::string &message) const;
	int insertClauses(std::vector<std::string> &retClauses, void *env, const std::string &message,
                      int inputFormat, const std::string &data,
                      const std::map<std::string, std::string> *properties);
	int createTable(void *env, const std::string &message);
	int insert(void *env, const std::string &message, int inputFormat, const std::string &data, const std::map<std::string, std::string> *properties);
	int open();
	int close() const;
	int exec(const std::string &statement) const;
	int select(std::vector<std::vector<std::string>> &retval, const std::string &statement);
};

/**
 */
class DatabaseByConfig
{
private:
	const ConfigDatabases *config;
protected:
	DatabaseIntf* open(const ConfigDatabase *dbc) const;
public:
	DatabaseByConfig(const ConfigDatabases *config);
	~DatabaseByConfig();

    void prepare(void *env, const std::string &data);

    size_t count() const;
	/**
	 * @param retConfig return config
	 * @param id 0..count() - 1
	 */ 
	DatabaseIntf* getDb(const ConfigDatabase **retConfig, int seqno) const;
	DatabaseIntf* findDb(const ConfigDatabase **retConfig, const std::string &name) const;

	DatabaseNConfig *get(int id) const;
	DatabaseNConfig *find(const std::string &name) const;

	/**
	 * @param retval return dayavase identifier
	 * @return  count of databases
	 */
	size_t getIds(std::vector<int> &retval);
};

#endif
