#ifndef DB_JSON_H
#define DB_JSON_H	1

#include "db-intf.h"

/**
 */
class DatabaseJSON : public DatabaseIntf
{
private:
    std::string url;    // POST
    std::string login;  // remember user credentials
    std::string password;  // remember user credentials
    std::string authorizationUrl;   // POST
    std::string auth;   // session auth token
public:
    DatabaseJSON();
	~DatabaseJSON();

    virtual int open(
		const std::string &connection,
		const std::string &login,
		const std::string &password,
		const std::string &auth,
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
    std::string errMessage;
};

#endif
