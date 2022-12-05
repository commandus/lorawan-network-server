#ifndef AUTH_FILE_H
#define AUTH_FILE_H 1

#include <map>

#include "auth-user.h"

class AuthUserFile : public AuthUserService{
private:
    std::map<std::string, std::string> passwords;
    void load(const std::string &json);
public:
    AuthUserFile(const std::string &issuer, const std::string &secret, const std::string &json);

    // return true if user authorized
    bool verify(const std::string &user, const std::string &password) override;
    // return empty string if not authorized
    std::string getToken(const std::string &user, const std::string &password) override;
};

#endif
