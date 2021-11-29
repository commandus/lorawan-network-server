#ifndef DB_MYSQL_H
#define DB_MYSQL_H	1

#include "db-intf.h"

#include <mysql/mysql.h>

/**
 */
class DatabaseMySQL : public DatabaseIntf
{
public:
	MYSQL *db;

	DatabaseMySQL();
	~DatabaseMySQL();

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
