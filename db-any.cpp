#include "db-any.h"
#ifdef ENABLE_DB_SQLITE
#include "db-sqlite.h"
#endif
#ifdef ENABLE_DB_POSTGRES
#include "db-pg.h"
#endif
#ifdef ENABLE_DB_MYSQL
#include "db-mysql.h"
#endif
#ifdef ENABLE_DB_FIREBIRD
#include "db-fb.h"
#endif
#ifdef ENABLE_DB_JSON
#include "db-json.h"
#endif

#include "errlist.h"

#define JSON_TYPE_NAME "json"

DatabaseNConfig::DatabaseNConfig(
	const ConfigDatabase *aConfig,
    PayloadInsertPlugins *aPlugins
)
    : db(nullptr), plugins(aPlugins)
{
	config = aConfig;
	// unknown database type
	if (config->type == "sqlite3")
#ifdef ENABLE_DB_SQLITE	
		db = new DatabaseSQLite();
#else
	;		
#endif
	else
		if (config->type == "postgresql")
#ifdef ENABLE_DB_POSTGRES			
			db = new DatabasePostgreSQL();
#else
	;		
#endif
		else
			if (config->type == "mysql")
#ifdef ENABLE_DB_MYSQL				
				db = new DatabaseMySQL();
#else
	;		
#endif
			else
				if (config->type == "firebird")
#ifdef ENABLE_DB_FIREBIRD					
					db = new DatabaseFirebird();
#else
	;		
#endif

                else
                    if (config->type == "json")
#ifdef ENABLE_DB_JSON
                        db = new DatabaseJSON();
#else
                    ;
#endif

}

DatabaseNConfig::~DatabaseNConfig()
{
	if (db) {
		db->close();
		delete db;
	}
}

std::string DatabaseNConfig::tableName(
	const std::string &message
) const
{
	std::map<std::string, std::string>::const_iterator it(config->tableAliases.find(message));
	if (it == config->tableAliases.end())
		return message;
	else
		return it->second;
}

int sqlDialectByName(const std::string &name)
{
    if (name == "postgresql")
        return 0;
    if (name == "mysql")
        return 1;
    if (name == "firebird")
        return 2;
    if (name == "sqlite3")
        return 3;
    return 0;
}

std::string DatabaseNConfig::createClause
(
	const std::string &message
) const
{
    int dialect = 0;
    if (db)
        dialect = sqlDialectByName(db->type);   // TODO move name resolve to the constructor
    if (plugins)
        return this->plugins->create(message, config->type == JSON_TYPE_NAME ? 0 : 3,
    dialect,  &config->tableAliases, &config->fieldAliases,&config->properties);
    else
        return "";
}

int DatabaseNConfig::insertClauses(
    std::vector<std::string> &retClauses,
    const std::string &message,
    int inputFormat,
    const std::string &data,
    const std::map<std::string, std::string> *properties,
	const std::string &nullValueString
)
{
    if (!plugins)
        return -1;
    int dialect = 0;
    if (db)
        dialect = sqlDialectByName(db->type);   // TODO move name resolve to the constructor
    int r = plugins->insert(retClauses, message, inputFormat, config->type == JSON_TYPE_NAME ? 0 : 3,
        dialect, data, &config->tableAliases, &config->fieldAliases,
        properties, nullValueString);
    return r;
}

int DatabaseNConfig::createTable(
    const std::string &message
)
{
	std::string clause = createClause(message);
	if (!db)
		return ERR_CODE_NO_DATABASE;
	return db->exec(clause);
}

/**
 * @return 0- success, ERR_CODE_INVALID_PACKET- data is not parseable, otherwise database error code
 */ 
int DatabaseNConfig::insert(
	const std::string &message,
	int inputFormat,
	const std::string &data,
	const std::map<std::string, std::string> *properties,
	const std::string &nullValueString
)
{
    int r = 0;
	std::vector<std::string> clauses;
    insertClauses(clauses, message, inputFormat, data, properties, nullValueString);
	if (clauses.empty())
		return ERR_CODE_INVALID_PACKET;
    for (std::vector<std::string>::const_iterator it(clauses.begin()); it != clauses.end(); it++) {
        r = db->exec(*it);
        if (r) {
            lastErroneousStatement = *it;
            break;
        }
    }
    if (plugins)
        plugins->afterInsert();
    return r;
}

