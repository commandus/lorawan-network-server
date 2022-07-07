#ifndef AUTH_USER_H
#define AUTH_USER 1

#include <string>

class AuthUserService {
private:
    std::string issuer;
    std::string secret;
public:
    // return true if user authorized
    AuthUserService(const std::string &issuer, const std::string &secret) {
        
    }
    bool verify(const std::string &user, const std::string &password) = 0;

    // return empty string if not authorized
    std::string getJWT(const std::string &user, const std::string &password) = 0;
};
#endif
