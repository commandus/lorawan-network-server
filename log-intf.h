#ifndef LOG_INTF_H_
#define LOG_INTF_H_	1

class Payload {
    time_t received;
    std::string emui;
    std::string devName;
    std::string hexPayload;
    int frequency;
    int rssi;
    float lsnr;
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
    virtual void onValue(Payload &value) = 0;
};

#endif
