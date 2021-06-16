#ifndef ENABLE_DB_FIREBIRD
#error "No firebird support is on. Enable it by ./configure --enable-db-firebird=yes"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ibase.h>
#include <string>
#include <vector>
#include <iostream>

#include "errlist.h"

static int dbopen(
    isc_db_handle &retdb,
	const std::string &login,
	const std::string &password,
	const std::string &dbname
)
{
    ISC_STATUS_ARRAY status;
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
	if (isc_attach_database(status, 0, dbname.c_str(), &retdb, dpb - dpb_buffer, dpb_buffer)) {
		std::cerr << "open db: " << login.c_str() << ", " << password.c_str() << ", " << dbname.c_str() << std::endl;
		return 10;
	}
	return 0;
}

typedef struct vary {
	unsigned short vary_length;
	char vary_string[1];
} VARY;

int doSelect
(
    isc_db_handle &db,
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
    ISC_STATUS_ARRAY status;
    isc_tr_handle trans = 0;
    short dialect = 3;
    short max_columns = 5;

	if (!db)
		return ERR_CODE_DB_DATABASE_OPEN;

	if (isc_start_transaction(status, &trans, 1, &db, 0, NULL))
    {
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
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

    // Prepare the statement.
    // if (isc_dsql_prepare(status, &trans, &stmt, 0, "SELECT \"status\", \"status\"  FROM \"iridium_packet\"", dialect, sqlda)) // statement.c_str()
    if (isc_dsql_prepare(status, &trans, &stmt, 0, statement.c_str(), dialect, sqlda)) // statement.c_str()
    {
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	 // Describe the statement
    if (isc_dsql_describe(status, &stmt, 1, sqlda))
    {
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
			isc_dsql_free_statement(status, &stmt, DSQL_close);
			isc_rollback_transaction(status, &trans);
			free(sqlda);
			return ERR_CODE_DB_SELECT;
        }
    }

 	if (isc_dsql_execute(status, &trans, &stmt, 1, NULL))
	{
		isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
	}

	std::vector<std::string> t;
    short flags[32]; 
    
	for (int i = 0; i < sqlda->sqld; i++)
	{
		sqlda->sqlvar[i].sqllen = 32;
		t.push_back(std::string(sqlda->sqlvar[i].sqllen, '\0'));
		//flags.push_back(0);
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
			std::cerr << s << " len: " << s.size() << std::endl;
			line.push_back(s);
		}
		retval.push_back(line);
    }

    
    if (!(fetch_stat == 100L || fetch_stat == 0))
    {
    	isc_dsql_free_statement(status, &stmt, DSQL_close);
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	if (isc_dsql_free_statement(status, &stmt, DSQL_drop))
    {
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_SELECT;
    }

	if (isc_commit_transaction(status, &trans))
    {
        isc_rollback_transaction(status, &trans);
        free(sqlda);
		return ERR_CODE_DB_COMMIT_TRANSACTION;
    }

	free(sqlda);
    
	return 0;
}

int main(int argc, char** argv)
{
    isc_db_handle db = 0;
    if (dbopen(db, "sysdba", "masterkey", "irthermometer")) {
        exit (1);
    }
	
    std::vector<std::vector<std::string>> vals;

    doSelect(db, vals, "SELECT \"version\", \"version\"  FROM \"iridium_packet\"");
    doSelect(db, vals, "SELECT \"version\", \"version\"  FROM \"iridium_packet\"");
    doSelect(db, vals, "SELECT \"version\", \"version\"  FROM \"iridium_packet\"");
    doSelect(db, vals, "SELECT \"version\", \"version\"  FROM \"iridium_packet\"");
    doSelect(db, vals, "SELECT \"version\", \"version\"  FROM \"iridium_packet\"");
    doSelect(db, vals, "SELECT \"version\", \"version\"  FROM \"iridium_packet\"");
    doSelect(db, vals, "SELECT \"version\", \"version\"  FROM \"iridium_packet\"");

    for (std::vector<std::vector<std::string>>::const_iterator it(vals.begin()); it != vals.end(); it++)
    {
        for (std::vector<std::string>::const_iterator it2(it->begin()); it2 != it->end(); it2++) {
            std::cout << *it2 << "|";
        }
        std::cout << std::endl;
    }

    return 0;
}            
