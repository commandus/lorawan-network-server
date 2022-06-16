#include <sstream>
#include "database-config-json.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#pragma clang diagnostic pop

#include "errlist.h"
#include "utilstring.h"

// Copy declaration from pkt2.proto
enum SQL_DIALECT : int {
    SQL_POSTGRESQL = 0,
    SQL_MYSQL = 1,
    SQL_FIREBIRD = 2,
    SQL_SQLITE = 3
};

ConfigDatabasesJson::ConfigDatabasesJson(const std::string &filename)
{
    std::string v = file2string(filename.c_str());
    load(v);
}

static const char *JSON_FLD_NAME[10] = {
    "id", "name", "type", "active", "connection",
    "login", "password", "table_aliases", "field_aliases", "properties"
};

/**
 * Each database entry looks like shown below:
 * "id": 3,
 * "name": "postgres",
 * "type": "postgresql",
 * "active": false,
 * "connection": "postgresql://irthermometer...",
 * "login"
 * "password"
 * "table_aliases": [ ["iridium.IEPacket", "iridium_packet"],...
 * "field_aliases": [ ["iridium.IEPacket", "iridium_packet"],...
 * "properties": [ ["addr", "loraaddr"],
 * @param value JSON value
 * @return 0- success
 */
int ConfigDatabasesJson::load(
    const std::string &value
) {
    int r = 0;
    rapidjson::Document doc;
    doc.Parse(value.c_str());
    if (!doc.IsArray())
        return ERR_CODE_INVALID_JSON;
    for (int i = 0; i < doc.Size(); i++) {
        rapidjson::Value &db = doc[i];
        if (!db.IsObject())
            continue;

        ConfigDatabase cdb;
        cdb.id = 0;
        cdb.active = true;
        cdb.port = 0;

        // id
        if (db.HasMember(JSON_FLD_NAME[0])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[0]];
            if (v.IsInt())
                cdb.id = v.GetInt();
        }
        // "name"
        if (db.HasMember(JSON_FLD_NAME[1])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[1]];
            if (v.IsString())
                cdb.name = v.GetString();
        }
        // "type"
        if (db.HasMember(JSON_FLD_NAME[2])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[2]];
            if (v.IsString())
                cdb.type = v.GetString();
        }
        // "active"
        if (db.HasMember(JSON_FLD_NAME[3])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[3]];
            if (v.IsBool())
                cdb.active = v.GetBool();
        }
        // "connection"
        if (db.HasMember(JSON_FLD_NAME[4])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[4]];
            if (v.IsString())
                cdb.connectionString = v.GetString();
        }
        // "login"
        if (db.HasMember(JSON_FLD_NAME[5])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[5]];
            if (v.IsString())
                cdb.login = v.GetString();
        }
        // "password"
        if (db.HasMember(JSON_FLD_NAME[6])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[6]];
            if (v.IsString())
                cdb.password = v.GetString();
        }
        // "table_aliases"
        if (db.HasMember(JSON_FLD_NAME[7])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[7]];
            if (v.IsArray()) {
                for (int c = 0; c < v.Size(); c++) {
                    rapidjson::Value &e = v[i];
                    if (!e.IsArray())
                        continue;
                    if (!e.Size() < 1)
                        continue;
                    rapidjson::Value &ek = e[0];
                    if (!ek.IsString())
                        continue;
                    std::string k = ek.GetString();
                    std::string kv;
                    if (!e.Size() >= 2) {
                        rapidjson::Value &e2 = e[1];
                        if (ek.IsString())
                            kv = e2.GetString();
                    }
                    cdb.tableAliases[k] = kv;
                }
            }
        }
        // "field_aliases"
        if (db.HasMember(JSON_FLD_NAME[8])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[8]];
            if (v.IsArray()) {
                for (int c = 0; c < v.Size(); c++) {
                    rapidjson::Value &e = v[i];
                    if (!e.IsArray())
                        continue;
                    if (!e.Size() < 1)
                        continue;
                    rapidjson::Value &ek = e[0];
                    if (!ek.IsString())
                        continue;
                    std::string k = ek.GetString();
                    std::string kv;
                    if (!e.Size() >= 2) {
                        rapidjson::Value &e2 = e[1];
                        if (ek.IsString())
                            kv = e2.GetString();
                    }
                    cdb.fieldAliases[k] = kv;
                }
            }
        }
        // "properties"
        if (db.HasMember(JSON_FLD_NAME[9])) {
            rapidjson::Value &v = db[JSON_FLD_NAME[9]];
            if (v.IsArray()) {
                for (int c = 0; c < v.Size(); c++) {
                    rapidjson::Value &e = v[i];
                    if (!e.IsArray())
                        continue;
                    if (!e.Size() < 1)
                        continue;
                    rapidjson::Value &ek = e[0];
                    if (!ek.IsString())
                        continue;
                    std::string k = ek.GetString();
                    std::string kv;
                    if (!e.Size() >= 2) {
                        rapidjson::Value &e2 = e[1];
                        if (ek.IsString())
                            kv = e2.GetString();
                    }
                    cdb.properties[k] = kv;
                }
            }
        }

        dbs.push_back(cdb);
    }
    return 0;
}

