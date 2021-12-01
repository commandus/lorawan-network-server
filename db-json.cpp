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
	const std::string &login,
	const std::string &password,
	const std::string &aAuth,
	int port
)
{
    url = aUrl;
    auth = aAuth;
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
    int r = postString(errMessage, url, auth, json);
    if (r >= 200 && r < 300)
        return LORA_OK;
    else
        return ERR_CODE_DB_EXEC;
}

int DatabaseJSON::select
(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	return 0;
}
