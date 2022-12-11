/**
 * Simple LoRaWAN network server MQTT plugin
 * MQTT plugin send payload to specified MQTT broker.
 * LoRaWAN network server's configuration file (lorawan-network-server.json)
 * must provide parameters:
 *
 * "pluginsParams": [
 *      ["mqtt-wss-service", "wss://mqtt.acme.com:443"],
 *		["mqtt-wss-user-name", "mosquitto-user-name"],
 *		["mqtt-wss-password", "mosquitto-user-password"],
 *		["mqtt-wss-topic", "#"]
 *  ]
 */
#include <sstream>

#include "mqtt/async_client.h"

#include "payload-insert.h"

#ifdef _MSC_VER
#define EXPORT_PLUGIN_FUNC extern "C" __declspec(dllexport)
#else
#define EXPORT_PLUGIN_FUNC extern "C"
#endif

class MqttClientEnv;
class MqttSubscribeListener : public virtual mqtt::iaction_listener
{
    MqttClientEnv *clientEnv;
    void on_failure(const mqtt::token& tok) override;
    void on_success(const mqtt::token& tok) override;
public:
    MqttSubscribeListener(MqttClientEnv *aClientEnv);
};

class MqttPluginCallback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
    // The MQTT client
    MqttClientEnv *clientEnv;
    // Options to use if we need to reconnect
    MqttSubscribeListener subscribeListener;

    void reconnect();
    // Re-connection failure
    void on_failure(const mqtt::token& tok) override;

    // (Re)connection success
    // Either this or connected() can be used for callbacks.
    void on_success(const mqtt::token& tok) override;

    // (Re)connection success
    void connected(const std::string& cause) override;

    // Callback for when the connection is lost.
    // This will initiate the attempt to manually reconnect.
    void connection_lost(const std::string& cause) override;

    // Callback for when a message arrives.
    void message_arrived(mqtt::const_message_ptr msg) override;
    void delivery_complete(mqtt::delivery_token_ptr token) override;

public:
    MqttPluginCallback(MqttClientEnv *aClientEnv);
};

typedef enum {
    MQTT_WSS_SERVICE = 0,
    MQTT_WSS_USER_NAME = 1,
    MQTT_WSS_USER_PASSWORD = 2,
    MQTT_WSS_TOPIC = 3
} MQTT_PARAMETER_NAME;

/**
 * Set parameters in the LoRaWAN network server's configuration file (lorawan-network-server.json)
 * "pluginsParams": [
 * ["mqtt-wss-service", "wss://mqtt.acme.com:443"],
 * ...
 * ]
 */
static const char *paramNames[4] = {
    "mqtt-wss-service",
    "mqtt-wss-user-name",
    "mqtt-wss-password",
    "mqtt-wss-topic"
};

class MqttClientEnv {
public:
    MqttPluginCallback callback;
    DatabaseByConfig *dbByConfig;
    std::string clientId;
    std::string mqttParameters[4];     ///< e.g. "wss://mqtt.acme.com:443"],
    mqtt::ssl_options sslOptions;
    mqtt::connect_options connOpts;
    mqtt::async_client *mqttClient;

    int qos;
    bool sendBinary;

    LogIntf *log;
    int lastStatusCode;

    MqttClientEnv();

    MqttClientEnv(
        const std::map<std::string, std::vector <std::string> > &params,
        LogIntf *aLog
    );

    ~MqttClientEnv();

    int clientDisconnect();

    int clientConnect();
};

EXPORT_PLUGIN_FUNC void *pluginInit(
    DatabaseByConfig *dbByConfig,
    const std::string &protoPath,
    const std::map<std::string, std::vector <std::string> > &params,
    LogIntf *log,
    int verbosity   // always 0
);

EXPORT_PLUGIN_FUNC void pluginDone(
    void *env
);

/**
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
EXPORT_PLUGIN_FUNC int payload2InsertClauses(
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
);
