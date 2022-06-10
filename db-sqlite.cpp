#include "db-sqlite.h"

DatabaseSQLite::DatabaseSQLite()
	: dbSqlite3(nullptr)
{
	errmsg = "";
	type = "sqlite3";
}

DatabaseSQLite::DatabaseSQLite(
	const DatabaseSQLite &value
) : dbSqlite3(value.dbSqlite3)
{
    errmsg = value.errmsg;
	type = value.type;
}

DatabaseSQLite::~DatabaseSQLite()
{
	if (dbSqlite3) {
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
    // remember
    this->connection = connection;

    int r = sqlite3_open(connection.c_str(), &dbSqlite3);
	if (r)
        dbSqlite3 = nullptr;
	return r;
}

int DatabaseSQLite::reopen()
{
    if (dbSqlite3)
        close();
    int r = sqlite3_open(this->connection.c_str(), &dbSqlite3);
    if (r)
        dbSqlite3 = nullptr;
}

int DatabaseSQLite::close()
{
	int r = sqlite3_close(dbSqlite3);
	if (!r)
        dbSqlite3 = nullptr;
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
    int r = sqlite3_exec(dbSqlite3, statement.c_str(), sqlite3Callback, nullptr, &zErrMsg);
    if (r != SQLITE_OK) {
        if (zErrMsg) {
            reopen();
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
    sqlite3_stmt *stmt;
    int r = sqlite3_prepare_v2(dbSqlite3, statement.c_str(), -1, &stmt, nullptr);
    if (r) {
        errmsg = sqlite3_errstr(r);
        reopen();
        return r;
    }
    int columns = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> line;
        for (int i = 0; i < columns; i++)
        {
            line.push_back(std::string((const char *) sqlite3_column_text(stmt, i)));
        }
        retval.push_back(line);
    }
	return 0;
}

//
// cursor
// 
int DatabaseSQLite::cursorOpen(
	void **retStmt,
	std::string &statement
)
{
    sqlite3_stmt *result;
	int r = sqlite3_prepare_v2(dbSqlite3, statement.c_str(), -1, &result, NULL);
	if (r) {
        errmsg = sqlite3_errstr(r);
        reopen();
        return r;
    }
    *retStmt = result;
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
