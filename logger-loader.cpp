#include "logger-loader.h"
#include "logger-huffman/logger-parse.h"
#include "db-any.h"

DbLoggerKosaPacketsLoader::DbLoggerKosaPacketsLoader()
    : db(nullptr), dbDialect(0)
{
}

DbLoggerKosaPacketsLoader::DbLoggerKosaPacketsLoader(DatabaseIntf *adb)
{
    setDatabase(adb);
}

DbLoggerKosaPacketsLoader::~DbLoggerKosaPacketsLoader()
{
}

bool DbLoggerKosaPacketsLoader::load(
    LoggerKosaPackets &retVal,
    uint32_t addr
)
{
    if (db) {
        std::vector<std::vector<std::string> > rowsNcols;
        std::string sqlStatement = loggerBuildSQLBaseMeasurementSelect(dbDialect, addr);
        int r = db->select(rowsNcols, sqlStatement);

        if (r)
            return false;
        if (rowsNcols.empty())
            return false;
        if (rowsNcols[0].empty())
            return false;
        std::vector <std::string> packets;
        loggerParseSQLBaseMeasurement(packets, rowsNcols[0][0]);
        if (packets.empty())
            return false;
        LoggerKosaCollector collector;
        collector.put(addr, packets);
        if (collector.koses.empty())
            return false;
        retVal = collector.koses[0];
        return true;
    } else
        return false;
}

void DbLoggerKosaPacketsLoader::setDatabase(
    DatabaseIntf *aDb
) {
    db = aDb;
    dbDialect = sqlDialectByName(db->type);
}
