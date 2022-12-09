#ifndef DEVICE_STAT_SERVICE_ABSTRACT_H_
#define DEVICE_STAT_SERVICE_ABSTRACT_H_ 1

#include "device-history-item.h"

typedef enum {
    DEVICE_STAT_NONE = 0,
    DEVICE_STAT_FILE_JSON = 1,
    DEVICE_STAT_FILE_CSV = 2,
    DEVICE_STAT_POST = 3,
} DEVICE_STAT_STORAGE;

/**
 * Device statistics service interface
 */
class DeviceStatService {
	public:
        DeviceStatService();
        DeviceStatService(const DeviceStatService &value);
        virtual ~DeviceStatService();
        virtual bool get(SemtechUDPPacket &retval, size_t id) = 0;
        virtual size_t size() = 0;

		virtual void put(const SemtechUDPPacket *packet) = 0;
		// force save
		virtual void flush() = 0;
		// reload
		virtual int init(const std::string &option, void *data) = 0;
		// close resources
		virtual void done() = 0;
};

DEVICE_STAT_STORAGE string2deviceStatStorageType(
        const std::string &value
);

std::string deviceStatStorageType2String(
        DEVICE_STAT_STORAGE value
);

#endif
