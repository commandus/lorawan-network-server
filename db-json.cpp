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
