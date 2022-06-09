#ifndef DB_INTF_H
#define DB_INTF_H	1

#include <string>
#include <vector>

typedef enum {
	DBT_INTEGER = 1,
	DBT_FLOAT = 2,
	DBT_BLOB = 4,
	DBT_NULL = 5,
	DBT_TEXT = 3
} DB_FIELD_TYPE;

/**
 */
class DatabaseIntf
{
public:
	std::string type;
	std::string errmsg;
	// Example: config->lasterr = sqlite3_open_v2(config->connectionString, (sqlite3 **) &config->dbSqlite3, SQLITE_OPEN_READWRITE, NULL);	// SQLITE_OPEN_READWRITE
	virtual int open(
		const std::string &connection,
		const std::string &login,
		const std::string &password,
		const std::string &db,
		int port
	) = 0;
	virtual int close() = 0;

	virtual int exec(
		const std::string &statement
	) = 0;

	virtual int select(
		std::vector<std::vector<std::string> > &retval,
		const std::string &statement
	) = 0;

	// cursor
	virtual int cursorOpen(
		void **retStmt,
		std::string &statement
	) = 0;

	virtual int cursorBindText(
		void *stmt,
		int position1,
		const std::string &value
	) = 0;

	virtual int cursorColumnCount(
		void *stmt
	) = 0;

	virtual std::string cursorColumnName(
		void *stmt,
		int column0
	) = 0;

	virtual std::string cursorColumnText(
		void *stmt,
		int column0
	) = 0;

	virtual DB_FIELD_TYPE cursorColumnType(
		void *stmt,
		int column0
	) = 0;

	virtual bool cursorNext(
		void *stmt
	) = 0;

	virtual int cursorClose(
		void *stmt
	) = 0;

};

#endif
