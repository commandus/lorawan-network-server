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

		//
	// cursor
	//
	virtual int cursorOpen(
		void **retStmt,
		std::string &statement
	) override;

	virtual int cursorBindText(
		void *stmt,
		int position1,
		const std::string &value
	) override;

	virtual int cursorColumnCount(
		void *stmt
	) override;

	virtual std::string cursorColumnName(
		void *stmt,
		int column0
	) override;

	virtual std::string cursorColumnText(
		void *stmt,
		int column0
	) override;

	virtual DB_FIELD_TYPE cursorColumnType(
		void *stmt,
		int column0
	) override;

	virtual bool cursorNext(
		void *stmt
	) override;

	virtual int cursorClose(
		void *stmt
	) override;

};

#endif
