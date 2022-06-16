#ifndef DATABASE_CONFIG_JSON_H
#define DATABASE_CONFIG_JSON_H 1

#include "pkt2/database-config.h"

class ConfigDatabasesJson : public ConfigDatabasesIntf
{
public:
    ConfigDatabasesJson(const std::string &filename);

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
