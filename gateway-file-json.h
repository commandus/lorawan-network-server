#ifndef GATEWAY_FILE_JSON_H_
#define GATEWAY_FILE_JSON_H_ 1

#include <vector>
#include "gateway-lora.h"
#include "regional-parameter-channel-plan.h"
#include "regional-parameter-channel-plans.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class GatewayJsonConfig {
public:
    virtual int parse(rapidjson::Value &jsonValue) = 0;
    virtual void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const = 0;
    std::string toString();
    virtual bool set() = 0;
};

/**
{
    "SX130x_conf": {
        "com_type": "USB",
        "com_path": "/dev/ttyACM1",
        "lorawan_public": true,
        "clksrc": 0,
        "antenna_gain": 0,
        "full_duplex": false,
        "fine_timestamp": {
            "enable": false,
            "mode": "all_sf"
        },
        "radio_0": {
            "enable": true,
            "type": "SX1250",
            "freq": 867500000,
            "rssi_offset": -215.4,
            "rssi_tcomp": {"coeff_a": 0, "coeff_b": 0, "coeff_c": 20.41, "coeff_d": 2162.56, "coeff_e": 0},
            "tx_enable": true,
            "tx_freq_min": 863000000,
            "tx_freq_max": 870000000,
            "tx_gain_lut":[
                {"rf_power": 12, "pa_gain": 0, "pwr_idx": 15}
            ]
        },
        "radio_1": {
        },
        "chan_multiSF_All": {"spreading_factor_enable": [ 5, 6, 7, 8, 9, 10, 11, 12 ]},
        "chan_multiSF_0": {"enable": true, "radio": 1, "if": -400000},
        "chan_multiSF_7": {"enable": true, "radio": 0, "if":  400000},
        "chan_Lora_std":  {"enable": true, "radio": 1, "if": -200000, "bandwidth": 250000, "spread_factor": 7,
        "implicit_hdr": false, "implicit_payload_length": 17, "implicit_crc_en": false, "implicit_coderate": 1},
        "chan_FSK":       {"enable": true, "radio": 1, "if":  300000, "bandwidth": 125000, "datarate": 50000}
    },
*/

class GatewaySX130xConfig : public GatewayJsonConfig {
public:
    struct lgw_conf_board_s boardConf ;
    int8_t antennaGain;

    GatewaySX130xConfig();
    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
    bool set() override;
};

/**
    "sx1261_conf": {
        "rssi_offset": 0,
        "spectral_scan": {
            "enable": false,
            "freq_start": 867100000,
            "nb_chan": 8,
            "nb_scan": 2000,
            "pace_s": 10
        },
        "lbt": {
            "enable": false,
            "rssi_target": -70,
            "channels":[
                {"freq_hz": 867100000, "bandwidth": 125000, "scan_time_us": 128,  "transmit_time_ms": 400}
            ]
        }
    }
 */
class GatewaySX1261Config  : public GatewayJsonConfig {
public:
    struct lgw_conf_sx1261_s value;
    spectral_scan_t spectralScan;
    struct lgw_conf_lbt_s lbt;
    GatewaySX1261Config();

    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
    bool set() override;
};

/*
    "gateway_conf": {
        "gateway_ID": "AA555A0000000000",
        "server_address": "84.237.104.128",
        "serv_port_up": 5000,
        "serv_port_down": 5000,
        "keepalive_interval": 10,
        "stat_interval": 30,
        "push_timeout_ms": 100,
        "forward_crc_valid": true,
        "forward_crc_error": false,
        "forward_crc_disabled": false,
        "gps_tty_path": "/dev/ttyS0",
        "ref_latitude": 0.0,
        "ref_longitude": 0.0,
        "ref_altitude": 0,
        "beaconPeriod": 0,
        "beaconFreqHz": 869525000,
        "beaconDataRate": 9,
        "beaconBandwidthHz": 125000,
        "beaconPower": 14,
        "beaconInfoDesc": 0
    }
*/
class GatewayGatewayConfig  : public GatewayJsonConfig {
public:
    uint32_t gatewayId;
    std::string serverAddress;
    uint16_t serverPortUp;
    uint16_t serverPortDown;
    uint32_t keepaliveInterval;
    uint32_t statInterval;
    struct timeval pushTimeoutMs;
    bool forwardCRCValid;
    bool forwardCRCError;
    bool forwardCRCDisabled;
    std::string gpsTTYPath;
    struct coord_s refGeoCoordinates;
    bool fakeGPS;
    uint32_t beaconPeriod;
    uint32_t beaconFreqHz;
    uint8_t beaconFreqNb;
    uint32_t beaconFreqStep;
    uint8_t beaconDataRate;
    uint32_t beaconBandwidthHz;
    uint8_t beaconPower;
    uint8_t beaconInfoDesc;
    uint32_t autoQuitThreshold;

    GatewayGatewayConfig();

    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
    bool set() override;
};

/**
   "debug_conf": {
        "ref_payload":[
            {"id": "0xCAFE1234"},
            {"id": "0xCAFE2345"}
        ],
        "log_file": "loragw_hal.log"
    }
*/
class GatewayDebugConfig  : public GatewayJsonConfig {
public:
    struct lgw_conf_debug_s value;
    GatewayDebugConfig();
    int parse(rapidjson::Value &jsonValue) override;
    void toJSON(rapidjson::Value &jsonValue, rapidjson::Document::AllocatorType& allocator) const override;
};

class GatewayConfigFileJson : public RegionalParameterChannelPlans {
	private:
        // helper hash map to get() region by the name
        std::map<std::string, const RegionalParameterChannelPlan *> nameIndex;
        std::map<int, const RegionalParameterChannelPlan *> idIndex;

        // default RegionalParameterChannelPlan
        // last region marked as default ("defaultRegion": true) is used
        const RegionalParameterChannelPlan *defaultRegionBand;
        int buildIndex();
        int loadFile(const std::string &fileName);
        int saveFile(const std::string &fileName) const;
		int load();
		int save();
	protected:
        std::string path;
		void clear();		

	public:
        RegionBands storage;
        GatewayConfigFileJson();
		~GatewayConfigFileJson();
        int errCode;
        std::string errDescription;

        virtual const RegionalParameterChannelPlan *get(const std::string &name) const override;
        virtual const RegionalParameterChannelPlan *get(int id) const override;

        virtual int init(const std::string &option, void *data) override;
        virtual void flush() override;
        virtual void done() override;

        virtual std::string toJsonString() const override;

        virtual std::string getErrorDescription(int &subCode) const override;
};

#endif
