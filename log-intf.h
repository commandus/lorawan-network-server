#ifndef LOG_INTF_H_
#define LOG_INTF_H_	1

#include "utillora.h"   // DeviceId

class Payload {
public:
    time_t received;
    std::string eui;
    std::string devName;
    int frequency;
    int rssi;
    float lsnr;
    std::string payload;
};

class LogIntf {
public:
    virtual void onInfo(
        void *env,
        int level,
        int moduleCode,
        int errorCode,
        const std::string &message
    ) = 0;
    virtual void onConnected(bool connected) = 0;
    virtual void onDisconnected() = 0;
    virtual void onStarted(uint64_t gatewayId, const std::string regionName, size_t regionIndex) = 0;
    virtual void onFinished(const std::string &message) = 0;
    // raw (ciphered) message received
    virtual void onReceive(Payload &value) = 0;
    // after payload deciphered and plugin successfully processed payload
    virtual void onValue(Payload &value) = 0;
    // identity callbacks
    virtual int identityGet(DeviceId &retVal, DEVADDR &devAddr) = 0;
    virtual int identityGetNetworkIdentity(NetworkIdentity &retVal, const DEVEUI &eui) = 0;
    // Entries count
    virtual size_t identitySize() = 0;
};

#endif
