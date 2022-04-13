#include <sstream>
#include <iostream>

#include "db-json.h"
#include "utilcurl.h"
#include "errlist.h"

DatabaseJSON::DatabaseJSON()
{
	type = "json";
}

DatabaseJSON::~DatabaseJSON()
{
	close();
}

int DatabaseJSON::open(
	const std::string &aUrl,
	const std::string &aLogin,
	const std::string &aPassword,
	const std::string &aAuthorizationUrl,
	int port
)
{
    url = aUrl;
    login = aLogin;
    password = aPassword;
    authorizationUrl = aAuthorizationUrl;
    auth = "";
	return 0;
}

int DatabaseJSON::close()
{
	return 0;
}

int DatabaseJSON::exec(
	const std::string &json
)
{
    if (json.empty())
        return ERR_CODE_PARAM_INVALID;
    int r = postString(errMessage, url, json, auth);
    if (r >= 200 && r < 300)
        return LORA_OK;
    else {
        // re-login
        std::stringstream ss;
        ss << "login=" << login << "&"
                << "password=" << password;
        r = postString(errMessage, authorizationUrl, ss.str(), "", "application/x-www-form-urlencoded");
        if (r >= 200 && r < 300) {
            // re-post data
            r = postString(errMessage, url, json, auth);
            if (r >= 200 && r < 300)
                return LORA_OK;
        }
        return ERR_CODE_DB_EXEC;
    }
}

int DatabaseJSON::select
(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	return 0;
}

//
// cursor
// 
int DatabaseJSON::cursorOpen(
	void **retStmt,
	std::string &statement
)
{
	errmsg = ERR_DB_SELECT;
	*retStmt = NULL;
	return ERR_CODE_DB_SELECT;
}

int DatabaseJSON::cursorBindText(
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

int DatabaseJSON::cursorColumnCount(
	void *stmt
)
{
	return 0;
}

std::string DatabaseJSON::cursorColumnName(
	void *stmt,
	int column0
)
{
	std::string r;
	return r;
}

std::string DatabaseJSON::cursorColumnText(
	void *stmt,
	int column0
)
{
	std::string r;
	return r;
}

DB_FIELD_TYPE DatabaseJSON::cursorColumnType(
	void *stmt,
	int column0
)
{
	return (DB_FIELD_TYPE) 0;
}

 bool DatabaseJSON::cursorNext(
	void *stmt
)
{
	return false;
}

int DatabaseJSON::cursorClose(
	void *stmt
)
{
	return 0;
}
