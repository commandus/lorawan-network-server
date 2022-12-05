#ifndef AUTH_USER_H
#define AUTH_USER_H 1

#include <string>
#include <map>

class AuthUserService {
private:
    std::string issuer;
    std::string secret;
protected:
    std::string jwtClaims(const std::map<std::string, std::string> &claims);
public:
    AuthUserService(const std::string &issuer, const std::string &secret);

    // return true if user authorized
    virtual bool verify(const std::string &user, const std::string &password) = 0;
    // return empty string if not authorized, or JWT token
    virtual std::string getToken(const std::string &user, const std::string &password) = 0;
};

#endif
