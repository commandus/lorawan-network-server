#include "db-sqlite.h"

DatabaseSQLite::DatabaseSQLite()
	: db(NULL)
{
	errmsg = "";
	type = "sqlite3";
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
	const std::string &password
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
	char *zErrMsg = 0;
	int r = sqlite3_exec(db, statement.c_str(), sqlite3Callback, NULL, &zErrMsg);
  	if (r)
		errmsg = std::string(zErrMsg);
	return r;
}

int DatabaseSQLite::select
(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	char *zErrMsg = 0;
	int r = sqlite3_exec(db, statement.c_str(), sqlite3Callback, &retval, &zErrMsg);
  	if (r)
		errmsg = std::string(zErrMsg);
	return r;
}
