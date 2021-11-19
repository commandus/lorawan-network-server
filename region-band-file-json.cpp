#include <fstream>
#include <regex>
#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#pragma clang diagnostic pop
 
#include "region-band-file-json.h"
#include "utilstring.h"
#include "errlist.h"

/**
 * 	JSON attribute names
 */
#define ATTRS_REGION_BAND_COUNT	33

enum JSON_STATE_REGION_BAND {
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

#define JSON_STATE_REGION_BAND_COUNT 17

static const char *PARSE_STATE_NAMES[JSON_STATE_REGION_BAND_COUNT] = {
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

enum JSON_KEY_REGION_BAND {
    JK_NONE = 0,
    JK_REGIONALPARAMETERSVERSION = 1,
    JK_REGIONBANDS = 2,

    JK_NAME = 3,
    JK_SUPPORTSEXTRACHANNELS = 4,
    JK_DEFAULT_REGION = 5,
    JK_BANDDEFAULTS = 6,
    JK_DATA_RATES = 7,
    JK_UPLINKCHANNELS = 8,
    JK_DOWNLINKCHANNELS = 9,
    JK_MAXPAYLOADSIZEPERDATARATE = 10,
    JK_MAXPAYLOADSIZEPERDATARATEREPEATOR = 11,
    JK_RX1DATARATEOFFSETS = 12,
    JK_TXPOWEROFFSETS = 13,

    JK_RX2FREQUENCY = 14,
    JK_RX2DATARATE = 15,
    JK_RECEIVEDELAY1 = 16,
    JK_RECEIVEDELAY2 = 17,
    JK_JOINACCEPTDELAY1 = 18,
    JK_JOINACCEPTDELAY2 = 19,

    JK_UPLINK = 20,
    JK_DOWNLINK = 21,
    JK_MODULATION = 22,
    JK_BANDWIDTH = 23,
    JK_SPREADINGFACTOR = 24,
    JK_BPS = 25,

    JK_M = 26,
    JK_N = 27,

    JK_FREQUENCY = 28,
    JK_MINDR = 29,
    JK_MAXDR = 30,
    JK_ENABLED = 31,
    JK_CUSTOM = 32

};

static const char *ATTR_REGION_BAND_NAMES[ATTRS_REGION_BAND_COUNT] = {
        "",
        "regionalParametersVersion",         // 1
        "RegionBands",                       // 2

        "name",                              // 3
        "supportsExtraChannels",             // 4
        "defaultRegion",                     // 5
        "bandDefaults",                      // 6
        "dataRates",                         // 7
        "uplinkChannels",                    // 8
        "downlinkChannels",                  // 9
        "maxPayloadSizePerDataRate",         // 10
        "maxPayloadSizePerDataRateRepeater", // 11
        "rx1DataRateOffsets",                // 12
        "txPowerOffsets",                    // 13

        "RX2Frequency",                      // 14
        "RX2DataRate",                       // 15
        "ReceiveDelay1",                     // 16
        "ReceiveDelay2",                     // 17
        "JoinAcceptDelay1",                  // 18
        "JoinAcceptDelay2",                  // 19

        "uplink",                            // 20
        "downlink",                          // 21
        "modulation",                        // 22
        "bandwidth",                         // 23
        "spreadingFactor",                   // 24
        "bps",                               // 25

        "m",                                 // 26
        "n",                                 // 27

        "frequency",                        // 28
        "minDR",                            // 29
        "maxDR",                            // 30
        "enabled",                          // 31
        "custom"                            // 32
};

static JSON_KEY_REGION_BAND getAttrByName(
        const char *name
)
{
    for (int i = 1; i < ATTRS_REGION_BAND_COUNT; i++) {
        if (strcmp(ATTR_REGION_BAND_NAMES[i], name) == 0)
            return (JSON_KEY_REGION_BAND) i;
    }
    return JK_NONE;
}

RegionBandsFileJson::RegionBandsFileJson() 
	: path(""), errcode(0), errMessage(""), defaultRegionBand(nullptr)
{

}

RegionBandsFileJson::~RegionBandsFileJson() 
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
class RegionBandsJsonHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, RegionBandsJsonHandler> {
	private:
        RegionBandsFileJson *value;
        JSON_KEY_REGION_BAND keyIndex;
        JSON_STATE_REGION_BAND state;
        JSON_STATE_REGION_BAND prevState;

        int bandCount, dataRateCount, maxPayloadSizePerDataRateCount,
            maxPayloadSizePerDataRateRepeaterCount,
            rx1DataRateOffsetCount, txPowerOffsetCount,
            uplinkChannelCount, downlinkChannelCount;

        void applyErrorDescription(const std::string &reason) {
            std::stringstream ss;
            ss << "reason: " << reason << ", key: " << ATTR_REGION_BAND_NAMES[keyIndex] << "(" << keyIndex << ")"
                << ", state: " << PARSE_STATE_NAMES[state];
            value->errMessage = ss.str();
        }

        bool Integer(int64_t val) {
            switch (state) {
                case JRB_BAND_TX_POWER_OFS:
                    if (state == JRB_BAND_TX_POWER_OFS) {
                        if (!value->storage.bands.empty()) {
                            value->storage.bands.back().txPowerOffsets[txPowerOffsetCount] = val;
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
                            value->storage.bands.back().bandDefaults.RX2Frequency = val;
                            return true;
                        case JK_RX2DATARATE:
                            value->storage.bands.back().bandDefaults.RX2DataRate = val;
                            return true;
                        case JK_RECEIVEDELAY1:
                            value->storage.bands.back().bandDefaults.ReceiveDelay1 = val;
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

                    RegionBand &rb = value->storage.bands.back();
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
                            channel->frequency = val;
                            return true;
                        case JK_MINDR:
                            channel->minDR = val;
                            return true;
                        case JK_MAXDR:
                            channel->maxDR = val;
                            return true;
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
                            value->storage.bands.back().maxPayloadSizePerDataRate[maxPayloadSizePerDataRateCount].m = val;
                            return true;
                        case JK_N:
                            value->storage.bands.back().maxPayloadSizePerDataRate[maxPayloadSizePerDataRateCount].n = val;
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
                default:
                    switch (keyIndex) {
                        case JK_RX2FREQUENCY:
                            return true;
                        case JK_RX2DATARATE:
                            return true;
                        case JK_RECEIVEDELAY1:
                            return true;
                        case JK_RECEIVEDELAY2:
                            return true;
                        case JK_JOINACCEPTDELAY1:
                            return true;
                        case JK_JOINACCEPTDELAY2:
                            return true;
                        default:
                            applyErrorDescription("Unexpected integer value");
                            return false;
                    }
            }
        }

    public:
        RegionBandsJsonHandler(RegionBandsFileJson *val)
			: value(val), keyIndex(JK_NONE), state(JRB_NONE), prevState(JRB_NONE),
            bandCount(0), dataRateCount(0), maxPayloadSizePerDataRateCount(0),
            maxPayloadSizePerDataRateRepeaterCount(0),
            rx1DataRateOffsetCount(0), txPowerOffsetCount(0),
            uplinkChannelCount(0), downlinkChannelCount(0)
		{
		}

		bool Null() {
			return true; 
		}

		bool Bool(bool b) {
            switch (keyIndex) {
                case JK_SUPPORTSEXTRACHANNELS:
                    if (state == JRB_BAND) {
                        if (!value->storage.bands.empty()) {
                            value->storage.bands.back().supportsExtraChannels = b;
                            return true;
                        }
                    }
                    applyErrorDescription("Unexpected boolean");
                    return false;
                case JK_DEFAULT_REGION:
                    if (state == JRB_BAND) {
                        if (!value->storage.bands.empty()) {
                            value->storage.bands.back().defaultRegion = b;
                            return true;
                        }
                    }
                    applyErrorDescription("Unexpected boolean");
                    return false;
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
            std::cerr << "=== Int64 " << val << ", state: " << state << std::endl;
            applyErrorDescription("Unexpected too big Int64 number");
			return false;
		}

		bool Uint64(uint64_t val) {
            applyErrorDescription("Unexpected too big UInt64 number");
			return false;
		}

		bool Double(double d) {
            switch(state) {
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
                        if (!value->storage.bands.empty()) {
                            value->storage.bands.back().name = s;
                            return true;
                        }
                    }
                    applyErrorDescription("String JK_NAME");
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
                        RegionBand b;
                        value->storage.bands.push_back(b);
                    }
                    bandCount++;
                    return true;
                case JRB_BAND:
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
                    state = JRB_ROOT;
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

void RegionBandsFileJson::clear()
{
    storage.bands.clear();
    nameIndex.clear();
    defaultRegionBand = nullptr;
}

void RegionBandsFileJson::buildIndex()
{
    nameIndex.clear();
    defaultRegionBand = nullptr;
    for (std::vector<RegionBand>::const_iterator it(storage.bands.begin()); it != storage.bands.end(); it++) {
        nameIndex[it->name] = &*it;
        if (it->defaultRegion)
            defaultRegionBand = &*it;
    }

}

const RegionBand *RegionBandsFileJson::get(const std::string &name) const
{
    std::map<std::string, const RegionBand*>::const_iterator it(nameIndex.find(name));
    if (it == nameIndex.end())
        return defaultRegionBand;
    return it->second;
}

int RegionBandsFileJson::load()
{
	clear();
    RegionBandsJsonHandler handler(this);
    rapidjson::Reader reader;
	FILE* fp = fopen(path.c_str(), "rb");
	if (!fp)
		return ERR_CODE_INVALID_JSON;
 	char readBuffer[4096];
	rapidjson::FileReadStream istrm(fp, readBuffer, sizeof(readBuffer));
    rapidjson::ParseResult r = reader.Parse(istrm, handler);
	if (r.IsError()) {
		errcode = r.Code();
		std::stringstream ss;
		ss << rapidjson::GetParseError_En(r.Code()) << " Offset: " << r.Offset()
            << ",  " << errMessage;
        errMessage = ss.str();
	} else {
		errcode = 0;
        errMessage = "";
	}
	fclose(fp);
    buildIndex();
	return r.IsError() ? ERR_CODE_INVALID_JSON : 0;
} 

int RegionBandsFileJson::save()
{
	std::fstream os;
	os.open(path.c_str(), std::ios::out);
	os << storage.toJsonString();
	int r = os.bad() ? ERR_CODE_OPEN_DEVICE : 0;
	os.close();
	return r;
}

int RegionBandsFileJson::init(
	const std::string &option, 
	void *data
)
{
	path = option;
	return load();
}

void RegionBandsFileJson::flush()
{
	save();
}

void RegionBandsFileJson::done()
{

}
