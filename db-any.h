#ifndef DB_ANY_H_
#define DB_ANY_H_	1

#include "db-intf.h"
#include "pkt2/database-config.h"

class DatabaseNConfig
{
public:
	const ConfigDatabase *config;
	DatabaseIntf* db;

	DatabaseNConfig(const ConfigDatabase *cd);
	~DatabaseNConfig();
	std::string selectClause(void *env, const std::string &message);
	std::string createClause(void *env, const std::string &message);
	std::string insertClause(void *env, const std::string &message, int inputFormat, const std::string &data);
	int createTable(void *env, const std::string &message);
	int insert(void *env, const std::string &message, int inputFormat, const std::string &data);
	int open();
	int close();
	int exec(const std::string &statement);
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

	// deprecated ;)
	DatabaseIntf* getDb(const ConfigDatabase **retConfig, int id) const;
	DatabaseIntf* findDb(const ConfigDatabase **retConfig, const std::string &name) const;

	DatabaseNConfig *get(int id) const;
	DatabaseNConfig *find(const std::string &name) const;
};

#endif
