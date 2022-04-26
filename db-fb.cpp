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

// @see http://docwiki.embarcadero.com/InterBase/2020/en/Isc_interprete()
// isc_interprete() deprecated

#define isc_interprete(buf, status) fb_interpret(buf, sizeof(buf), status)

std::string DatabaseFirebird::errstr() 
{
	std::stringstream ss;
	if (status[0] == 1 && status[1] > 0) {
		char err_buf[512];
		long SQLCODE = isc_sqlcode(status);
		isc_sql_interprete(SQLCODE, err_buf, sizeof(err_buf));
		ss << SQLCODE << ", " << err_buf;

        const ISC_STATUS *pvector = status;
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
    if (isc_dsql_prepare(status, &trans, &stmt, 0, statement.c_str(), dialect, sqlda))
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
	short *flags = new short[sqlda->sqld];

	for (int i = 0; i < sqlda->sqld; i++)
	{
		if ((sqlda->sqlvar[i].sqltype == SQL_TEXT) || (sqlda->sqlvar[i].sqltype == SQL_VARYING)) {
		} else {
			sqlda->sqlvar[i].sqllen = 32 - sizeof(unsigned short);
		}
		t.push_back(std::string(sqlda->sqlvar[i].sqllen, '\0'));
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
			line.push_back(std::string(v->vary_string, v->vary_length));
		}
		retval.push_back(line);
    }

    if (!(fetch_stat == 100L || fetch_stat == 0))
    {
    	errmsg = errstr();
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
		free(flags);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	if (isc_dsql_free_statement(status, &stmt, DSQL_drop))
    {
        errmsg = errstr();
        isc_rollback_transaction(status, &trans);
		free(flags);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	if (isc_commit_transaction(status, &trans))
    {
        errmsg = errstr();
        isc_rollback_transaction(status, &trans);
		free(flags);
        free(sqlda);
		return ERR_CODE_DB_COMMIT_TRANSACTION;
    }

	free(flags);
	free(sqlda);

	return 0;
}

//
// cursor
// 
int DatabaseFirebird::cursorOpen(
	void **retStmt,
	std::string &statement
)
{
	errmsg = ERR_DB_SELECT;
	*retStmt = NULL;
	return ERR_CODE_DB_SELECT;
}

int DatabaseFirebird::cursorBindText(
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

int DatabaseFirebird::cursorColumnCount(
	void *stmt
)
{
	return 0;
}

std::string DatabaseFirebird::cursorColumnName(
	void *stmt,
	int column0
)
{
	std::string r;
	return r;
}

std::string DatabaseFirebird::cursorColumnText(
	void *stmt,
	int column0
)
{
	std::string r;
	return r;
}

DB_FIELD_TYPE DatabaseFirebird::cursorColumnType(
	void *stmt,
	int column0
)
{
	return (DB_FIELD_TYPE) 0;
}

 bool DatabaseFirebird::cursorNext(
	void *stmt
)
{
	return false;
}

int DatabaseFirebird::cursorClose(
	void *stmt
)
{
	return 0;
}
