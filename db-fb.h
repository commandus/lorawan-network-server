#ifndef DB_FB_H
#define DB_FB_H	1

#include "db-intf.h"

#include <ibase.h>

/**
 */
class DatabaseFirebird : public DatabaseIntf
{
protected:
	signed short max_columns;
	std::string errstr();
public:
	short dialect;
	isc_db_handle db;			// database handle
	isc_tr_handle trans;		// transaction handle
	ISC_STATUS_ARRAY status;	// status vector

	DatabaseFirebird();
	~DatabaseFirebird();

	int open(
		const std::string &connection,
		const std::string &login,
		const std::string &password,
		const std::string &db,
		int port
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
