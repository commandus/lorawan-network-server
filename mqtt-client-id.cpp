#include "mqtt-client-id.h"

#include <sstream>
#include <iomanip>
#include <cstring>

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#define DEF_MQTT_CLIENT_NAME    "lns-plg-"

/**
 *
 * @return unique MQTT client identifier string
 * @see https://stackoverflow.com/questions/1779715/how-to-get-mac-address-of-your-machine-using-a-c-program
 */
std::string getMqttClientId()
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
        return DEF_MQTT_CLIENT_NAME;
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
        return DEF_MQTT_CLIENT_NAME;

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else
            return DEF_MQTT_CLIENT_NAME;
    }
    if (!success)
        return DEF_MQTT_CLIENT_NAME;
    std::stringstream ss;
    ss << DEF_MQTT_CLIENT_NAME;
    for (int i = 0; i < 6; ++i)
        ss << std::setfill('0') << std::setw(2) << std::hex << (int) (unsigned char) ifr.ifr_hwaddr.sa_data[i];
    return ss.str();
}
