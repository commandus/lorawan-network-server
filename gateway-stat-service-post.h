#ifndef GATEWAY_STAT_SERVICE_POST_H_
#define GATEWAY_STAT_SERVICE_POST_H_ 1

#include "gateway-stat-service-file.h"

/**
 * Gateway statistics service append statistics to the web service
 * specified in the option paramater of init() method
 */
class GatewayStatServicePost : public GatewayStatServiceFile {
	public:
        virtual void save();
};

#endif
