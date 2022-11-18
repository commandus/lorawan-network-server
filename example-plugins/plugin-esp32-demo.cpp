#include <sstream>
#include <iomanip>
#include "payload-insert.h"

#define SQL_POSTGRESQL 0
#define SQL_MYSQL 1
#define SQL_FIREBIRD 2
#define SQL_SQLITE 3

// Bluetooth or Wi-Fi event
typedef struct {
    uint8_t tag;		// 'B'- bluetooth 'W'- Wi-Fi
    int8_t rssi;		// RSSI
    uint8_t mac[6];		// Bluetooth or Wi-Fi MAC client address
} probe_ev_t;			// 8 bytes, send 8 bytes

// Temperature and hall sensor event
typedef struct {
    uint8_t tag;		// 'H'- Hall sensor
    uint8_t ev;			// reserved, always 0
    int16_t t;			// external sensor temperature, 10xC
    int16_t chip;		// chip temperature, 10xC
    int16_t hall;		// Hall sensor
} temperature_ev_t;		// 8 bytes, send 8 bytes

static const std::string tableNameTemperature = "esp32temperature";
static const std::string temperatureFieldNames[3] = {
        "t",			// external sensor temperature, 10xC
        "chip", 		// chip temperature, 10xC
        "hall"  		// Hall sensor
};

static const std::string tableNameWiFi = "esp32wifi";
static const std::string wifiFieldNames[2] = {
        "rssi",			// RSSI (int8_t)
        "mac"           // Wi-Fi MAC client address
};

static std::string insertTemperature(
    const temperature_ev_t *data,
    int sqlDialect, // SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::map<std::string, std::string> *properties,
    const std::string &nullValueString
)
{
    const std::string quote2 = "'";
    std::string quote;
    if (sqlDialect == SQL_MYSQL)
        quote = "`";    // MySQL's exceptions for spaces and reserved words
    else
        quote = "\"";

    std::stringstream ss;
    ss << "INSERT INTO " << quote << tableNameTemperature << quote << "("
        << quote << temperatureFieldNames[0] << quote
        << ", " << quote << temperatureFieldNames[1] << quote
        << ", " << quote << temperatureFieldNames[2] << quote;
    if (properties) {
        for (std::map<std::string, std::string>::const_iterator it(properties->begin()); it != properties->end(); it++) {
            ss << ", " << quote << it->first << quote;
        }
    }
    // values
    ss << ") VALUES ("
        << std::fixed << std::setprecision(1) << data->t / 10.0
        << ", " << std::fixed << std::setprecision(1) << data->chip / 10.0
        << ", " << data->hall;
    if (properties) {
        for (std::map<std::string, std::string>::const_iterator it(properties->begin()); it != properties->end(); it++) {
            ss << ", " << quote2 << it->second << quote2;
        }
    }
    ss << ");";
    return ss.str();
}

static std::string insertWiFiEvent(
    const probe_ev_t *data,
    int sqlDialect, // SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::map<std::string, std::string> *properties,
    const std::string &nullValueString
)
{
    const std::string quote2 = "'";
    std::string quote;
    if (sqlDialect == SQL_MYSQL)
        quote = "`";    // MySQL's exceptions for spaces and reserved words
    else
        quote = "\"";

    std::stringstream ss;
    ss << "INSERT INTO " << quote << tableNameWiFi << quote << "("
       << quote << wifiFieldNames[0] << quote
       << ", " << quote << wifiFieldNames[1] << quote;
    if (properties) {
        for (std::map<std::string, std::string>::const_iterator it(properties->begin()); it != properties->end(); it++) {
            ss << ", " << quote << it->first << quote;
        }
    }
    // values
    ss << ") VALUES (" << (int) data->rssi
        << ", " << quote2 << std::setw(2) << std::setfill('0') << std::hex
        << (int) data->mac[5]
        << (int) data->mac[4]
        << (int) data->mac[3]
        << (int) data->mac[2]
        << (int) data->mac[1]
        << (int) data->mac[0]
        << quote2;
    if (properties) {
        for (std::map<std::string, std::string>::const_iterator it(properties->begin()); it != properties->end(); it++) {
            ss << ", " << quote2 << it->second << quote2;
        }
    }
    ss << ");";
    return ss.str();
}

/**
 * @return count of added to the retClauses "INSERT" clauses. Return <0 if data is not parseable.
 * If return value is <0, next loaded function would be called.
 * If return value is 0 or >0, chain of loaded function is cancelled.
 * @param retClauses return INSERT clauses(usually one)
 * @param env
 * @param message name of preferred handler (message type name). Default "".
 * @param inputFormat: 0- binary (always) 1- hex (never used)
 * @param sqlDialect SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
 * @param data: payload
 * @param properties: LoRaWAN metadata properties
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
    int inputFormat,
    int sqlDialect, // SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::string &data,
    const std::map<std::string, std::string> *properties,
    const std::string &nullValueString
)
{
    size_t sz = data.size();
    switch (sz) {
        case 8:
            switch (data[0]) {
                case 'H':
                    retClauses.push_back(insertTemperature(
                        (temperature_ev_t *) data.c_str(), sqlDialect, properties, nullValueString));
                    return 1;
                case 'W':
                    retClauses.push_back(insertWiFiEvent(
                        (probe_ev_t *) data.c_str(), sqlDialect, properties, nullValueString));
                    return 1;
            }
    }
    return -1;
}
