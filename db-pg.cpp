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
