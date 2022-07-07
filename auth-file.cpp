#include "auth-file.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#pragma clang diagnostic pop

AuthUserFile::AuthUserFile(
    const std::string &issuer,
    const std::string &secret,
    const std::string &json
)
    : AuthUserService(issuer, secret)
{
    load(json);
}

void AuthUserFile::load(
    const std::string &json
)
{
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.IsArray()) {
        for (int i = 0; i < doc.Size(); i++) {
            std::string userName;
            std::string password;
            rapidjson::Value &cred = doc[i];
            if (cred.IsObject()) {
                if (cred.HasMember("user")) {
                    rapidjson::Value &u = cred["user"];
                    if (u.IsString()) {
                        std::string user = u.GetString();
                        if (cred.HasMember("password")) {
                            rapidjson::Value &p = cred["password"];
                            if (p.IsString()) {
                                std::string pwd = p.GetString();
                                passwords[user] = pwd;
                            }
                        }
                    }
                }
            }
        }
    }
}

// return true if user authorized
bool AuthUserFile::verify(
    const std::string &user,
    const std::string &password
)
{
    std::map<std::string, std::string>::const_iterator f = passwords.find(user);
    if (f == passwords.end())
        return false;
    return (f->second == password);
}

// return empty string if not authorized
std::string AuthUserFile::getJWT(
    const std::string &user,
    const std::string &password
)
{
    std::map<std::string, std::string>::const_iterator f = passwords.find(user);
    if (f != passwords.end()) {
        if (f->second == password) {
            std::map<std::string, std::string> claims;
            claims["user"] = user;
            return jwtClaims(claims);
        }
    }
    return "";
}
