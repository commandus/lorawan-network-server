#ifndef LOGGER_KOSA_LOADER_H
#define LOGGER_KOSA_LOADER_H     1

#include <inttypes.h>

#include "logger-collection.h"

class LoggerKosaPacketsLoader {
public:
    /**
     * Load last kosa record with "base" values by address
     * @param retVal set kosa packets if found
     * @param addr kosa address
     * @return true- kosa with "base" record found
     */
    virtual bool load(LoggerKosaPackets &retVal, uint32_t addr) = 0;
};

#endif
