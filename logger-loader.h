#ifndef DB_LOGGER_PLUME_PACKETS_LOADER_H
#define DB_LOGGER_PLUME_PACKETS_LOADER_H     1

#include "logger-huffman/logger-collection.h"
#include "db-intf.h"

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
    ~DbLoggerKosaPacketsLoader();
    bool load(LoggerKosaPackets &retVal, uint32_t addr) override;
    /**
     * Set database interface
     * @param db
     */
    void setDatabase(DatabaseIntf *db);
};

#endif
