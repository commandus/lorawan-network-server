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
 *		["mqtt-wss-topic", "#"],
 *		["mqtt-wss-send-binary", "false"],
 *  ]
 */
#include <sstream>

#include "plugin-mqtt-wss.h"

#include "mqtt-client-id.h"
#include "utilstring.h"
#include "errlist.h"

void MqttSubscribeListener::on_failure(
    const mqtt::token& tok
) {
    if (clientEnv && clientEnv->log) {
        std::stringstream ss;
        ss << "Subscribe error " << tok.get_return_code() << " reason "
            << tok.get_reason_code()
            << " token: " << tok.get_message_id();
        clientEnv->log->logMessage(this, LOG_ERR, LOG_PLUGIN_MQTT,
    tok.get_return_code(), ss.str());
    }
}

void MqttSubscribeListener::on_success(
    const mqtt::token& tok
)
{
}

MqttSubscribeListener::MqttSubscribeListener(
    MqttClientEnv *aClientEnv
)
    : clientEnv(aClientEnv)
{

}

void MqttPluginCallback::reconnect()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    try {
        clientEnv->mqttClient->connect(clientEnv->connOpts, nullptr, *this);
    } catch (const mqtt::exception& exc) {
        if (clientEnv && clientEnv->log) {
            std::stringstream ss;
            ss << ERR_PLUGIN_MQTT_CONNECT << exc.what();
            clientEnv->log->logMessage(this, LOG_ERR, LOG_PLUGIN_MQTT,
        exc.get_reason_code(), ss.str());
        }
    }
}

// Re-connection failure
void MqttPluginCallback::on_failure(const mqtt::token& tok)  {
    if (clientEnv && clientEnv->log) {
        std::stringstream ss;
        ss << ERR_PLUGIN_MQTT_CONNECT << tok.get_return_code()
            << " reason " << tok.get_reason_code();
        clientEnv->log->logMessage(this, LOG_ERR, LOG_PLUGIN_MQTT,
    tok.get_reason_code(), ss.str());
    }
    reconnect();
}

// (Re)connection success
// Either this or connected() can be used for callbacks.
void MqttPluginCallback::on_success(const mqtt::token& tok)  {}

// (Re)connection success
void MqttPluginCallback::connected(const std::string& cause)  {
    if (clientEnv && clientEnv->log) {
        std::stringstream ss;
        ss << "Connected, client " << getMqttClientId() << " QoS" << clientEnv->qos;
        clientEnv->log->logMessage(this, LOG_INFO, LOG_PLUGIN_MQTT,
            0, ss.str());
    }
}

// Callback for when the connection is lost.
// This will initiate the attempt to manually reconnect.
void MqttPluginCallback::connection_lost(const std::string& cause)  {
    if (clientEnv && clientEnv->log) {
        std::stringstream ss;
        ss << "Connection lost, cause " << cause;
        clientEnv->log->logMessage(this, LOG_INFO, LOG_PLUGIN_MQTT,
0, ss.str());
    }
    reconnect();
}

// Callback for when a message arrives.
void MqttPluginCallback::message_arrived(mqtt::const_message_ptr msg)  {
    if (clientEnv && clientEnv->log) {
        std::stringstream ss;
        ss << "MQTT message arrived, topic: " << msg->get_topic()
            << ", payload: " << msg->to_string();
        clientEnv->log->logMessage(this, LOG_INFO, LOG_PLUGIN_MQTT,
                                   0, ss.str());
    }
}

void MqttPluginCallback::delivery_complete(mqtt::delivery_token_ptr token)  {

}

MqttPluginCallback::MqttPluginCallback(
    MqttClientEnv *aClientEnv
)
    : clientEnv(aClientEnv), subscribeListener(aClientEnv)
{

}

MqttClientEnv::MqttClientEnv()
    : log(nullptr), lastStatusCode(0), qos(1), callback(this), sendBinary(true)
{
    clientId = getMqttClientId();
}

