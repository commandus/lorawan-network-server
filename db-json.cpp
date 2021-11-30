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
    return postString(errMessage, url, auth, json);
}

int DatabaseJSON::select
(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	return 0;
}
