#include "mqtt-client-id.h"

#include <sstream>
#include <iomanip>
#include <cstring>

#ifdef _MSC_VER
#include <Windows.h>
#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

#define DEF_MQTT_CLIENT_NAME    "lns-plg-"

#ifdef _MSC_VER

/*
 * @return unique MQTT client identifier string as MAC address
 * @see https://stackoverflow.com/questions/13646621/how-to-get-mac-address-in-windows-with-c
 */
std::string getMqttClientId()
{
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
    unsigned char mac_addr[6];

    AdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
    if (!AdapterInfo) {
        free(mac_addr);
        return DEF_MQTT_CLIENT_NAME;
    }

    // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO *) malloc(dwBufLen);
        if (!AdapterInfo) {
            free(mac_addr);
            return DEF_MQTT_CLIENT_NAME;
        }
    }

    bool success = false;

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
        // Contains pointer to current adapter info
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        do {
            success = true;
            if (pAdapterInfo-> AddressLength != 6)
                continue;
            memmove(mac_addr, pAdapterInfo->Address, 6);
            // TODO select best MAC address if some
            break;
            // pAdapterInfo = pAdapterInfo->Next;
        } while(pAdapterInfo);
    }
    free(AdapterInfo);

    if (!success)
        return DEF_MQTT_CLIENT_NAME;
    std::stringstream ss;
    ss << DEF_MQTT_CLIENT_NAME;
    for (int i = 0; i < 6; ++i)
        ss << std::setfill('0') << std::setw(2) << std::hex << (int) mac_addr[i];
    return ss.str();
}

#else

/**
 * @return unique MQTT client identifier string as MAC address
 * @see https://stackoverflow.com/questions/1779715/how-to-get-mac-address-of-your-machine-using-a-c-program
 */
std::string getMqttClientId()
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    bool success = false;

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
                    success = true;
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

#endif
