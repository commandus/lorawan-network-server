#include <map>
#include <vector>
#include <string>

class ConfigDatabase
{
public:
	std::string name;
	std::string connectionString;
	std::string login;
	std::string password;
	std::map<std::string, std::string> tableAliases;
	std::map<std::string, std::string> fieldAliases;

	ConfigDatabase();
	std::string toString();
};

class ConfigDatabases
{
public:
	std::vector<ConfigDatabase> dbs;
	
	ConfigDatabases(const std::string &filename);
	void load(const std::string &value);
	std::string toString();
	ConfigDatabase *findByName(const std::string &name);
};
