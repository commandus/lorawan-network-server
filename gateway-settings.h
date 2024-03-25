#ifndef GATEWAY_SETTINGS
#define GATEWAY_SETTINGS 1

/**
 * Lora gateway settings structure access method
 */
#include <string>
#include "gateway-lora.h"

class GatewaySettings {
public:
    sx1261_config_t sx1261;
    sx130x_config_t sx130x;
    gateway_t gateway;
    struct lgw_conf_debug_s debug;
    std::string serverAddr;
    std::string gpsTtyPath;
    std::string name;
};

#endif
