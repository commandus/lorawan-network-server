#include <sstream>
#include <iomanip>
#include "payload-insert.h"
/**
 * Required lorawan-network-server.json must provide 4 parameters
 * 
 * "pluginsParams": [
 *      ["mqtt-wss-service", "wss://mqtt.acme.com:443"],
 *		["mqtt-wss-user-name", "mosquitto-user-name"],
 *		["mqtt-wss-password", "mosquitto-user-password"],
 *		["mqtt-wss-topic", "#"]
 *  ]
 */

/**
 * @return count of added to the retClauses "INSERT" clauses. Return <0 if data is not parseable.
 * If return value is <0, next loaded function would be called.
 * If return value is 0 or >0, chain of loaded function is cancelled.
 * @param retClauses return INSERT clauses(usually one)
 * @param env
 * @param message name of preferred handler (message type name). Default "".
 * @param inputFormat: 0- binary (always) 1- hex (never used)
 * @param outputFormat 0- json 3- sql
 * @param sqlDialect SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
 * @param data: payload
 * @param properties: LoRaWAN metadata properties
 * @param tableAliases optional aliases (not used)
 * @param fieldAliases optional aliases (not used)
 * @param nullValueString: magic number "8888" by default. You can set it to "NULL".
 *
 * Property keys are:
 * activation (ABP|OTAA)
 * class A|B|C
 * deveui global end-device identifier in IEEE EUI64 address space
 * appeui
 * appKey
 * nwkKey
 * devNonce
 * joinNonce
 * name device name
 * version LoRaWAN version
 * addr network address string
 * fport application port number (1..223). 0- MAC, 224- test, 225..255- reserved as decimal number string
 * id packet id
 * time (32 bit integer, seconds since Unix epoch) as decimal number string
 * timestamp string
 * id number of packet received by the server as decimal number string
 */
extern "C" int payload2InsertClauses(
    std::vector<std::string> &retClauses,
    void *env,
    const std::string &message,
    int inputFormat,    // always 0
    int outputFormat,   // 3- SQL, 0- JSON
    int sqlDialect, // SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::string &data,
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties,
    const std::string &nullValueString
)
{
    if (outputFormat == 0) {
        // JSON not implemented yet
        return -1;
    }
    size_t sz = data.size();
    return -1;
}
