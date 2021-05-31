#ifndef DB_SQLITE_H
#define DB_SQLITE_H	1

#include "db-intf.h"

#include <postgresql/libpq-fe.h>

/**
 */
class DatabasePostgreSQL : public DatabaseIntf
{
public:
	PGconn *conn;

	DatabasePostgreSQL();
	~DatabasePostgreSQL();

	/**
	 * @brief Connset to PostgreSQL
	 * @param login not used
	 * @param password not used
	 */ 
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
