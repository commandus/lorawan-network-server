#include "gateway-stat-service-abstract.h"

GatewayStatService::GatewayStatService()
{

}

GatewayStatService::GatewayStatService(
    const GatewayStatService &value
)
{

}

GatewayStatService::~GatewayStatService()
{

}

GW_STAT_STORAGE string2gwStatStorageType(
        const std::string &value
) {
    if (value == "file")
        return GW_STAT_FILE_JSON;
    if (value == "post")
        return GW_STAT_POST;
    return GW_STAT_NONE;
}

std::string gwStatStorageType2String(
    GW_STAT_STORAGE value
) {
    switch(value) {
        case GW_STAT_FILE_JSON:
            return "file";
        case GW_STAT_POST:
            return "post";
        default:
            return "";
    }
}