MqttClientEnv::MqttClientEnv(
    const std::map<std::string, std::vector <std::string> > &params,
    LogIntf *aLog
)
    : MqttClientEnv()
{
    log = aLog;
    std::string v;

    std::map<std::string, std::vector<std::string> >::const_iterator pit = params.find("mqtt-wss-send-binary");
    if (pit != params.end()) {
        if (!pit->second.empty() && !pit->second[0].empty()) {
            const char firstChar = *pit->second[0].c_str();
            sendBinary = (firstChar == 'T') || (firstChar == '1') || (firstChar == 't');
        }
    }

    for (int i = MQTT_WSS_SERVICE; i <= MQTT_WSS_TOPIC; i++) {
        std::map<std::string, std::vector<std::string> >::const_iterator paramIt = params.find(paramNames[i]);
        if (paramIt != params.end()) {
            if (!paramIt->second.empty())
                mqttParameters[i] = paramIt->second[0];
        }
    }
    mqttClient = new mqtt::async_client(mqttParameters[MQTT_WSS_SERVICE], clientId);
    mqtt::ssl_options sslOptions = mqtt::ssl_options_builder()
        .enable_server_cert_auth(false)
        .error_handler([](const std::string& msg) {
            std::cerr << "SSL Error: " << msg << std::endl;
        })
        .finalize();
    connOpts = mqtt::connect_options_builder()
        .keep_alive_interval(std::chrono::seconds(10))
        .ssl(sslOptions)
        .mqtt_version(MQTT_SSL_VERSION_DEFAULT)
        .automatic_reconnect(true)
        .user_name(mqttParameters[MQTT_WSS_USER_NAME])
        .password(mqttParameters[MQTT_WSS_USER_PASSWORD])
        .clean_session(false)
        .finalize();

    lastStatusCode = clientConnect();
}

MqttClientEnv::~MqttClientEnv() {
    if (mqttClient) {
        clientDisconnect();
        delete mqttClient;
        mqttClient = nullptr;
    }
}

int MqttClientEnv::clientDisconnect() {
    try {
        if (mqttClient) {
            if (mqttClient->is_connected()) {
                if (log) {
                    std::stringstream ss;
                    ss << MSG_PLUGIN_MQTT_DISCONNECTING << mqttParameters[MQTT_WSS_SERVICE];
                    log->logMessage(this, LOG_INFO, LOG_PLUGIN_MQTT,
                        0, ss.str());
                    return ERR_CODE_PLUGIN_MQTT_DISCONNECT;
                }
                mqttClient->disconnect()->wait();

                if (log) {
                    std::stringstream ss;
                    ss << MSG_PLUGIN_MQTT_DISCONNECTING << mqttParameters[MQTT_WSS_SERVICE];
                    log->logMessage(this, LOG_INFO, LOG_PLUGIN_MQTT,
                                    0, ss.str());
                    return ERR_CODE_PLUGIN_MQTT_DISCONNECT;
                }
            }
        }
    } catch (const mqtt::exception &e) {
        if (log) {
            std::stringstream ss;
            ss << ERR_PLUGIN_MQTT_DISCONNECT << e.get_message() << " reason "
               << e.get_reason_code()
               << " return code " << e.get_return_code();
            log->logMessage(this, LOG_ERR, LOG_PLUGIN_MQTT,
                            e.get_reason_code(), ss.str());
            return ERR_CODE_PLUGIN_MQTT_DISCONNECT;
        }
    }
    return 0;
}

int MqttClientEnv::clientConnect() {
    try {
        // Connect to the server
        if (log) {
            std::stringstream ss;
            ss << MSG_PLUGIN_MQTT_CONNECTING << mqttParameters[MQTT_WSS_SERVICE] << "...";
            log->logMessage(this, LOG_DEBUG, LOG_PLUGIN_MQTT, 0, ss.str());
        }
        // connect response will block waiting for the connection to complete.
        int version = mqttClient->connect(connOpts)->get_connect_response().get_mqtt_version();
        if (log) {
            std::stringstream ss;
            ss << MSG_PLUGIN_MQTT_CONNECTED << mqttParameters[MQTT_WSS_SERVICE] << " version " << version;
            log->logMessage(this, LOG_DEBUG, LOG_PLUGIN_MQTT, 0, ss.str());
        }
    } catch (const mqtt::exception& e) {
        if (log) {
            std::stringstream ss;
            ss << ERR_PLUGIN_MQTT_CONNECT << e.get_message() << " reason "
                << e.get_reason_code()
                << ":  " << e.get_reason_code()
                << ", return code " << e.get_return_code()
                << ":  " << e.get_error_str();
            log->logMessage(this, LOG_ERR, LOG_PLUGIN_MQTT,
                e.get_reason_code(), ss.str());
            return ERR_CODE_PLUGIN_MQTT_CONNECT;
        }
    }
    return 0;
}

