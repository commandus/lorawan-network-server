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
    jwt::builder<jwt::traits::kazuho_picojson> token = jwt::create()
    .set_issuer(issuer)
    .set_type("JWS");

    for (std::map<std::string, std::string>::const_iterator it(claims.begin()); it != claims.end(); it++) {
        token.set_payload_claim(it->first, jwt::claim(it->second));
    }

    return token.sign(jwt::algorithm::hs256{ secret });
#else
    return "";
#endif

}
