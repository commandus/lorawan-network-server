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
    bool active;
	std::string connectionString;
	std::string login;
	std::string password;
	std::string db;
	int port;

	std::map<std::string, std::string> tableAliases;
	std::map<std::string, std::string> fieldAliases;
	std::map<std::string, std::string> properties;
};

class ConfigDatabasesIntf
{
public:
    std::vector<ConfigDatabase> dbs;

    virtual int load(const std::string &value) = 0;
    virtual std::string toString(int index) const = 0;

    virtual int getDialect(int index) const = 0;
    virtual std::string toString() const = 0;
    virtual const ConfigDatabase *findByName(const std::string &name) const = 0;
    virtual void setProperties(
        std::map<std::string, std::string> &retval,
        const std::map<std::string, std::string> &values,
        int index
    ) const = 0;

};

class ConfigDatabases : public ConfigDatabasesIntf
{
public:
	ConfigDatabases(const std::string &filename);
	int load(const std::string &value) override;

    int getDialect(int index) const override;
	std::string toString() const override;
    std::string toString(int index) const override;
	const ConfigDatabase *findByName(const std::string &name) const override;
    void setProperties(
        std::map<std::string, std::string> &retval,
        const std::map<std::string, std::string> &values,
        int index
    ) const override;

};

#endif