extern "C" void *pluginInit(
    DatabaseByConfig *dbByConfig,
    const std::string &protoPath,
    const std::map<std::string, std::vector <std::string> > &params,
    LogIntf *log,
    int verbosity   // always 0
)
{
    MqttClientEnv *cliEnv = new MqttClientEnv(params, log);
    if (log) {
        std::stringstream ss;
        ss << MSG_PLUGIN_MQTT_INIT
            << " url: " << cliEnv->mqttParameters[MQTT_WSS_SERVICE]
            << " user: " << cliEnv->mqttParameters[MQTT_WSS_USER_NAME]
        //  << " password: " << cliEnv->mqttParameters[MQTT_WSS_USER_PASSWORD]
            << " status " << cliEnv->lastStatusCode;
        log->logMessage(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }
    return cliEnv;
}

extern "C" void pluginDone(
    void *env
)
{
    if (env) {
        MqttClientEnv *c = (MqttClientEnv *) env;
        if (c->log) {
            std::stringstream ss;
            ss << MSG_PLUGIN_MQTT_DONE << " status " << c->lastStatusCode;
            c->log->logMessage(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss.str());
        }
        delete c;
    }
}

/**
 * Replaces temperature/$(devname) to temperature/SI-13 using property values.
 * @param topic Topic with placeholders $(property-name) e.g. temperature/$(devname)
 * @param properties Property name=values map.
 * @return MQTT topic with substituted property values
 */
std::string buildTopic(
    const std::string &topic,
    const std::map<std::string, std::string> *properties
) {
    if (!properties)
        return topic;
    std::string r = topic;

    int cnt = 128; // prevent cyclic replacement
    std::string::size_type start = 0, finish;
    while((start = r.find("$(", start)) != std::string::npos) {
        std::string::size_type startVar = start + 2;
        if ((finish = r.find(")", startVar)) != std::string::npos) {
            std::string s = r.substr(startVar, finish - startVar);
            std::map<std::string, std::string>::const_iterator it(properties->find(s));
            if (it == properties->end()) {
                r.replace(start, finish - start + 1, "");
            } else {
                r.replace(start, finish - start + 1, it->second);
                start += it->second.length();
            }
        }
        cnt--;
        if (cnt <= 0)
            break;
    }
    return r;
}

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
    if (!env)
        return 0;
    MqttClientEnv *c = (MqttClientEnv *) env;
    if (!c->mqttClient)
        return 0;
    std::string topic = buildTopic(c->mqttParameters[MQTT_WSS_TOPIC], properties);
    if (c->log) {
        std::stringstream ss;
        ss << MSG_PLUGIN_MQTT_SENDING << c->mqttParameters[MQTT_WSS_SERVICE]
           << " topic " << topic << " " << hexString(data);
        c->log->logMessage(c, LOG_INFO, LOG_PLUGIN_MQTT,
        0, ss.str());
    }
    try {
        if (c->sendBinary)
            c->mqttClient->publish(topic,
                data.c_str(), data.size(), c->qos, false)
                ->wait_for(std::chrono::seconds(60));
        else {
            std::string p = hexString(data);
            c->mqttClient->publish(topic,
                                   p.c_str(), p.size(), c->qos, false)
                    ->wait_for(std::chrono::seconds(60));
        }
    } catch (const mqtt::exception &e) {
        if (c->log) {
            std::stringstream ss;
            ss << ERR_PLUGIN_MQTT_SEND << e.get_message() << " reason "
               << e.get_reason_code()
               << ":  " << e.get_reason_code()
               << ", return code " << e.get_return_code()
               << ":  " << e.get_error_str()
               ;
            c->log->logMessage(nullptr, LOG_ERR, LOG_PLUGIN_MQTT,
                e.get_reason_code(), ss.str());
            return 0;
        }
    }

    if (c->log) {
        std::stringstream ss;
        ss << MSG_PLUGIN_MQTT_SENT << c->mqttParameters[MQTT_WSS_SERVICE]
            << " topic " << topic
            << " " << hexString(data);
        c->log->logMessage(c, LOG_INFO, LOG_PLUGIN_MQTT,
                           0, ss.str());
    }
    return 0;   // processed, but no INSERT clauses produced
}
