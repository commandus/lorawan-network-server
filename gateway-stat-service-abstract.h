#ifndef GATEWAY_STAT_SERVICE_ABSTRACT_H_
#define GATEWAY_STAT_SERVICE_ABSTRACT_H_ 1

#include "gateway-stat.h"

typedef enum {
    GW_STAT_NONE = 0,
    GW_STAT_FILE_JSON = 1,
    GW_STAT_POST = 2,
} GW_STAT_STORAGE;

/**
 * Gateway statistics service interface
 */
class GatewayStatService {
	public:
        virtual bool get(GatewayStat &retval, size_t id) = 0;
        virtual size_t size() = 0;
		virtual void put(GatewayStat *stat) = 0;
		// force save
		virtual void flush() = 0;
		// reload
		virtual int init(const std::string &option, void *data) = 0;
		// close resources
		virtual void done() = 0;
};

GW_STAT_STORAGE string2gwStatStorageType(
        const std::string &value
);

std::string gwStatStorageType2String
(
        GW_STAT_STORAGE value
);

#endif
