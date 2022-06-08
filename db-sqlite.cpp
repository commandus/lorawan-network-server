#include "db-sqlite.h"

DatabaseSQLite::DatabaseSQLite()
	: db(NULL)
{
	errmsg = "";
	type = "sqlite3";
}

DatabaseSQLite::DatabaseSQLite(
	const DatabaseSQLite &value
) : db(value.db)
{
	errmsg = value.errmsg;
	type = value.type;
}

DatabaseSQLite::~DatabaseSQLite()
{
	if (db) {
		close();
	}
}

int DatabaseSQLite::open(
	const std::string &connection,
	const std::string &login,
	const std::string &password,
	const std::string &dbname,
	int port
)
{
	int r = sqlite3_open(connection.c_str(), &db);
	if (r)
		db = NULL;
	return r;
}

int DatabaseSQLite::close()
{
	int r = sqlite3_close(db);
	if (!r)
		db = NULL;
	return r;
}

static int sqlite3Callback(
	void *env,
	int columns,
	char **value,
	char **column
)
{
	if (!env)
		return 0;
	std::vector<std::vector<std::string>> *retval = (std::vector<std::vector<std::string>> *) env;

	std::vector<std::string> line;
	for (int i = 0; i < columns; i++)
	{
		// printf("%s = %s\n", column[i], value[i] ? value[i] : "NULL");
		line.push_back(value[i] ? value[i] : "");
	}
	retval->push_back(line);
	return 0;
}

int DatabaseSQLite::exec(
	const std::string &statement
)
{
	char *zErrMsg = nullptr;
	int r = sqlite3_exec(db, statement.c_str(), sqlite3Callback, NULL, &zErrMsg);
  	if (r != SQLITE_OK) {
          if (zErrMsg) {
              errmsg = std::string(zErrMsg);
              sqlite3_free(zErrMsg);
          }
    }
	return r;
}

int DatabaseSQLite::select
(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	char *zErrMsg = nullptr;
	int r = sqlite3_exec(db, statement.c_str(), sqlite3Callback, &retval, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            errmsg = std::string(zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }
	return r;
}

//
// cursor
// 
int DatabaseSQLite::cursorOpen(
	void **retStmt,
	std::string &statement
)
{
	int r = sqlite3_prepare_v2(db, statement.c_str(), -1, (sqlite3_stmt **) retStmt, NULL);
	if (r)
		errmsg = sqlite3_errstr(r);
	return r;
}

int DatabaseSQLite::cursorBindText(
	void *stmt,
	int position1,
	const std::string &value
)
{
	// int r = sqlite3_bind_text((sqlite3_stmt *) stmt, position1, value.c_str(), -1, SQLITE_STATIC);
	long int v = strtol(value.c_str(), NULL, 10);
	int r = sqlite3_bind_int((sqlite3_stmt *) stmt, position1, v);
	if (r)
		errmsg = sqlite3_errstr(r);
	return r;
}

int DatabaseSQLite::cursorColumnCount(
	void *stmt
)
{
	return sqlite3_column_count((sqlite3_stmt *) stmt);
}

std::string DatabaseSQLite::cursorColumnName(
	void *stmt,
	int column0
)
{
	std::string r(sqlite3_column_name((sqlite3_stmt *) stmt, column0));
	return r;
}

std::string DatabaseSQLite::cursorColumnText(
	void *stmt,
	int column0
)
{
	std::string r((const char *) sqlite3_column_text((sqlite3_stmt *) stmt, column0));
	return r;
}

DB_FIELD_TYPE DatabaseSQLite::cursorColumnType(
	void *stmt,
	int column0
)
{
	return (DB_FIELD_TYPE) sqlite3_column_type((sqlite3_stmt *) stmt, column0);
}

 bool DatabaseSQLite::cursorNext(
	void *stmt
)
{
	int r = sqlite3_step((sqlite3_stmt *) stmt);
	if (r != SQLITE_ROW)
		errmsg = sqlite3_errstr(r);
	return (r == SQLITE_ROW);
}

int DatabaseSQLite::cursorClose(
	void *stmt
)
{
	int r = sqlite3_finalize((sqlite3_stmt *) stmt);
	if (r)
		errmsg = sqlite3_errstr(r);
	return r;
}
