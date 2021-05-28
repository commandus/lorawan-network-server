#include <map>
#include <string>

class ConfigDatabase
{
public:
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
};
