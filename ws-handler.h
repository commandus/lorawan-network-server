#ifndef WS_HANDLER_H_
#define WS_HANDLER_H_	1

#include "lorawan-ws/lorawan-ws.h"
#include "config-json.h"
#include "database-config-json.h"
#include "auth-user.h"
#include "regional-parameter-channel-plans.h"
#include "identity-service.h"
#include "gateway-list.h"
#include "gateway-stat-service-abstract.h"
#include "device-stat-service-abstract.h"

class WsSpecialPathHandler : public WebServiceRequestHandler
{
private:
    std::string versionString;
public:
    Configuration *config;
    ConfigDatabasesIntf *configDatabases;
    RegionalParameterChannelPlans *regionalParameterChannelPlans;
    IdentityService *identityService;
    GatewayList *gatewayList;
    GatewayStatService *gatewayStatService;
    DeviceStatService *deviceStatService;
    void *loggerParser;
    AuthUserService *authUserService;

    WsSpecialPathHandler();
    int handle(
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
    ) override;
};

#endif
