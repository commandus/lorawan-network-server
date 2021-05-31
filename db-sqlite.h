#ifndef DB_SQLITE_H
#define DB_SQLITE_H	1

#include "db-intf.h"

#include <sqlite3.h>

/**
 */
class DatabaseSQLite : public DatabaseIntf
{
public:
	sqlite3 *db;

	DatabaseSQLite();
	~DatabaseSQLite();
	
	int open(
		const std::string &connection,
		const std::string &login,
		const std::string &password
	);
	int close();

	int exec(
		const std::string &statement
	);

	int select(
		std::vector<std::vector<std::string>> &retval,
		const std::string &statement
	);
};

#endif
