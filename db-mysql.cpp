#include "db-mysql.h"
#include "errlist.h"

DatabaseMySQL::DatabaseMySQL()
	: db(NULL)
{
	errmsg = "";
	type = "mysql";
}

DatabaseMySQL::~DatabaseMySQL()
{
	if (db) {
		close();
	}
}

int DatabaseMySQL::open(
	const std::string &connection,
	const std::string &login,
	const std::string &password,
	const std::string &dbname,
	int port
)
{
	db = mysql_init(NULL);
	// std::cerr << "open db: " << connection.c_str() << ", " << login.c_str() << ", " << password.c_str() << ", " << dbname.c_str() << std::endl;
	if (mysql_real_connect(db, connection.c_str(), login.c_str(), password.c_str(), dbname.c_str(), 0, NULL, 0) == NULL) {
		errmsg = std::string(mysql_error(db));
		return ERR_CODE_DB_DATABASE_OPEN;
	}
	return 0;
}

int DatabaseMySQL::close()
{
	if (!db)
		return 0;
	mysql_close(db);
	db = NULL;
	return 0;
}

int DatabaseMySQL::exec(
	const std::string &statement
)
{
	if (!db)
		return ERR_CODE_DB_DATABASE_OPEN;
	int r = mysql_query(db, statement.c_str());
  	if (r)
		errmsg = std::string(mysql_error(db));
	return r;
}

int DatabaseMySQL::select
(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	if (!db)
		return ERR_CODE_DB_DATABASE_OPEN;
	int r = mysql_query(db, statement.c_str());
  	if (r) {
		errmsg = std::string(mysql_error(db));
		return r;
	}
	
	MYSQL_RES *result = mysql_store_result(db);
	if (result == NULL) {
		errmsg = std::string(mysql_error(db));
		return ERR_CODE_DB_SELECT;
	}

  	int num_fields = mysql_num_fields(result);
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		std::vector<std::string> line;
    	for(int i = 0; i < num_fields; i++) {
			line.push_back(row[i] ? row[i] : "");
		}
		retval.push_back(line);
	}
	return 0;
}

//
// cursor
// 
int DatabaseMySQL::cursorOpen(
	void **retStmt,
	std::string &statement
)
{
	int rc = ERR_CODE_DB_SELECT;
	*retStmt = NULL;
	return rc;
}

int DatabaseMySQL::cursorBindText(
	void *stmt,
	int position1,
	const std::string &value
)
{
	// int r = sqlite3_bind_text((sqlite3_stmt *) stmt, position1, value.c_str(), -1, SQLITE_STATIC);
	long int v = strtol(value.c_str(), NULL, 10);
	int r = ERR_CODE_DB_EXEC;
	if (r)
		errmsg = ERR_DB_EXEC;
	return r;
}

int DatabaseMySQL::cursorColumnCount(
	void *stmt
)
{
	return 0;
}

std::string DatabaseMySQL::cursorColumnName(
	void *stmt,
	int column0
)
{
	std::string r;
	return r;
}

std::string DatabaseMySQL::cursorColumnText(
	void *stmt,
	int column0
)
{
	std::string r;
	return r;
}

DB_FIELD_TYPE DatabaseMySQL::cursorColumnType(
	void *stmt,
	int column0
)
{
	return (DB_FIELD_TYPE) 0;
}

 bool DatabaseMySQL::cursorNext(
	void *stmt
)
{
	return false;
}

int DatabaseMySQL::cursorClose(
	void *stmt
)
{
	return 0;
}
