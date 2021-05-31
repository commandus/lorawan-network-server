#ifndef DB_INTF_H
#define DB_INTF_H	1

#include <string>
#include <vector>

/**
 */
class DatabaseIntf
{
public:
	std::string errmsg;
	virtual int open(
		const std::string &connection,
		const std::string &login,
		const std::string &password
	) = 0;
	virtual int close() = 0;

	virtual int exec(
		const std::string &statement
	) = 0;

	virtual int select(
		std::vector<std::vector<std::string>> &retval,
		const std::string &statement
	) = 0;
};

#endif
