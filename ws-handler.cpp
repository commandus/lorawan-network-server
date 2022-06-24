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
        loggerParser(nullptr)
{

}

bool WsSpecialPathHandler::handle(
    std::string &content,
    std::string &contentType,
    void *env,
    int modulecode,
    // copy following parameters from the web request
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size
)
{
    std::string p(url);
    if (p.find("/about") == 0) {
        content = "{\"version\": \"" + versionString + "\"}";
        return true;
    }
    if (p.find("/config") == 0) {
        if (config) {
            content = config->toString();
        }
        return true;
    }
    if (p.find("/databases") == 0) {
        if (configDatabases) {
            content = configDatabases->toString();
        }
        return true;
    }
    if (p.find("/plans") == 0 || p.find("/regional") == 0) {
        if (regionalParameterChannelPlans) {
            content = regionalParameterChannelPlans->toJsonString();
        }
        return true;
    }
    if (p.find("/loggers") == 0 || p.find("/passports") == 0) {
#ifdef ENABLE_LOGGER_HUFFMAN
        std::vector<std::string> js;
        // 1- text, 2- JSON
        size_t cnt = lsPassports(loggerParser, 0, nullptr, 0, 0);
        lsPassports(loggerParser, 2, &js, 0, cnt);
        std::stringstream ss;
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
        content = ss.str();
#else
        content = "{\"error\": \"Feature disabled\"}";
#endif
        return true;
    }

    if (p.find("/devices") == 0 || p.find("/identities") == 0) {
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
        return true;
    }
    if (p.find("/gateways") == 0) {
        if (gatewayList) {
            content = gatewayList->toJsonString();
        }
        return true;
    }
    if (p.find("/gateway-stat") == 0) {
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
        return true;
    }
    if (p.find("/device-stat") == 0) {
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
        return true;
    }

    return false;
}

