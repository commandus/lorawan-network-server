#include "logger-loader.h"

DbLoggerKosaPacketsLoader::DbLoggerKosaPacketsLoader()
    : collection(nullptr)
{

}

bool DbLoggerKosaPacketsLoader::load(
    LoggerKosaPackets &retVal,
    uint32_t addr
)
{
    if (collection && (!collection->koses.empty())) {
        retVal = collection->koses[0];
        return true;
    } else
        return false;
}

void DbLoggerKosaPacketsLoader::setCollection(
    LoggerKosaCollector *aCollection
) {
    collection = aCollection;
}

