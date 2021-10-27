#include "device-stat-service-abstract.h"

DEVICE_STAT_STORAGE string2deviceStatStorageType(
        const std::string &value
) {
    if (value == "file")
        return DEVICE_STAT_FILE_JSON;
    if (value == "post")
        return DEVICE_STAT_POST;
    return DEVICE_STAT_NONE;
};

std::string deviceStatStorageType2String(
    DEVICE_STAT_STORAGE value
) {
    switch(value) {
        case DEVICE_STAT_FILE_JSON:
            return "file";
        case DEVICE_STAT_POST:
            return "post";
        default:
            return "";
    }
}
