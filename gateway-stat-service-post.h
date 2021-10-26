#ifndef GATEWAY_STAT_SERVICE_POST_H_
#define GATEWAY_STAT_SERVICE_POST_H_ 1

#include "gateway-stat-service-abstract.h"

/**
 * Gateway statistics service append statistics to the web service
 * specified in the option paramater of init() method
 */
class GatewayStatServicePost : public GatewayStatService {
	public:
		virtual void put(GatewayStat *stat);
		// force save
		virtual void flush();
		// reload
		virtual int init(const std::string &option, void *data);
		// close resources
		virtual void done();
};

#endif
