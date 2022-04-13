#ifndef DB_PG_H
#define DB_PG_H	1

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
