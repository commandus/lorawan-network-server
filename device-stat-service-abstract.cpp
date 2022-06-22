#include "device-stat-service-abstract.h"

DEVICE_STAT_STORAGE string2deviceStatStorageType(
        const std::string &value
) {
    if (value == "json")
        return DEVICE_STAT_FILE_JSON;
    if (value == "csv")
        return DEVICE_STAT_FILE_CSV;
    if (value == "post")
        return DEVICE_STAT_POST;
    return DEVICE_STAT_NONE;
};

std::string deviceStatStorageType2String(
    DEVICE_STAT_STORAGE value
) {
    switch(value) {
        case DEVICE_STAT_FILE_JSON:
            return "json";
        case DEVICE_STAT_FILE_CSV:
            return "csv";
        case DEVICE_STAT_POST:
            return "post";
        default:
            return "none";
    }
}
