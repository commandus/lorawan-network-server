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

	virtual int open(
		const std::string &connection,
		const std::string &login,
		const std::string &password,
		const std::string &db,
		int port
	) override;
    virtual int close() override;

    virtual int exec(
		const std::string &statement
	) override;

    virtual int select(
		std::vector<std::vector<std::string>> &retval,
		const std::string &statement
	) override;
};

#endif
