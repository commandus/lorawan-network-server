#include "auth-user.h"

#ifdef ENABLE_JWT
#include "jwt-cpp/jwt.h"
#endif

AuthUserService::AuthUserService(
    const std::string &aIssuer,
    const std::string &aSecret
)
    : issuer(aIssuer), secret(aSecret)
{

}

std::string AuthUserService::jwtClaims(
    const std::map<std::string, std::string> &claims
)
{
#ifdef ENABLE_JWT
    auto token = jwt::create()
    .set_issuer(issuer)
    .set_type("JWS");

    for (std::map<std::string, std::string>::const_iterator it(claims); it != claims.end(); it++) {
        token.set_payload_claim(it->first, jwt::claim(it->second))
    }

    token.sign(jwt::algorithm::hs256{ secret });
    return token;
#else
    return "";
#endif

}
