#include "db-pg.h"

#include "errlist.h"

DatabasePostgreSQL::DatabasePostgreSQL()
	: conn(NULL)
{
	errmsg = "";
	type = "postgresql";
}

DatabasePostgreSQL::~DatabasePostgreSQL()
{
	if (conn) {
		close();
	}
}

/**
 * @brief Connset to PostgreSQL
 * @param login not used
 * @param password not used
 */ 
int DatabasePostgreSQL::open(
	const std::string &connection,
	const std::string &login,
	const std::string &password,
	const std::string &db,
	int port
)
{
	conn = PQconnectdb(connection.c_str());
	ConnStatusType r = PQstatus(conn);
	if (r != CONNECTION_OK)
	{
		conn = NULL;
		errmsg = std::string(PQerrorMessage(conn));
	}
	return r;
}

int DatabasePostgreSQL::close()
{
	PQfinish(conn);
	conn = NULL;
	return 0;
}

int DatabasePostgreSQL::exec(
	const std::string &statement
)
{
	PGresult *r = PQexec(conn, statement.c_str());
	ExecStatusType status = PQresultStatus(r);
	int rc;
	if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK ) {
		rc = 0;
	} else {
		errmsg = std::string(PQerrorMessage(conn));
		rc = ERR_CODE_DB_SELECT;
	}
	PQclear(r);
	return rc;
}

int DatabasePostgreSQL::select(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	PGresult *r = PQexec(conn, statement.c_str());
	ExecStatusType status = PQresultStatus(r);
	int rc;
	if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK ) {
		rc = 0;
	} else {
		rc = ERR_CODE_DB_SELECT;
		errmsg = std::string(PQerrorMessage(conn));
	}

	for (int row = 0; row < PQntuples(r); row++)
    {
		std::vector<std::string> line;
		for (int fld = 0; fld < PQnfields(r); fld++)
	    {
			std::string v(PQgetvalue(r, row, fld));
			line.push_back(v);
		}
		retval.push_back(line);
	}

	PQclear(r);
	return rc;
}

//
// cursor
// 
int DatabasePostgreSQL::cursorOpen(
	void **retStmt,
	std::string &statement
)
{
	PGresult *r = PQexec(conn, statement.c_str());
	ExecStatusType status = PQresultStatus(r);
	int rc;
	if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK ) {
		rc = 0;
	} else {
		rc = ERR_CODE_DB_SELECT;
		errmsg = std::string(PQerrorMessage(conn));
	}
	*retStmt = r;
	return rc;
}

int DatabasePostgreSQL::cursorBindText(
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

int DatabasePostgreSQL::cursorColumnCount(
	void *stmt
)
{
	return PQnfields((const PGresult *) stmt);
}

std::string DatabasePostgreSQL::cursorColumnName(
	void *stmt,
	int column0
)
{
	std::string r;
	return r;
}

std::string DatabasePostgreSQL::cursorColumnText(
	void *stmt,
	int column0
)
{
	std::string r;
	return r;
}

DB_FIELD_TYPE DatabasePostgreSQL::cursorColumnType(
	void *stmt,
	int column0
)
{
	return (DB_FIELD_TYPE) 0;
}

 bool DatabasePostgreSQL::cursorNext(
	void *stmt
)
{
	return false;
}

int DatabasePostgreSQL::cursorClose(
	void *stmt
)
{
	PQclear((PGresult *) stmt);
	return 0;
}
