#ifndef DUMB_LOGGER_LOADER_H
#define DUMB_LOGGER_LOADER_H     1

#include "logger-huffman/logger-collection.h"

#endif

/**
 * Database logger plume loader class implementation
 */
class DbLoggerKosaPacketsLoader: public LoggerKosaPacketsLoader {
public:
    LoggerKosaCollector *collection;
    DbLoggerKosaPacketsLoader();
    bool load(LoggerKosaPackets &retVal, uint32_t addr) override;
    void setCollection(LoggerKosaCollector *aCollection);
};
