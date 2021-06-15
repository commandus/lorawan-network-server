#include <sstream>
#include <iostream>

#include "db-fb.h"
#include "errlist.h"

DatabaseFirebird::DatabaseFirebird()
	: dialect(3), max_columns(32), db(0), trans(0)

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
		char err_buf[512];
		long SQLCODE = isc_sqlcode(status);
		isc_sql_interprete(SQLCODE, err_buf, sizeof(err_buf));
		ss << SQLCODE << ", " << err_buf;
		
		// @see http://docwiki.embarcadero.com/InterBase/2020/en/Isc_interprete()
		// deprecated
		long *pvector = status;
		isc_interprete(err_buf, &pvector);
		ss << err_buf << std::endl;
		while (isc_interprete(err_buf, &pvector))
		{
			isc_interprete(err_buf, &pvector);
			ss << err_buf << std::endl;
		}
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

 	if (isc_dsql_execute_immediate(status, &db, &trans, 0, statement.c_str(), dialect, NULL))
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

typedef struct vary {
	unsigned short vary_length;
	char vary_string[1];
} VARY;

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

	XSQLDA *sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(max_columns));
    sqlda->sqln = max_columns;
    sqlda->sqld = max_columns;
    sqlda->version = 1;

	isc_stmt_handle stmt = 0;
	// Allocate a statement.
    if (isc_dsql_allocate_statement(status, &db, &stmt))
    {
    	errmsg = errstr();
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

    // Prepare the statement.
    if (isc_dsql_prepare(status, &trans, &stmt, 0, "SELECT \"status\" FROM \"iridium_packet\"", dialect, sqlda)) // statement.c_str()
    {
    	errmsg = errstr();
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	 // Describe the statement
    if (isc_dsql_describe(status, &stmt, 1, sqlda))
    {
    	errmsg = errstr();
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	// Reallocate SQLDA if necessary.
    if (sqlda->sqln < sqlda->sqld)
    {
		free(sqlda);
        sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(sqlda->sqld));
        sqlda->sqln = sqlda->sqld;
        sqlda->version = 1;
    
        // Re-describe the statement.
        if (isc_dsql_describe(status, &stmt, 1, sqlda))
        {
    		errmsg = errstr();
			isc_dsql_free_statement(status, &stmt, DSQL_close);
			isc_rollback_transaction(status, &trans);
			free(sqlda);
			return ERR_CODE_DB_SELECT;
        }
    }

 	if (isc_dsql_execute(status, &trans, &stmt, 1, NULL))
	{
    	errmsg = errstr();
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
	}


	std::vector<std::string> t;
	std::vector<short> flags;

	for (int i = 0; i < sqlda->sqld; i++)
	{
		if ((sqlda->sqlvar[i].sqltype == SQL_TEXT) || (sqlda->sqlvar[i].sqltype == SQL_VARYING)) {
		} else {
			sqlda->sqlvar[i].sqllen = 32 - sizeof(unsigned short);
		}

		t.push_back(std::string(sqlda->sqlvar[i].sqllen, '\0'));
		flags.push_back(0);
		sqlda->sqlvar[i].sqldata = (char *) t[i].c_str();
		sqlda->sqlvar[i].sqltype = SQL_VARYING + 1;
		sqlda->sqlvar[i].sqlind = &flags[i];
	}

	ISC_STATUS fetch_stat;
	while (fetch_stat = isc_dsql_fetch(status, &stmt, 1, sqlda) == 0)
    {
		std::vector<std::string> line;
    	for(int i = 0; i < sqlda->sqld; i++)
		{
			VARY *v = (VARY *) sqlda->sqlvar[i].sqldata;
			std::string s(v->vary_string, v->vary_length);	//
			// std::cerr << s << " len: " << s.size() << std::endl;
			line.push_back(s);
		}
		retval.push_back(line);
    }

    if (!(fetch_stat == 100L || fetch_stat == 0))
    {
    	errmsg = errstr();
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	if (isc_dsql_free_statement(status, &stmt, DSQL_drop))
    {
        errmsg = errstr();
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	if (isc_commit_transaction(status, &trans))
    {
        errmsg = errstr();
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_COMMIT_TRANSACTION;
    }

	free(sqlda);

	return 0;
}