int ConfigDatabasesJson::getDialect(int index) const
{
    const ConfigDatabase &db = dbs.at(index);
    if (db.type == "postgresql")
        return SQL_POSTGRESQL;
    else
    if (db.type == "mysql")
        return SQL_MYSQL;
    else
    if (db.type == "firebird")
        return SQL_FIREBIRD;
    else
    if (db.type == "sqlite3")
        return SQL_SQLITE;
    else
        return SQL_SQLITE;
}

std::string ConfigDatabasesJson::toString() const
{
    std::stringstream ss;
    ss << "[";
    bool isNext = false;
    for (int i = 0; i < dbs.size(); i++) {
        if (isNext) {
            ss << ", ";
        } else {
            isNext = true;
        }
        ss << toString(i) << std::endl;
    }
    ss << "]";
    return ss.str();
}

std::string ConfigDatabasesJson::toString(int index) const
{
    const ConfigDatabase &db = dbs.at(index);
    std::stringstream ss;
    ss << "{"
       << "\"id\": " << db.id << ", "
       << "\"name\": \"" << db.name << "\", "
       << "\"type\": \"" << db.type << "\", "
       << "\"active\": " << (db.active ? "true" : "false" ) << ", "
       << "\"connection\": \"" << db.connectionString << "\", "
       << "\"login\": \"" << db.login << "\", "
       << "\"password\": \"" << db.password << "\", "
       << "\"db\": \"" << db.db << "\"";

    if (db.tableAliases.size()) {
        ss << ", \"table_aliases\": [";
        bool isNext = false;
        for (std::map<std::string, std::string>::const_iterator it(db.tableAliases.begin()); it != db.tableAliases.end(); it++) {
            if (isNext) {
                ss << ", ";
            } else {
                isNext = true;
            }
            ss << "[\"" << it->first << "\", \"" << it->second << "\"]";
        }
        ss << "]";
    }

    if (db.fieldAliases.size()) {
        ss << ", \"field_aliases\": [";
        bool isNext = false;
        for (std::map<std::string, std::string>::const_iterator it(db.fieldAliases.begin()); it != db.fieldAliases.end(); it++) {
            if (isNext) {
                ss << ", ";
            } else {
                isNext = true;
            }
            ss << "[\"" << it->first << "\", \"" << it->second << "\"]";
        }
        ss << "]";
    }

    if (db.properties.size()) {
        ss << ", \"properties\": [";
        bool isNext = false;
        for (std::map<std::string, std::string>::const_iterator it(db.properties.begin()); it != db.properties.end(); it++) {
            if (isNext) {
                ss << ", ";
            } else {
                isNext = true;
            }
            ss << "[\"" << it->first << "\", \"" << it->second << "\"]";
        }
        ss << "]";
    }

    ss << "}";
    return ss.str();
}

const ConfigDatabase *ConfigDatabasesJson::findByName(const std::string &name) const
{
    for (std::vector<ConfigDatabase>::const_iterator it(dbs.begin()); it != dbs.end(); it++ ) {
        if (it->name == name) {
            return &(*it);
        }
    }
    return nullptr;
}

void ConfigDatabasesJson::setProperties(
    std::map<std::string, std::string> &retval,
    const std::map<std::string, std::string> &values,
    int index
) const
{
    const ConfigDatabase &db = dbs.at(index);
    // copy only values listed in aliases, and replace key to the alias name
    for (std::map<std::string, std::string>::const_iterator it(db.properties.begin()); it != db.properties.end(); it++) {
        std::map<std::string, std::string>::const_iterator f = values.find(it->first);
        if (f != values.end()) {
            if (!it->second.empty()) {
                retval[it->second] = f->second;
            }
        }
    }
}
