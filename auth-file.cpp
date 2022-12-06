#include "auth-file.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

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
    doc.Parse<rapidjson::kParseCommentsFlag>(json.c_str());
    if (doc.IsArray()) {
        for (rapidjson::SizeType i = 0; i < doc.Size(); i++) {
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
std::string AuthUserFile::getToken(
    const std::string &user,
    const std::string &password
)
{
    std::map<std::string, std::string>::const_iterator f = passwords.find(user);
    if (f != passwords.end()) {
        if (f->second == password) {
            std::map<std::string, std::string> claims;
            claims["user"] = user;
            claims["password"] = password;
            return jwtClaims(claims);
        }
    }
    return "";
}
