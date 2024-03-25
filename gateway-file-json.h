#ifndef GATEWAY_FILE_JSON_H_
#define GATEWAY_FILE_JSON_H_ 1

/**
 * Parse Lora gateway JSON file
 * Usage:
 *     std::string v = file2string(fn.c_str());
 *     GatewayConfigFileJson c;
 *     int r = c.parseString(v);
 *     if (r) {
 *       std::cerr << "Parse error " << c.errorDescription << " at " << c.errorOffset << std::endl;
 *       return r;
 *     }
 */
#include "gateway-lora.h"
#include "gateway-settings.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/document.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

class GatewayJsonConfig {
public:
    int errorCode;
    size_t errorOffset;
    std::string errorDescription;

    GatewayJsonConfig();
    int parseString(const std::string &json);
    virtual int parse(rapidjson::Value &jsonValue) = 0;
    virtual void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const = 0;
    virtual void toCpp(std::ostream &retVal, const std::string &name) const = 0;
    virtual void toHeader(std::ostream &retVal, const std::string &name) const = 0;
    std::string toString();
};

class GatewaySX1261Config : public GatewayJsonConfig {
public:
    sx1261_config_t value;
    GatewaySX1261Config();

    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
    void toCpp(std::ostream &retVal, const std::string &name) const override;
    void toHeader(std::ostream &retVal, const std::string &name) const override;
    void reset();

    bool operator==(const GatewaySX1261Config &value) const;
};

class GatewaySX130xConfig : public GatewayJsonConfig {
public:
    GatewaySX1261Config sx1261Config;
    sx130x_config_t value;

    GatewaySX130xConfig();
    void reset();
    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
    void toCpp(std::ostream &retVal, const std::string &name) const override;
    void toHeader(std::ostream &retVal, const std::string &name) const override;

    bool operator==(const GatewaySX130xConfig &value) const;

    std::string getUsbPath();
    void setUsbPath(const std::string &value);
};

class GatewayGatewayConfig  : public GatewayJsonConfig {
public:
    gateway_t value;
    std::string serverAddr;
    std::string gpsTtyPath;
    GatewayGatewayConfig();
    void reset();
    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
    void toCpp(std::ostream &retVal, const std::string &name) const override;
    void toHeader(std::ostream &retVal, const std::string &name) const override;

    bool operator==(const GatewayGatewayConfig &value) const;
};

class GatewayDebugConfig  : public GatewayJsonConfig {
public:
    struct lgw_conf_debug_s value;
    GatewayDebugConfig();
    void reset();
    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
    void toCpp(std::ostream &retVal, const std::string &name) const override;
    void toHeader(std::ostream &retVal, const std::string &name) const override;

    bool operator==(const GatewayDebugConfig &value) const;
};

class GatewayConfigFileJson : public GatewaySettings, public GatewayJsonConfig {
public:
    GatewaySX130xConfig sx130xConf;
    GatewayGatewayConfig gatewayConf;
    GatewayDebugConfig debugConf;

    GatewayConfigFileJson();
    ~GatewayConfigFileJson();

    void reset();
    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
    void toCpp(std::ostream &retVal, const std::string &name) const override;
    void toHeader(std::ostream &retVal, const std::string &name) const override;

    std::string toString() const;
    bool operator==(const GatewayConfigFileJson &value) const;
};

#endif
