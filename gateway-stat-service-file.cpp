#include "gateway-stat-service-file.h"
/**
 * Gateway statistics service append statistics to the file
 * specified in the option paramater of init() method
 */
void GatewayStatServiceFile::put(GatewayStat *stat)
{

}

// force save
void GatewayStatServiceFile::flush()
{

}

// reload
int GatewayStatServiceFile::init(
	const std::string &option,
	void *data
)
{

}

// close resources
 void GatewayStatServiceFile::done()
 {

 }
