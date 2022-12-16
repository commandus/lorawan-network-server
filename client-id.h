#ifndef CLIENT_ID_H_
#define CLIENT_ID_H_	1

#include <string>

/**
 * Get MQTT client unique identifier
 * @return MQTT client unique identifier string
 * "lns-plg-XX" where XX is MAC address of the first found network adapter
 */
std::string getMqttClientId();

/**
 * Get gateway unique identifier
 * @return gateway unique identifier
 * MAC address of the first found network adapter plus zeroed last 2 bytes
 */
uint64_t getGatewayId();

#endif
