#ifndef GATEWAY_SETTINGS
#define GATEWAY_SETTINGS 1

/**
 * Lora gateway settings structure access method
 */
#include <string>
#include "gateway-lora.h"

class GatewaySettings {
public:
    virtual sx1261_config_t *sx1261() = 0;
    virtual sx130x_config_t *sx130x() = 0;
    virtual gateway_t *gateway() = 0;
    virtual struct lgw_conf_debug_s *debug() = 0;
    virtual std::string *serverAddress() = 0;
    virtual  std::string *gpsTTYPath() = 0;
};

#endif
