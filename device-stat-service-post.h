#ifndef DEVICE_STAT_SERVICE_POST_H_
#define DEVICE_STAT_SERVICE_POST_H_ 1

#include "device-stat-service-file.h"

/**
 * Device statistics service append statistics to the web service
 * specified in the option parameter of init() method
 */
class DeviceStatServicePost : public DeviceStatServiceFile {
	public:
        int save() override;
};

#endif
