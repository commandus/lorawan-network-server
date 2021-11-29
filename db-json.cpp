#include <sstream>
#include <iostream>

#include "db-json.h"
#include "errlist.h"

DatabaseJSON::DatabaseJSON()
{
	type = "json";
}

DatabaseJSON::~DatabaseJSON()
{
	if (db) {
		close();
	}
}

int DatabaseJSON::open(
	const std::string &connection,
	const std::string &login,
	const std::string &password,
	const std::string &dbname,
	int port
)
{
	return 0;
}

int DatabaseJSON::close()
{
	return 0;
}

int DatabaseJSON::exec(
	const std::string &statement
)
{
	return 0;
}

int DatabaseJSON::select
(
	std::vector<std::vector<std::string>> &retval,
	const std::string &statement
)
{
	return 0;
}
