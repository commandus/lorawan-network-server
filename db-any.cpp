#include <iostream>

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

#include "errlist.h"

#include "pkt2/str-pkt2.h"

DatabaseNConfig::DatabaseNConfig
(
	const ConfigDatabase *aconfig
)
{
	config = aconfig;
	// unknown database type
	db = NULL;
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
}

DatabaseNConfig::~DatabaseNConfig()
{
	std::cerr << "~DatabaseNConfig" << std::endl;
	if (db) {
		db->close();
		delete db;
	}
}

std::string DatabaseNConfig::tableName(
	void *env,
	const std::string &message
)
{
	std::map<std::string, std::string>::const_iterator it(config->tableAliases.find(message));
	if (it == config->tableAliases.end())
		return "";
	else
		return it->second;
}

std::string DatabaseNConfig::createClause
(
	void *env,
	const std::string &message
)
{
	return createTableSQLClause(env, message, OUTPUT_FORMAT_SQL, config->getDialect(), 
		&config->tableAliases, &config->fieldAliases);
}

std::string DatabaseNConfig::insertClause(
	void *env,
	const std::string &message,
	int inputFormat,
	const std::string &data
)
{
	return parsePacket(env, inputFormat, OUTPUT_FORMAT_SQL, config->getDialect(), data, message,
		&config->tableAliases, &config->fieldAliases);
}

int DatabaseNConfig::createTable(void *env, const std::string &message)
{
	std::string clause = createClause(env, message);
	if (!db)
		return ERR_CODE_NO_DATABASE;
	return db->exec(clause);
}

int DatabaseNConfig::insert(
	void *env,
	const std::string &message,
	int inputFormat,
	const std::string &data
)
{
	std::string clause = insertClause(env, message, inputFormat, data);
	return db->exec(clause);
}

int DatabaseNConfig::select(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	return db->select(retval, statement);
}

int DatabaseNConfig::exec(
	const std::string &statement
)
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

int DatabaseNConfig::close()
{
	if (!db)
		return ERR_CODE_NO_DATABASE;
	return db->close();
}

//------------------- DatabaseByConfig -------------------

DatabaseByConfig::DatabaseByConfig
(
	const ConfigDatabases *aconfig
)
	: config(aconfig)
{

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
	int id
) const 
{
	if (id < 0 || id >= config->dbs.size())
		return NULL;
	if (retConfig)
		*retConfig = &config->dbs[id];
	return open(&config->dbs[id]);
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
		return NULL;
	return open(cd);
}

DatabaseNConfig* DatabaseByConfig::get(
	int id
) const
{
	if (id < 0 || id >= config->dbs.size())
		return NULL;
	return new DatabaseNConfig(&config->dbs[id]);
}

DatabaseNConfig* DatabaseByConfig::find(
	const std::string &name
) const
{
	const ConfigDatabase *cd = config->findByName(name);
	if (!cd)
		return NULL;
	return new DatabaseNConfig(cd);
}
