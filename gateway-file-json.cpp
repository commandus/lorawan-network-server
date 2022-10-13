#include <fstream>
#include <regex>
#include <iostream>
#include <iomanip>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#pragma clang diagnostic pop
 
#include "gateway-file-json.h"
#include "utilstring.h"
#include "errlist.h"

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
        "beaconDatarate": 9,
        "beaconBandwidthHz": 125000,
        "beaconPower": 14,
        "beaconInfodesc": 0
    }
*/

GatewayGatewayConfig::GatewayGatewayConfig()
    : gatewayId(0), serverPortUp(0), serverPortDown(0),
      keepaliveInterval(0), statInterval(0),
      forwardCRCValid(false), forwardCRCError(false), forwardCRCDisabled(false),
      fakeGPS(false),
      beaconPeriod(0), beaconFreqHz(0), beaconFreqNb(0), beaconFreqStep(0), beaconDatarate(0),
      beaconBandwidthHz(0), beaconPower(0), beaconInfodesc(0), autoquitThreshold(0)
{
    pushTimeoutMs.tv_sec = 0;
    pushTimeoutMs.tv_usec = 0;
    refGeoCoordinates.lat = 0.0;
    refGeoCoordinates.lon = 0.0;
    refGeoCoordinates.alt = 0;
}

