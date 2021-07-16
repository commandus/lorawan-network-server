#ifndef DATABASE_CONFIG_H_
#define DATABASE_CONFIG_H_	1

#include <map>
#include <vector>
#include <string>

class ConfigDatabase
{
public:
	int id;	// negative id means do not serve. If two database has same id, message stored in one of them
	std::string name;
	std::string type;
	std::string connectionString;
	std::string login;
	std::string password;
	std::string db;
	int port;

	std::map<std::string, std::string> tableAliases;
	std::map<std::string, std::string> fieldAliases;
	std::map<std::string, std::string> properties;

	ConfigDatabase();
	std::string toString() const;
	int getDialect() const;
	void setProperties(
		std::map<std::string, std::string> &retval,
		const std::map<std::string, std::string> &values
	) const;
};

class ConfigDatabases
{
public:
	std::vector<ConfigDatabase> dbs;
	
	ConfigDatabases(const std::string &filename);
	void load(const std::string &value);
	std::string toString() const;
	const ConfigDatabase *findByName(const std::string &name) const;
};

#endif