int DatabaseNConfig::select(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	return db->select(retval, statement);
}

void DatabaseNConfig::setProperties(
    std::map<std::string, std::string> &retval,
    const std::map<std::string, std::string> &values
) const
{
    // copy only values listed in aliases, and replace key to the alias name
    for (std::map<std::string, std::string>::const_iterator it(config->properties.begin()); it != config->properties.end(); it++) {
        std::map<std::string, std::string>::const_iterator f = values.find(it->first);
        if (f != values.end()) {
            if (!it->second.empty()) {
                retval[it->second] = f->second;
            }
        }
    }
}

int DatabaseNConfig::exec(
	const std::string &statement
) const
{
	return db->exec(statement);
}

int DatabaseNConfig::open()
{
	if (!db)
		return ERR_CODE_NO_DATABASE;
	return db->open(config->connectionString, config->login, config->password,
		config->db, config->port);
}

int DatabaseNConfig::close() const
{
	if (!db)
		return ERR_CODE_NO_DATABASE;
	return db->close();
}

void DatabaseByConfig::prepare(
    const ReceiverQueueValue &value
)
{
    DEVADDRINT a(value.addr);
    if (plugins)
        plugins->prepare(a.a, value.payload);
}

void DatabaseByConfig::prepare(
    uint32_t addr,
    const std::string &payload
)
{
    if (plugins)
        plugins->prepare(addr, payload);
}

//------------------- DatabaseByConfig -------------------

DatabaseByConfig::DatabaseByConfig
(
	const ConfigDatabasesIntf *aconfig,
    PayloadInsertPlugins *aPlugins
)
	: config(aconfig)
{
    plugins = aPlugins;
    if (plugins)
        plugins->dbByConfig = this;
}

DatabaseByConfig::~DatabaseByConfig()
{

}

DatabaseIntf* DatabaseByConfig::open
(
	const ConfigDatabase *dbc
) const
{
	if (dbc->type == "sqlite3")
#ifdef ENABLE_DB_SQLITE	
		return new DatabaseSQLite();
#else
	;		
#endif
	else
		if (dbc->type == "postgresql")
#ifdef ENABLE_DB_POSTGRES			
			return new DatabasePostgreSQL();
#else
	;		
#endif
		else
			if (dbc->type == "mysql")
#ifdef ENABLE_DB_MYSQL				
				return new DatabaseMySQL();
#else
	;		
#endif
			else
				if (dbc->type == "firebird")
#ifdef ENABLE_DB_FIREBIRD					
					return new DatabaseFirebird();
#else
	;		
#endif
#ifdef ENABLE_DB_JSON
    return new DatabaseJSON();
#else
    ;
#endif
    return NULL;
}

size_t DatabaseByConfig::count() const
{
	return config->dbs.size();
}

DatabaseIntf* DatabaseByConfig::getDb
(
	const ConfigDatabase **retConfig,
	int seqno
) const 
{
	if (seqno < 0 || seqno >= config->dbs.size())
		return NULL;
	if (retConfig)
		*retConfig = &config->dbs[seqno];
	return open(&config->dbs[seqno]);
}

DatabaseIntf* DatabaseByConfig::findDb
(
	const ConfigDatabase **retConfig,
	const std::string &name
) const
{
	const ConfigDatabase *cd = config->findByName(name);
	if (retConfig)
		*retConfig = cd;
	if (!cd)
		return nullptr;
	return open(cd);
}

DatabaseNConfig* DatabaseByConfig::get(
	int id
) const
{
	if (id < 0 || id >= config->dbs.size())
		return nullptr;
	return new DatabaseNConfig(&config->dbs[id], plugins);
}

DatabaseNConfig* DatabaseByConfig::find(
	const std::string &name
) const
{
	const ConfigDatabase *cd = config->findByName(name);
	if (!cd)
		return nullptr;
	return new DatabaseNConfig(cd, plugins);
}

/**
 * @param retval return dayavase identiofiiers
 * @return  count of databases
 */
size_t DatabaseByConfig::getIds(
	std::vector<int> &retval
)
{
	size_t r = config->dbs.size();
	retval.clear();
	for (std::vector<ConfigDatabase>::const_iterator it(config->dbs.begin()); it != config->dbs.end(); it++) {
		retval.push_back(it->id);
	}
	return r;
}
