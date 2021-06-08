#include <sstream>
#include <iostream>

#include "db-fb.h"
#include "errlist.h"

DatabaseFirebird::DatabaseFirebird()
	: max_columns(32), db(0), trans(0)

{
	errmsg = "";
	type = "firebird";
}

DatabaseFirebird::~DatabaseFirebird()
{
	if (db) {
		close();
	}
}

std::string DatabaseFirebird::errstr() 
{
	std::stringstream ss;
	if (status[0] == 1 && status[1] > 0) {
		char err_buf[256];
		long SQLCODE = isc_sqlcode(status);
		isc_sql_interprete(SQLCODE, err_buf, sizeof(err_buf));
		ss << SQLCODE << ", " << err_buf;
	};
	return ss.str();
}

int DatabaseFirebird::open(
	const std::string &connection,
	const std::string &login,
	const std::string &password,
	const std::string &dbname,
	int port
)
{
	// Construct the database parameter buffer
	// @see http://docwiki.embarcadero.com/InterBase/2020/en/Isc_attach_database()
	char dpb_buffer[256];
	char *dpb = dpb_buffer;
	*dpb++ = isc_dpb_version1;
	*dpb++ = isc_dpb_user_name;
	*dpb++ = login.size();
	for (const char *p = login.c_str(); *p;)
		*dpb++ = *p++;
	*dpb++ = isc_dpb_password;
	*dpb++ = password.size();
	for (const char *p = password.c_str(); *p;)
		*dpb++ = *p++;
	if (isc_attach_database(status, 0, dbname.c_str(), &db, dpb - dpb_buffer, dpb_buffer)) {
		errmsg = errstr();
		std::cerr << "open db: " << connection.c_str() << ", " << login.c_str() << ", " << password.c_str() << ", " << dbname.c_str() << std::endl;
		std::cerr << "Error: " << errmsg << std::endl;
		return ERR_CODE_DB_DATABASE_OPEN;
	}
	return 0;
}

int DatabaseFirebird::close()
{
	if (!db)
		return 0;
	if (isc_detach_database(status, &db))
    {
		errmsg = errstr();
		return ERR_CODE_DB_DATABASE_CLOSE;
    }
	db = 0;
	return 0;
}

int DatabaseFirebird::exec(
	const std::string &statement
)
{
	if (!db)
		return ERR_CODE_DB_DATABASE_OPEN;
	if (isc_start_transaction(status, &trans, 1, &db, 0, NULL))
    {
        errmsg = errstr();
		return ERR_CODE_DB_START_TRANSACTION;
    }

 	if (isc_dsql_execute_immediate(status, &db, &trans, 0, statement.c_str(), 1, NULL))
	{
    	errmsg = errstr();
		return ERR_CODE_DB_EXEC;
	}

	if (isc_commit_transaction(status, &trans))
    {
        errmsg = errstr();
		return ERR_CODE_DB_COMMIT_TRANSACTION;
    }
	return 0;
}

int DatabaseFirebird::select
(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	if (!db)
		return ERR_CODE_DB_DATABASE_OPEN;
	if (isc_start_transaction(status, &trans, 1, &db, 0, NULL))
    {
        errmsg = errstr();
		return ERR_CODE_DB_START_TRANSACTION;
    }

	XSQLDA *sqlda;
	sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(3));
    sqlda->sqln = max_columns;
    sqlda->sqld = max_columns;
    sqlda->version = 1;

	isc_stmt_handle stmt = 0;
	// Allocate a statement.
    if (isc_dsql_allocate_statement(status, &db, &stmt))
    {
    	errmsg = errstr();
		return ERR_CODE_DB_SELECT;
    }
    
    /* Prepare the statement. */
    if (isc_dsql_prepare(status, &trans, &stmt, 0, statement.c_str(), 1, sqlda))
    {
    	errmsg = errstr();
		return ERR_CODE_DB_SELECT;
    }

	short flag;
	std::vector<std::string> t;
	for (int i = 0; i < max_columns; i++)
	{
		t.push_back(std::string(16, '\0'));
		sqlda->sqlvar[0].sqldata = (char *) t[i].c_str();
		sqlda->sqlvar[0].sqltype = SQL_TEXT + 1;
		sqlda->sqlvar[0].sqlind = &flag;
	}

 	if (isc_dsql_execute(status, &trans, &stmt, 1, NULL))
	{
    	errmsg = errstr();
		return ERR_CODE_DB_SELECT;
	}

	long fetch_stat;
	int num_fields = 1;
 	while ((fetch_stat = isc_dsql_fetch(status, &stmt, 1, sqlda)) == 0)
    {
		
		std::cerr << t[0] << std::endl;

		std::vector<std::string> line;
    	for(int i = 0; i < num_fields; i++) {
			std::string s(t[i]);
			line.push_back(s);
		}
		retval.push_back(line);
    }

    if (fetch_stat != 100L)
    {
    	errmsg = errstr();
		return ERR_CODE_DB_SELECT;
    }

	if (isc_commit_transaction(status, &trans))
    {
        errmsg = errstr();
		return ERR_CODE_DB_COMMIT_TRANSACTION;
    }

	return 0;
}
