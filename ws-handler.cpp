#include <string>
#include "ws-handler.h"
#include "utillora.h"
#ifdef ENABLE_LOGGER_HUFFMAN
#include "logger-huffman/logger-parse.h"
#endif

#ifndef LNS_VERSION
#define LNS_VERSION 0.0
#endif

#define X_STR(a) XX_STR(a)
#define XX_STR(a) #a

#define LNS_VERSION_STR X_STR(LNS_VERSION)

WsSpecialPathHandler::WsSpecialPathHandler()
    : versionString(LNS_VERSION_STR), config(nullptr),
      configDatabases(nullptr), regionalParameterChannelPlans(nullptr),
      identityService(nullptr), gatewayList(nullptr),
      gatewayStatService(nullptr), deviceStatService(nullptr),
      loggerParser(nullptr), authUserService(nullptr)
{

}

int WsSpecialPathHandler::handle(
    std::string &content,
    std::string &contentType,
    void *env,
    int moduleCode,
    // copy following parameters from the web request
    const char *path,
    const char *method,
    const char *version,
    std::map<std::string, std::string> &params,
    const char *uploadData,
    size_t *uploadDataSize,
    bool authorized
)
{
    // Paths with no authorization required
    if (strcmp(method, "OPTIONS") == 0)
        return 200;
    std::string p(path);
    if (p.find("/about") == 0) {
        // 	version, enableJWT, enableLogger, enableSQLite, enablePostgres, enableMySQL, enableFirebird
        bool enableJWT = false;
        bool enableLogger = false;
        bool enableSQLite = false;
        bool enablePostgres = false;
        bool enableMySQL = false;
        bool enableFirebird = false;
#ifdef ENABLE_JWT
        enableJWT = true;
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
        enableLogger = true;
#endif
#ifdef ENABLE_DB_SQLITE
        enableSQLite = true;
#endif
#ifdef ENABLE_DB_POSTGRES
        enablePostgres = true;
#endif
#ifdef ENABLE_DB_MYSQL
        enableMySQL = true;
#endif
#ifdef ENABLE_DB_FIREBIRD
        enableFirebird = true;
#endif
        std::stringstream ss;
        ss << "{\"version\": \"" << versionString << "\", "
            << "\"enableJWT\": " << enableJWT << ","
            << "\"enableLogger\": " << enableLogger << ","
            << "\"enableSQLite\": " << enableSQLite << ","
            << "\"enablePostgres\": " << enablePostgres << ","
            << "\"enableMySQL\": " << enableMySQL << ","
            << "\"enableFirebird\": " << enableFirebird << "}";
        content = ss.str();
        return 200;
    }

    if (authUserService) {
        if (p.find("/token") == 0) {
            content = authUserService->getToken(params["user"], params["password"]);
            contentType = "text/plain";
            return 200;
        }
    }

    // Paths with authorization required
    if (p.find("/config") == 0) {
        if (!authorized)
            return 401;
        if (config) {
            content = config->toString();
        }
        return 200;
    }
    if (p.find("/databases") == 0) {
        if (!authorized)
            return 401;
        if (configDatabases) {
            content = configDatabases->toString();
        }
        return 200;
    }
    if (p.find("/plans") == 0 || p.find("/regional") == 0) {
        if (!authorized)
            return 401;
        if (regionalParameterChannelPlans) {
            content = regionalParameterChannelPlans->toJsonString();
        }
        return 200;
    }
    if (p.find("/passport-file") == 0) {
        if (!authorized)
            return 401;
        contentType = "text/plain";
#ifdef ENABLE_LOGGER_HUFFMAN
        int year = 0;
        int plume = 0;

        std::vector<std::string> fileContent;
        // 1- text, 2- JSON
        size_t ofs = 0;
        size_t sz = 0;
        std::string s;
        s = params["year"];
        if (!s.empty())
            year = std::strtol(s.c_str(), nullptr, 10);
        s = params["plume"];
        if (!s.empty())
            plume = std::strtol(s.c_str(), nullptr, 10);

        std::stringstream ss;
        s = params["o"];
        if (!s.empty())
            ofs = std::strtol(s.c_str(), nullptr, 10);
        s = params["s"];
        if (!s.empty())
            sz = std::strtol(s.c_str(), nullptr, 10);
        lsPassports(loggerParser, 1, &fileContent, year, plume, ofs, sz);
        for (size_t i = 0; i < fileContent.size(); i++) {
            ss << fileContent[i];
        }
        ss << "NEND" << std::endl;
        content = ss.str();
#else
        content = "Feature disabled";
#endif
        return 200;
    }
    if (p.find("/passport") == 0) {
        if (!authorized)
            return 401;

#ifdef ENABLE_LOGGER_HUFFMAN
        int year = 0;
        int plume = 0;

        std::vector<std::string> js;
        // 1- text, 2- JSON
        size_t ofs = 0;
        size_t sz = 0;
        std::string s;
        s = params["year"];
        if (!s.empty())
            year = std::strtol(s.c_str(), nullptr, 10);
        s = params["plume"];
        if (!s.empty())
            plume = std::strtol(s.c_str(), nullptr, 10);

        std::stringstream ss;
        if (p.find("/passport-count") == 0) {
            size_t cnt = lsPassports(loggerParser, 0, nullptr, year, plume, 0, 0);
            ss << cnt;
        } else {
            s = params["o"];
            if (!s.empty())
                ofs = std::strtol(s.c_str(), nullptr, 10);
            s = params["s"];
            if (!s.empty())
                sz = std::strtol(s.c_str(), nullptr, 10);
            lsPassports(loggerParser, 2, &js, year, plume, ofs, sz);
            ss << "[";
            bool isFirst = true;
            for (size_t i = 0; i < js.size(); i++) {
                if (isFirst)
                    isFirst = false;
                else
                    ss << ", ";
                ss << js[i];
            }
            ss << "]";
        }
        content = ss.str();
#else
        content = "{\"error\": \"Feature disabled\"}";
#endif
        return 200;
    }

    if (p.find("/devices") == 0 || p.find("/identities") == 0) {
        if (!authorized)
            return 401;
        if (identityService) {
            std::stringstream ss;
            std::vector<NetworkIdentity> identities;
            identityService->list(identities, 0, 0);
            size_t c = 0;
            ss << "[";
            bool isFirst = true;
            for (std::vector<NetworkIdentity>::const_iterator it(identities.begin()); it != identities.end(); it++) {
                if (isFirst)
                    isFirst = false;
                else
                    ss << ", ";
                ss << it->toJsonString();
                c++;
            }
            ss << "]";
            content = ss.str();
        }
        return 200;
    }
    if (p.find("/gateways") == 0) {
        if (!authorized)
            return 401;
        if (gatewayList) {
            content = gatewayList->toJsonString();
        }
        return 200;
    }
    if (p.find("/gateway-stat") == 0) {
        if (!authorized)
            return 401;
        if (gatewayStatService) {
            std::stringstream ss;
            GatewayStat stat;
            ss << "[";
            bool isFirst = true;
            for (size_t i = 0; i < gatewayStatService->size(); i++) {
                if (gatewayStatService->get(stat, i)) {
                    if (isFirst)
                        isFirst = false;
                    else
                        ss << ", ";
                    ss << stat.toJsonString();
                }
            }
            ss << "]";
            content = ss.str();
        }
        return 200;
    }
    if (p.find("/device-stat") == 0) {
        if (!authorized)
            return 401;
        if (deviceStatService) {
            std::stringstream ss;
            SemtechUDPPacket lastPacket;
            ss << "[";
            bool isFirst = true;
            for (size_t i = 0; i < deviceStatService->size(); i++) {
                if (deviceStatService->get(lastPacket, i)) {
                    if (isFirst)
                        isFirst = false;
                    else
                        ss << ", ";
                    ss << lastPacket.toJsonString();
                }
            }
            ss << "]";
            content = ss.str();
        }
        return 200;
    }
    return 404;
}