int GatewayGatewayConfig::parse(rapidjson::Value &jsonValue)
{
    if (jsonValue.HasMember("gateway_ID")) {
        rapidjson::Value &jGatewayId = jsonValue["gateway_ID"];
        if (jGatewayId.IsString()) {
            gatewayId = std::stoull(jGatewayId.GetString(), nullptr, 16);
        }
    }
    if (jsonValue.HasMember("server_address")) {
        rapidjson::Value &jServerAddress = jsonValue["server_address"];
        if (jServerAddress.IsString()) {
            serverAddress = jServerAddress.GetString();
        }
    }
    if (jsonValue.HasMember("serv_port_up")) {
        rapidjson::Value &jServerPortUp = jsonValue["serv_port_up"];
        if (jServerPortUp.IsInt()) {
            gatewayId = jServerPortUp.GetInt();
        }
    }
    if (jsonValue.HasMember("serv_port_down")) {
        rapidjson::Value &jServerPortDown = jsonValue["serv_port_down"];
        if (jServerPortDown.IsInt()) {
            gatewayId = jServerPortDown.GetInt();
        }
    }
    if (jsonValue.HasMember("keepalive_interval")) {
        rapidjson::Value &jKeepaliveInterval = jsonValue["keepalive_interval"];
        if (jKeepaliveInterval.IsInt()) {
            keepaliveInterval = jKeepaliveInterval.GetInt();
        }
    }
    if (jsonValue.HasMember("stat_interval")) {
        rapidjson::Value &jStatInterval = jsonValue["stat_interval"];
        if (jStatInterval.IsInt()) {
            statInterval = jStatInterval.GetInt();
        }
    }
    if (jsonValue.HasMember("push_timeout_ms")) {
        rapidjson::Value &jPushTimeoutMs = jsonValue["push_timeout_ms"];
        if (jPushTimeoutMs.IsInt()) {
            pushTimeoutMs.tv_sec = 0;
            pushTimeoutMs.tv_usec = jPushTimeoutMs.GetInt() * 500;
        }
    }
    if (jsonValue.HasMember("forward_crc_valid")) {
        rapidjson::Value &jForwardCRCValid = jsonValue["forward_crc_valid"];
        if (jForwardCRCValid.IsInt()) {
            forwardCRCValid = jForwardCRCValid.GetBool();
        }
    }
    if (jsonValue.HasMember("forward_crc_error")) {
        rapidjson::Value &jForwardCRCError = jsonValue["forward_crc_error"];
        if (jForwardCRCError.IsInt()) {
            forwardCRCError = jForwardCRCError.GetBool();
        }
    }
    if (jsonValue.HasMember("forward_crc_disabled")) {
        rapidjson::Value &jForwardCRCDisabled = jsonValue["forward_crc_disabled"];
        if (jForwardCRCDisabled.IsInt()) {
            forwardCRCDisabled = jForwardCRCDisabled.GetBool();
        }
    }
    if (jsonValue.HasMember("gps_tty_path")) {
        rapidjson::Value &jGpsTTYPath = jsonValue["gps_tty_path"];
        if (jGpsTTYPath.IsString()) {
            gpsTTYPath = jGpsTTYPath.GetString();
        }
    }
    if (jsonValue.HasMember("ref_latitude")) {
        rapidjson::Value &jRefCoord = jsonValue["ref_latitude"];
        if (jRefCoord.IsDouble()) {
            refGeoCoordinates.lat = jRefCoord.GetDouble();
        }
    }
    if (jsonValue.HasMember("ref_longitude")) {
        rapidjson::Value &jRefCoord = jsonValue["ref_longitude"];
        if (jRefCoord.IsDouble()) {
            refGeoCoordinates.lon = jRefCoord.GetDouble();
        }
    }
    if (jsonValue.HasMember("ref_altitude")) {
        rapidjson::Value &jRefCoord = jsonValue["ref_altitude"];
        if (jRefCoord.IsInt()) {
            refGeoCoordinates.alt = jRefCoord.GetInt();
        }
    }
    if (jsonValue.HasMember("fake_gps")) {
        rapidjson::Value &jForwardCRCDisabled = jsonValue["fake_gps"];
        if (jForwardCRCDisabled.IsBool()) {
            fakeGPS = jForwardCRCDisabled.GetBool();
        }
    }
    if (jsonValue.HasMember("beaconPeriod")) {
        rapidjson::Value &jValue = jsonValue["beaconPeriod"];
        if (jValue.IsInt()) {
            beaconPeriod = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beaconFreqHz")) {
        rapidjson::Value &jValue = jsonValue["beaconFreqHz"];
        if (jValue.IsInt()) {
            beaconFreqHz = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beaconFreqNb")) {
        rapidjson::Value &jValue = jsonValue["beaconFreqNb"];
        if (jValue.IsInt()) {
            beaconFreqNb = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beaconFreqStep")) {
        rapidjson::Value &jValue = jsonValue["beaconFreqStep"];
        if (jValue.IsInt()) {
            beaconFreqStep = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beaconDatarate")) {
        rapidjson::Value &jValue = jsonValue["beaconDatarate"];
        if (jValue.IsInt()) {
            beaconDatarate = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beaconBandwidthHz")) {
        rapidjson::Value &jValue = jsonValue["beaconBandwidthHz"];
        if (jValue.IsInt()) {
            beaconBandwidthHz = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beaconPower")) {
        rapidjson::Value &jValue = jsonValue["beaconPower"];
        if (jValue.IsInt()) {
            beaconPower = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beaconInfodesc")) {
        rapidjson::Value &jValue = jsonValue["beaconInfodesc"];
        if (jValue.IsInt()) {
            beaconInfodesc = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("autoquitThreshold")) {
        rapidjson::Value &jValue = jsonValue["autoquitThreshold"];
        if (jValue.IsInt()) {
            autoquitThreshold = jValue.GetInt();
        }
    }
    return LORA_OK;
}

void GatewayGatewayConfig::toJSON(
    rapidjson::Value &jsonValue,
    rapidjson::Document::AllocatorType& allocator
) const
{
    jsonValue.SetObject();
    rapidjson::Value jgatewayId;
    std::stringstream ssGatewayId;
    ssGatewayId << std::hex << gatewayId;
    jgatewayId.SetString(ssGatewayId.str().c_str(), allocator);
    jsonValue.AddMember("gateway_ID", jgatewayId, allocator);

    rapidjson::Value jserverAddress;
    jserverAddress.SetString(serverAddress.c_str(), allocator);
    jsonValue.AddMember("server_address", jserverAddress, allocator);

    rapidjson::Value jserverPortUp;
    jserverPortUp.SetInt(serverPortUp);
    jsonValue.AddMember("server_port_up", jserverPortUp, allocator);

    rapidjson::Value jserverPortDown;
    jserverPortDown.SetInt(serverPortDown);
    jsonValue.AddMember("server_port_down", jserverPortDown, allocator);

    rapidjson::Value jkeepaliveInterval;
    jkeepaliveInterval.SetInt(keepaliveInterval);
    jsonValue.AddMember("keepalive_interval", jkeepaliveInterval, allocator);

    rapidjson::Value jstatInterval;
    jstatInterval.SetInt(statInterval);
    jsonValue.AddMember("stat_interval", jstatInterval, allocator);

    rapidjson::Value jpushTimeoutMs;
    jpushTimeoutMs.SetInt(pushTimeoutMs.tv_usec / 500);
    jsonValue.AddMember("push_timeout_ms", jpushTimeoutMs, allocator);

    rapidjson::Value jforwardCRCValid;
    jforwardCRCValid.SetBool(forwardCRCValid);
    jsonValue.AddMember("forward_crc_valid", jforwardCRCValid, allocator);

    rapidjson::Value jforwardCRCError;
    jforwardCRCError.SetBool(forwardCRCError);
    jsonValue.AddMember("forward_crc_error", jforwardCRCError, allocator);

    rapidjson::Value jforwardCRCDisabled;
    jforwardCRCDisabled.SetBool(forwardCRCDisabled);
    jsonValue.AddMember("forward_crc_disabled", jforwardCRCDisabled, allocator);

    rapidjson::Value jgpsTTYPath;
    jgpsTTYPath.SetString(gpsTTYPath.c_str(), allocator);
    jsonValue.AddMember("gps_tty_path", jgpsTTYPath, allocator);

    rapidjson::Value jrefGeoCoordinatesLat;
    jrefGeoCoordinatesLat.SetDouble(refGeoCoordinates.lat);
    jsonValue.AddMember("ref_latitude", jrefGeoCoordinatesLat, allocator);

    rapidjson::Value jrefGeoCoordinatesLon;
    jrefGeoCoordinatesLon.SetDouble(refGeoCoordinates.lon);
    jsonValue.AddMember("ref_longitude", jrefGeoCoordinatesLon, allocator);

    rapidjson::Value jrefGeoCoordinatesAlt;
    jrefGeoCoordinatesAlt.SetInt(refGeoCoordinates.alt);
    jsonValue.AddMember("ref_altitude", jrefGeoCoordinatesAlt, allocator);

    rapidjson::Value jfakeGPS;
    jfakeGPS.SetBool(fakeGPS);
    jsonValue.AddMember("fake_gps", jfakeGPS, allocator);

    rapidjson::Value jbeaconPeriod;
    jbeaconPeriod.SetUint(beaconPeriod);
    jsonValue.AddMember("ref_altitude", jbeaconPeriod, allocator);

    rapidjson::Value jbeaconFreqHz;
    jbeaconFreqHz.SetUint(beaconFreqHz);
    jsonValue.AddMember("beacon_freq_hz", jbeaconFreqHz, allocator);

    rapidjson::Value jbeaconFreqNb;
    jbeaconFreqNb.SetUint(beaconFreqNb);
    jsonValue.AddMember("beacon_freq_nb", jbeaconFreqNb, allocator);

    rapidjson::Value jbeaconFreqStep;
    jbeaconFreqStep.SetUint(beaconFreqNb);
    jsonValue.AddMember("beacon_freq_step", jbeaconFreqStep, allocator);

    rapidjson::Value jbeaconDatarate;
    jbeaconDatarate.SetUint(beaconDatarate);
    jsonValue.AddMember("beacon_datarate", jbeaconDatarate, allocator);

    rapidjson::Value jbeaconBandwidthHz;
    jbeaconDatarate.SetUint(beaconBandwidthHz);
    jsonValue.AddMember("beacon_bw_hz", jbeaconBandwidthHz, allocator);

    rapidjson::Value jbeaconPower;
    jbeaconDatarate.SetUint(beaconPower);
    jsonValue.AddMember("beacon_power", jbeaconPower, allocator);

    rapidjson::Value jbeaconInfodesc;
    jbeaconInfodesc.SetUint(beaconInfodesc);
    jsonValue.AddMember("beacon_infodesc", jbeaconInfodesc, allocator);

    rapidjson::Value jautoquitThreshold;
    jbeaconInfodesc.SetUint(autoquitThreshold);
    jsonValue.AddMember("autoquit_threshold", jautoquitThreshold, allocator);
}

std::string GatewayGatewayConfig::toString()
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document doc;
    toJSON(doc, doc.GetAllocator());
    doc.Accept(writer);
    return std::string(buffer.GetString());
}

/**
   "ref_payload":[
        {"id": "0xCAFE1234"},
        {"id": "0xCAFE2345"}
    ],
    "log_file": "loragw_hal.log"
*/
GatewayDebugConfig::GatewayDebugConfig()
{
    memset(&value, 0, sizeof(value));
}

int GatewayDebugConfig::parse(rapidjson::Value &jsonValue)
{
    if (jsonValue.HasMember("log_file")) {
        rapidjson::Value &jLogFileName = jsonValue["log_file"];
        if (jLogFileName.IsString()) {
            std::string s = jLogFileName.GetString();
            size_t sz = s.size();
            if (sz < 128) {
                strncpy(&value.log_file_name[0], s.c_str(), 128);
                value.log_file_name[sz] = 0;
            }
        }
    }

    if (jsonValue.HasMember("ref_payload")) {
        rapidjson::Value &jRefPayloads = jsonValue["ref_payload"];
        if (jRefPayloads.IsArray()) {
            value.nb_ref_payload = jRefPayloads.Size();
            if (value.nb_ref_payload > 16)
                value.nb_ref_payload = 16;
            for (int i = 0; i < value.nb_ref_payload; i++) {
                rapidjson::Value &jrp = jRefPayloads[i];
                if (jrp.IsObject()) {
                    if (jrp.HasMember("id")) {
                        rapidjson::Value &jrpId = jrp["id"];
                        if (jrpId.IsString()) {
                            value.ref_payload[i].id = std::stoull(jrpId.GetString(), nullptr, 0);
                        }
                    }
                }
            }
        }
    }
    return LORA_OK;
}

void GatewayDebugConfig::toJSON(
    rapidjson::Value &jsonValue,
    rapidjson::Document::AllocatorType& allocator
) const
{
    jsonValue.SetObject();
    rapidjson::Value jrefPayloads;
    jrefPayloads.SetArray();
    // identifiers
    for (int i = 0; i < value.nb_ref_payload; i++) {
        rapidjson::Value jrp;
        rapidjson::Value jrpId;
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(8) << value.ref_payload[i].id;
        std::string s = ss.str();
        jrpId.SetString(s.c_str(), s.size(), allocator);
        jrp.AddMember("id", jrpId, allocator);
        jrefPayloads.PushBack(jrp, allocator);
    }
    jsonValue.AddMember("ref_payload", jrefPayloads, allocator);
    // log file name
    std::string lfn(value.log_file_name);
    rapidjson::Value jlfn;
    jlfn.SetString(lfn.c_str(), lfn.size(), allocator);
    jsonValue.AddMember("log_file", jlfn, allocator);
}

std::string GatewayDebugConfig::toString()
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document doc;
    toJSON(doc, doc.GetAllocator());
    doc.Accept(writer);
    return std::string(buffer.GetString());
}

/**
 * 	JSON attribute names
 */
#define ATTRS_GATEWAY_CONFIG_COUNT	39

enum JSON_STATE_GATEWAY_CONFIG {
    JRB_NONE = 0,                  	        // Initial state
    JRB_ROOT = 1,                           //
    JRB_BANDS = 2,                 	        // Array of bands

    JRB_BAND = 3,        	                // Band
    JRB_BAND_DEFAULTS = 4,         	        //     Band defaults
    JRB_BAND_DATA_RATES = 5,                // Array
    JRB_BAND_DATA_RATE = 6,           	    //     Data rate
    JRB_BAND_P_SIZES = 7,               	//     Array
    JRB_BAND_P_SIZE = 8,	                //          Max payload size
    JRB_BAND_PRPT_SIZES = 9,                //     Array
    JRB_BAND_PRPT_SIZE = 10,	            //          Max payload size
    JRB_BAND_RX1_DR_OFFSETS = 11,        	//     Array
    JRB_BAND_RX1_DR_OFFSET = 12,            //              Array
    JRB_BAND_TX_POWER_OFS = 13,        	    //     Array
    JRB_BAND_UPLINKS = 14,           	    //     Array
    JRB_BAND_DOWNLINKS = 15,           	    //     Array
    JRB_BAND_CHANNEL = 16              	    //          Channel
};

#define JSON_STATE_GATEWAY_CONFIG_COUNT 17

static const char *PARSE_STATE_NAMES_GATEWAY_CONFIG[JSON_STATE_GATEWAY_CONFIG_COUNT] = {
    "none",
    "root",
    "RegionBands",
    "RegionBands[]",
    "bandDefaults",
    "band_data_rates",
    "band_data_rate",
    "band_p_sizes",
    "band_p_size",
    "maxPayloadSizePerDataRate",
    "maxPayloadSizePerDataRateRepeater",
    "rx1DataRateOffsets",
    "rx1DataRateOffsets[]",
    "txPowerOffsets",
    "uplinkChannels",
    "downlinkChannels",
    "up/downlinkChannels[]"
};

enum JSON_KEY_GATEWAY_CONFIG {
    JK_NONE = 0,
    JK_REGIONALPARAMETERSVERSION = 1,
    JK_REGIONBANDS = 2,
    JK_ID = 3,
    JK_NAME = 4,

    JK_CN = 5,
    JK_MAXUPLINKEIRP = 6,
    JK_DEFAULTDOWNLINKTXPOWER = 7,
    JK_PINGSLOTFREQUENCY = 8,
    JK_IMPLEMENTSTXPARAMSETUP = 9,

    JK_SUPPORTSEXTRACHANNELS = 10,
    JK_DEFAULT_REGION = 11,
    JK_BANDDEFAULTS = 12,
    JK_DATA_RATES = 13,
    JK_UPLINKCHANNELS = 14,
    JK_DOWNLINKCHANNELS = 15,
    JK_MAXPAYLOADSIZEPERDATARATE = 16,
    JK_MAXPAYLOADSIZEPERDATARATEREPEATOR = 17,
    JK_RX1DATARATEOFFSETS = 18,
    JK_TXPOWEROFFSETS = 19,

    JK_RX2FREQUENCY = 20,
    JK_RX2DATARATE = 21,
    JK_RECEIVEDELAY1 = 22,
    JK_RECEIVEDELAY2 = 23,
    JK_JOINACCEPTDELAY1 = 24,
    JK_JOINACCEPTDELAY2 = 25,

    JK_UPLINK = 26,
    JK_DOWNLINK = 27,
    JK_MODULATION = 28,
    JK_BANDWIDTH = 29,
    JK_SPREADINGFACTOR = 30,
    JK_BPS = 31,

    JK_M = 32,
    JK_N = 33,

    JK_FREQUENCY = 34,
    JK_MINDR = 35,
    JK_MAXDR = 36,
    JK_ENABLED = 37,
    JK_CUSTOM = 38

};

static const char *ATTR_GATEWAY_CONFIG_NAMES[ATTRS_GATEWAY_CONFIG_COUNT] = {
        "",
        "regionalParametersVersion",         // 1
        "RegionBands",                       // 2
        "id",                                // 3
        "name",                              // 4

        "cn",                                // 5
        "maxUplinkEIRP",                     // 6
        "defaultDownlinkTXPower",            // 7
        "pingSlotFrequency",                 // 8
        "implementsTXParamSetup",            // 9

        "supportsExtraChannels",             // 10
        "defaultRegion",                     // 11
        "bandDefaults",                      // 12
        "dataRates",                         // 13
        "uplinkChannels",                    // 14
        "downlinkChannels",                  // 15
        "maxPayloadSizePerDataRate",         // 16
        "maxPayloadSizePerDataRateRepeater", // 17
        "rx1DataRateOffsets",                // 18
        "txPowerOffsets",                    // 19
        "RX2Frequency",                      // 20
        "RX2DataRate",                       // 21
        "ReceiveDelay1",                     // 22
        "ReceiveDelay2",                     // 23
        "JoinAcceptDelay1",                  // 24
        "JoinAcceptDelay2",                  // 25

        "uplink",                            // 26
        "downlink",                          // 27
        "modulation",                        // 28
        "bandwidth",                         // 29
        "spreadingFactor",                   // 30
        "bps",                               // 31

        "m",                                 // 32
        "n",                                 // 33

        "frequency",                         // 34
        "minDR",                             // 35
        "maxDR",                             // 36
        "enabled",                           // 37
        "custom"                             // 38
};

static JSON_KEY_GATEWAY_CONFIG getAttrByName(
        const char *name
)
{
    for (int i = 1; i < ATTRS_GATEWAY_CONFIG_COUNT; i++) {
        if (strcmp(ATTR_GATEWAY_CONFIG_NAMES[i], name) == 0)
            return (JSON_KEY_GATEWAY_CONFIG) i;
    }
    return JK_NONE;
}

GatewayConfigFileJson::GatewayConfigFileJson()
	: errCode(0), defaultRegionBand(nullptr)
{

}

GatewayConfigFileJson::~GatewayConfigFileJson()
{
	done();
}

/**
 *
 * Loads NetworkIdentities
 *	[
 *		{
 *	 		"addr": "network address (hex string, 4 bytes)"
 * 			"eui": "device identifier (hex string, 8 bytes)",
 * 			"nwkSKey": "shared session key (hex string, 16 bytes)",
 *			"appSKey": "private key (hex string, 16 bytes)"
 *		},
 *		..
 *	]
 */
class GatewayConfigJsonHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, GatewayConfigJsonHandler> {
private:
    GatewayConfigFileJson *value;
    JSON_KEY_GATEWAY_CONFIG keyIndex;
    JSON_STATE_GATEWAY_CONFIG state;
    JSON_STATE_GATEWAY_CONFIG prevState;

    int bandCount, dataRateCount, maxPayloadSizePerDataRateCount,
            maxPayloadSizePerDataRateRepeaterCount,
            rx1DataRateOffsetCount, txPowerOffsetCount,
            uplinkChannelCount, downlinkChannelCount;

    bool Integer(int64_t val) {
        switch (state) {
            case JRB_BAND_TX_POWER_OFS:
                if (state == JRB_BAND_TX_POWER_OFS) {
                    if (!value->storage.bands.empty()) {
                        if (txPowerOffsetCount >= TX_POWER_OFFSET_MAX_SIZE) {
                            applyErrorDescription("Too many tx offsets");
                            return false;
                        }
                        RegionalParameterChannelPlan &rb(value->storage.bands.back());
                        rb.txPowerOffsets.push_back((int8_t) val);
                        txPowerOffsetCount++;
                        return true;
                    }
                }
                return false;
            case JRB_BAND_DEFAULTS:
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                switch (keyIndex) {
                    case JK_RX2FREQUENCY:
                        value->storage.bands.back().bandDefaults.RX2Frequency = (int) val;
                        return true;
                    case JK_RX2DATARATE:
                        value->storage.bands.back().bandDefaults.RX2DataRate = (int) val;
                        return true;
                    case JK_RECEIVEDELAY1:
                        value->storage.bands.back().bandDefaults.ReceiveDelay1 = (int) val;
                        return true;
                    case JK_RECEIVEDELAY2:
                        value->storage.bands.back().bandDefaults.ReceiveDelay2 = val;
                        return true;
                    case JK_JOINACCEPTDELAY1:
                        value->storage.bands.back().bandDefaults.JoinAcceptDelay1 = val;
                        return true;
                    case JK_JOINACCEPTDELAY2:
                        value->storage.bands.back().bandDefaults.JoinAcceptDelay2 = val;
                        return true;
                    default:
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                }
            case JRB_BAND_RX1_DR_OFFSET: {
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                if (rx1DataRateOffsetCount >= DATA_RATE_SIZE) {
                    applyErrorDescription("rx1DataRateOffset more than 8 elements");
                    return false;
                }

                RegionalParameterChannelPlan &rb = value->storage.bands.back();
                rb.rx1DataRateOffsets[rx1DataRateOffsetCount].push_back(val);
                return true;
            }
            case JRB_BAND_CHANNEL: {
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                Channel *channel;
                switch (prevState) {
                    case JRB_BAND_UPLINKS:
                        if (value->storage.bands.back().uplinkChannels.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        channel = &value->storage.bands.back().uplinkChannels.back();
                        break;
                    case JRB_BAND_DOWNLINKS:
                        if (value->storage.bands.back().downlinkChannels.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        channel = &value->storage.bands.back().downlinkChannels.back();
                        break;
                    default:
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                }

                switch (keyIndex) {
                    case JK_FREQUENCY:
                        channel->frequency = (int) val;
                        return true;
                    case JK_MINDR:
                        channel->minDR = val;
                        return true;
                    case JK_MAXDR:
                        channel->maxDR = val;
                        return true;
                    default:
                        break;
                }
                applyErrorDescription(ERR_REGION_BAND_EMPTY);
                return false;
            }
            case JRB_BAND_P_SIZE: {
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                if (maxPayloadSizePerDataRateCount >= DATA_RATE_SIZE) {
                    applyErrorDescription("UInt maxPayloadSizePerDataRate more than 8 elements");
                    return false;
                }
                switch (keyIndex) {
                    case JK_M:
                        value->storage.bands.back().maxPayloadSizePerDataRate[maxPayloadSizePerDataRateCount].m = (uint8_t) val;
                        return true;
                    case JK_N:
                        value->storage.bands.back().maxPayloadSizePerDataRate[maxPayloadSizePerDataRateCount].n = (uint8_t) val;
                        return true;
                }
                applyErrorDescription("Unexpected number in the maxPayloadSizePerDataRate");
                return false;
            }
            case JRB_BAND_PRPT_SIZE: {
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                if (maxPayloadSizePerDataRateRepeaterCount  >= DATA_RATE_SIZE) {
                    applyErrorDescription("maxPayloadSizePerDataRateRepeater has more than 8 elements");
                    return false;
                }
                switch (keyIndex) {
                    case JK_M:
                        value->storage.bands.back().maxPayloadSizePerDataRateRepeater[maxPayloadSizePerDataRateRepeaterCount].m = val;
                        return true;
                    case JK_N:
                        value->storage.bands.back().maxPayloadSizePerDataRateRepeater[maxPayloadSizePerDataRateRepeaterCount].n = val;
                        return true;
                }
                applyErrorDescription(ERR_REGION_BAND_EMPTY);
                return false;
            }
            case JRB_BAND_DATA_RATE: {
                switch (keyIndex) {
                    case JK_BANDWIDTH:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        if (dataRateCount > DATA_RATE_SIZE) {
                            applyErrorDescription("dataRates array size bigger than 8 elements");
                            return false;
                        }
                        value->storage.bands.back().dataRates[dataRateCount - 1].bandwidth = int2BANDWIDTH(val);
                        return true;
                    case JK_SPREADINGFACTOR:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        if (dataRateCount > DATA_RATE_SIZE) {
                            applyErrorDescription("dataRates array size bigger than 8 elements");
                            return false;
                        }
                        value->storage.bands.back().dataRates[dataRateCount - 1].spreadingFactor = (SPREADING_FACTOR) val;
                        return true;
                    case JK_BPS:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        if (dataRateCount > DATA_RATE_SIZE) {
                            applyErrorDescription("dataRates array size bigger than 8 elements");
                            return false;
                        }
                        value->storage.bands.back().dataRates[dataRateCount - 1].bps = val;
                        return true;
                }
                applyErrorDescription("Unexpected integer");
                return false;
            }
            case JRB_BAND:
                switch (keyIndex) {
                    case JK_DEFAULTDOWNLINKTXPOWER:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().defaultDownlinkTXPower = val;
                        return true;
                    case JK_PINGSLOTFREQUENCY:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().pingSlotFrequency = val;
                        return true;
                    case JK_MAXUPLINKEIRP:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().maxUplinkEIRP = (float) val;
                        return true;
                    case JK_ID:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().id = val;
                        return true;
                }
                applyErrorDescription("Unexpected integer value");
                return false;
            default:
                applyErrorDescription("Unexpected integer value");
                return false;
        }
    }

    void applyErrorDescription(const std::string &reason) {
        std::stringstream ss;
        ss << "reason: " << reason << ", key: " << ATTR_GATEWAY_CONFIG_NAMES[keyIndex] << "(" << keyIndex << ")"
           << ", state: " << PARSE_STATE_NAMES_GATEWAY_CONFIG[state];
        value->errDescription = ss.str();
    }


public:
    explicit GatewayConfigJsonHandler(GatewayConfigFileJson *val)
            : value(val), keyIndex(JK_NONE), state(JRB_NONE), prevState(JRB_NONE),
              bandCount(0), dataRateCount(0), maxPayloadSizePerDataRateCount(0),
              maxPayloadSizePerDataRateRepeaterCount(0),
              rx1DataRateOffsetCount(0), txPowerOffsetCount(0),
              uplinkChannelCount(0), downlinkChannelCount(0)
    {
    }

    bool Bool(bool b) {
        switch (keyIndex) {
            case JK_SUPPORTSEXTRACHANNELS:
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                value->storage.bands.back().supportsExtraChannels = b;
                return true;
            case JK_IMPLEMENTSTXPARAMSETUP:
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                value->storage.bands.back().implementsTXParamSetup = b;
                return true;
            case JK_DEFAULT_REGION:
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                value->storage.bands.back().defaultRegion = b;
                return true;
            case JK_UPLINK:
            case JK_DOWNLINK:
                if (state == JRB_BAND_DATA_RATE) {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                    }
                    if (dataRateCount > DATA_RATE_SIZE) {
                        applyErrorDescription("dataRates array size bigger than 8 elements");
                        return false;
                    }
                    if (keyIndex == JK_UPLINK)
                        value->storage.bands.back().dataRates[dataRateCount - 1].uplink = b;
                    else
                        value->storage.bands.back().dataRates[dataRateCount - 1].downlink = b;
                    return true;
                }
                applyErrorDescription("Unexpected boolean");
                return false;
            case JK_ENABLED:
            case JK_CUSTOM: {
                if (state != JRB_BAND_CHANNEL || value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                switch (prevState) {
                    case JRB_BAND_UPLINKS:
                        if (value->storage.bands.back().uplinkChannels.empty()) {
                            applyErrorDescription("uplinkChannels array element disappeared ");
                            return false;
                        }
                        break;
                    case JRB_BAND_DOWNLINKS:
                        if (value->storage.bands.back().downlinkChannels.empty()) {
                            applyErrorDescription("downlinkChannels array element disappeared");
                            return false;
                        }
                        break;
                }
                Channel &channel = value->storage.bands.back().uplinkChannels.back();

                switch (keyIndex) {
                    case JK_ENABLED:
                        channel.enabled = b;
                        break;
                    case JK_CUSTOM:
                        channel.custom = b;
                        break;
                    default:
                            break;
                }
                return true;
            }
            default:
                applyErrorDescription("Unexpected boolean value");
                return false;
        }
    }

    bool Int(int val) {
        return Integer(val);
    }

    bool Uint(unsigned val) {
        return Integer(val);
    }

    bool Int64(int64_t val) {
        applyErrorDescription("Unexpected too big Int64 number");
        return false;
    }

    bool Uint64(uint64_t val) {
        applyErrorDescription("Unexpected too big UInt64 number");
        return false;
    }

    bool Double(double d) {
        switch(state) {
            case JRB_BAND:
                switch (keyIndex) {
                    case JK_MAXUPLINKEIRP:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().maxUplinkEIRP = (float) d;
                        return true;
                    default:
                        break;
                }
                return false;
            case JRB_BAND_DATA_RATE: {
                switch (keyIndex) {
                    case JK_BANDWIDTH:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        if (dataRateCount > DATA_RATE_SIZE) {
                            applyErrorDescription("dataRates array size bigger than 8 elements");
                            return false;
                        }
                        value->storage.bands.back().dataRates[dataRateCount - 1].bandwidth = double2BANDWIDTH(d);
                        return true;
                }
            }
        }
        applyErrorDescription("Unexpected float number");
        return false;
    }

    bool String(const char* str, rapidjson::SizeType length, bool copy) {
        std::string s(str, length);
        switch(keyIndex) {
            case JK_REGIONALPARAMETERSVERSION:
                if (state == JRB_ROOT) {
                    value->storage.regionalParametersVersion = string2REGIONAL_PARAMETERS_VERSION(s);
                    return true;
                }
                applyErrorDescription("String JK_REGIONALPARAMETERSVERSION");
                return false;
            case JK_NAME:
                if (state == JRB_BAND) {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                    }
                    value->storage.bands.back().name = s;
                    return true;
                }
                applyErrorDescription("String JK_NAME");
                return false;
            case JK_CN:
                if (state == JRB_BAND) {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                    }
                    value->storage.bands.back().cn = s;
                    return true;
                }
                applyErrorDescription("String JK_CN");
                return false;
            case JK_MODULATION:
                if (state == JRB_BAND_DATA_RATE) {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                    }
                    if (dataRateCount > DATA_RATE_SIZE) {
                        applyErrorDescription("dataRates array size bigger than 8 elements");
                        return false;
                    }
                    value->storage.bands.back().dataRates[dataRateCount - 1].modulation = string2MODULATION(s.c_str());
                    return true;
                }
                applyErrorDescription("Unexpected boolean");
                return false;
            default:
                return false;
        }
    }

    bool StartObject() {
        switch (state) {
            case JRB_NONE:
                state = JRB_ROOT;
                return true;
            case JRB_BANDS:
                state = JRB_BAND;
                {
                    RegionalParameterChannelPlan b;
                    value->storage.bands.push_back(b);
                }
                bandCount++;
                return true;
            case JRB_BAND:
                dataRateCount = 0;
                maxPayloadSizePerDataRateCount = 0;
                maxPayloadSizePerDataRateRepeaterCount = 0;
                rx1DataRateOffsetCount = 0;
                txPowerOffsetCount = 0;
                uplinkChannelCount = 0;
                downlinkChannelCount = 0;
                switch(keyIndex) {
                    case JK_BANDDEFAULTS:
                        state = JRB_BAND_DEFAULTS;
                        return true;
                }
                applyErrorDescription("Object JRB_BAND JK_BANDDEFAULTS");
                return false;
            case JRB_BAND_DATA_RATES:
                state = JRB_BAND_DATA_RATE;
                dataRateCount++;
                return true;
            case JRB_BAND_P_SIZES:
                state = JRB_BAND_P_SIZE;
                return true;
            case JRB_BAND_PRPT_SIZES:
                state = JRB_BAND_PRPT_SIZE;
                return true;
            case JRB_BAND_UPLINKS:
                state = JRB_BAND_CHANNEL;
                if (!value->storage.bands.empty()) {
                    Channel ch;
                    value->storage.bands.back().uplinkChannels.push_back(ch);
                    uplinkChannelCount++;
                    return true;
                }
                applyErrorDescription("Object JRB_BAND_UPLINKS");
                return false;
            case JRB_BAND_DOWNLINKS:
                state = JRB_BAND_CHANNEL;
                if (!value->storage.bands.empty()) {
                    Channel ch;
                    value->storage.bands.back().downlinkChannels.push_back(ch);
                    downlinkChannelCount++;
                    return true;
                }
                applyErrorDescription("Object JRB_BAND_DOWNLINKS");
                return false;
            default:
                applyErrorDescription("Unexpected object");
                return false;
        }
    }

    bool Key(const char* str, rapidjson::SizeType length, bool copy) {
        keyIndex = getAttrByName(str);
        bool ok = keyIndex != JK_NONE;
        if (!ok) {
            applyErrorDescription("Unknown attribute " + std::string(str, length));
        }
        return ok;
    }

    bool EndObject(rapidjson::SizeType memberCount)
    {
        switch (state) {
            case JRB_BAND:
                state = JRB_BANDS;
                return true;
            case JRB_ROOT:
                state = JRB_NONE;
                return true;
            case JRB_BAND_DEFAULTS:
                state = JRB_BAND;
                return true;
            case JRB_BAND_DATA_RATE:
                state = JRB_BAND_DATA_RATES;
                return true;
            case JRB_BAND_P_SIZE:
                state = JRB_BAND_P_SIZES;
                maxPayloadSizePerDataRateCount++;
                return true;
            case JRB_BAND_PRPT_SIZE:
                state = JRB_BAND_PRPT_SIZES;
                maxPayloadSizePerDataRateRepeaterCount++;
                return true;
            case JRB_BAND_CHANNEL:
                state = prevState;
                return true;
            default:
                applyErrorDescription("Unexpected end of object");
                return false;
        }
    }

    bool StartArray() {
        switch (state) {
            case JRB_ROOT:
                switch (keyIndex) {
                    case JK_REGIONBANDS:
                        state = JRB_BANDS;
                        bandCount = 0;
                        return true;
                    default:
                        applyErrorDescription("Unexpected array");
                        return false;
                }
            case JRB_BAND:
                switch (keyIndex) {
                    case JK_DATA_RATES:
                        state = JRB_BAND_DATA_RATES;
                        return true;
                    case JK_MAXPAYLOADSIZEPERDATARATE:
                        state = JRB_BAND_P_SIZES;
                        return true;
                    case JK_MAXPAYLOADSIZEPERDATARATEREPEATOR:
                        state = JRB_BAND_PRPT_SIZES;
                        return true;
                    case JK_RX1DATARATEOFFSETS:
                        state = JRB_BAND_RX1_DR_OFFSETS;
                        return true;
                    case JK_TXPOWEROFFSETS:
                        state = JRB_BAND_TX_POWER_OFS;
                        return true;
                    case JK_UPLINKCHANNELS:
                        prevState = JRB_BAND_UPLINKS;
                        state = JRB_BAND_UPLINKS;
                        return true;
                    case JK_DOWNLINKCHANNELS:
                        prevState = JRB_BAND_DOWNLINKS;
                        state = JRB_BAND_DOWNLINKS;
                        return true;
                    default:
                        applyErrorDescription("Unexpected array in the band");
                        return false;
                }
            case JRB_BAND_RX1_DR_OFFSETS:
                state = JRB_BAND_RX1_DR_OFFSET;
                return true;
            default:
                applyErrorDescription("Unexpected array");
                return false;
        }
    }

    bool EndArray(rapidjson::SizeType elementCount) {
        switch (state) {
            case JRB_ROOT:
                return true;
            case JRB_BAND:
                state = JRB_BANDS;
                return true;
            case JRB_BANDS:
                state = JRB_ROOT;
                return true;
            case JRB_BAND_DATA_RATES:
            case JRB_BAND_P_SIZES:
            case JRB_BAND_PRPT_SIZES:
            case JRB_BAND_RX1_DR_OFFSETS:
            case JRB_BAND_TX_POWER_OFS:
            case JRB_BAND_UPLINKS:
            case JRB_BAND_DOWNLINKS:
                state = JRB_BAND;
                return true;
            case JRB_BAND_RX1_DR_OFFSET:
                state = JRB_BAND_RX1_DR_OFFSETS;
                rx1DataRateOffsetCount++;
                return true;
            default:
                applyErrorDescription("Unexpected end of array");
                return false;
        }
    }
};

void GatewayConfigFileJson::clear()
{
    storage.bands.clear();
    nameIndex.clear();
    defaultRegionBand = nullptr;
}

/**
 * @return ERR_CODE_REGION_BAND_EMPTY if not loaded any
 */
int GatewayConfigFileJson::buildIndex()
{
    nameIndex.clear();
    idIndex.clear();
    defaultRegionBand = nullptr;
    for (std::vector<RegionalParameterChannelPlan>::const_iterator it(storage.bands.begin()); it != storage.bands.end(); it++) {
        nameIndex[it->name] = &*it;
        idIndex[it->id] = &*it;
        if (it->defaultRegion)
            defaultRegionBand = &*it;
    }
    // Assign default regional settings
    if (!defaultRegionBand) {
        if (storage.bands.empty()) {
            // TODO add dumb region?
        } else {
            // use the first one
            defaultRegionBand = &*storage.bands.begin();
        }
    }
    return storage.bands.empty() ? ERR_CODE_INIT_REGION_NO_DEFAULT : LORA_OK;
}

const RegionalParameterChannelPlan *GatewayConfigFileJson::get(const std::string &name) const
{
    std::map<std::string, const RegionalParameterChannelPlan*>::const_iterator it(nameIndex.find(name));
    if (it == nameIndex.end())
        return defaultRegionBand;
    return it->second;
}

const RegionalParameterChannelPlan *GatewayConfigFileJson::get(int id) const
{
    std::map<int, const RegionalParameterChannelPlan*>::const_iterator it(idIndex.find(id));
    if (it == idIndex.end())
        return defaultRegionBand;
    return it->second;
}

int GatewayConfigFileJson::loadFile(const std::string &fileName)
{
    GatewayConfigJsonHandler handler(this);
    rapidjson::Reader reader;
    FILE* fp = fopen(fileName.c_str(), "rb");
    if (!fp)
        return ERR_CODE_INVALID_JSON;
    char readBuffer[4096];
    rapidjson::FileReadStream istrm(fp, readBuffer, sizeof(readBuffer));
    rapidjson::ParseResult r = reader.Parse<rapidjson::kParseCommentsFlag>(istrm, handler);
    if (r.IsError()) {
        errCode = r.Code();
        std::stringstream ss;
        ss << rapidjson::GetParseError_En(r.Code()) << " Offset: " << r.Offset()
           << ",  " << errDescription;
        errDescription = ss.str();
    } else {
        errCode = 0;
        errDescription = "";
    }
    fclose(fp);
    return r.IsError() ? ERR_CODE_INVALID_JSON : 0;
}

int GatewayConfigFileJson::load()
{
	clear();
    int r = loadFile(path);
    if (!r) {
        r = buildIndex();
    }
    return r;
}

int GatewayConfigFileJson::saveFile(const std::string &fileName) const
{
    std::fstream os;
    os.open(fileName.c_str(), std::ios::out);
    os << storage.toJsonString();
    int r = os.bad() ? ERR_CODE_OPEN_DEVICE : 0;
    os.close();
    return r;
}

int GatewayConfigFileJson::save()
{
    int r = saveFile(path);
	return r;
}

int GatewayConfigFileJson::init(
	const std::string &option, 
	void *data
)
{
	path = option;
	return load();
}

void GatewayConfigFileJson::flush()
{
	save();
}

void GatewayConfigFileJson::done()
{

}

std::string GatewayConfigFileJson::toJsonString() const {
    return storage.toJsonString();
}

std::string GatewayConfigFileJson::getErrorDescription(int &subCode) const
{
    subCode = errCode;
    return errDescription;
}
