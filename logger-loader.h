#ifndef DUMB_LOGGER_LOADER_H
#define DUMB_LOGGER_LOADER_H     1

#include "logger-huffman/logger-collection.h"
#include "db-intf.h"
#endif

/**
 * Database logger plume loader class implementation
 */
class DbLoggerKosaPacketsLoader: public LoggerKosaPacketsLoader {
private:
    /**
     * Store database dialect
     */
    int dbDialect;
    DatabaseIntf *db;
public:
    DbLoggerKosaPacketsLoader();
    DbLoggerKosaPacketsLoader(DatabaseIntf *db);
    bool load(LoggerKosaPackets &retVal, uint32_t addr) override;
    /**
     * Set database interface
     * @param db
     */
    void setDatabase(DatabaseIntf *db);
};
