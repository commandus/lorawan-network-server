#ifndef LOG_INTF_H_
#define LOG_INTF_H_	1

class LogIntf {
public:
    virtual void logMessage(
        void *env,
        int level,
        int moduleCode,
        int errorCode,
        const std::string &message
    ) = 0;
};

#endif
