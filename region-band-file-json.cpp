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
#define ATTRS_REGION_BAND_COUNT	32

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
    JRB_BAND_PRPT_SIZE = 10,	                //          Max payload size
    JRB_BAND_RX1_DR_OFS = 11,        	    //     Array
    JRB_BAND_TX_POWER_OFS = 12,        	    //     Array
    JRB_BAND_UPLINKS = 13,           	    //     Array
    JRB_BAND_DOWNLINKS = 14,           	    //     Array
    JRB_BAND_CHANNEL = 15              	    //          Channel
};

#define JSON_STATE_REGION_BAND_COUNT 17

static const char *PARSE_STATE_NAMES[JSON_STATE_REGION_BAND_COUNT] = {
    "none",
    "root",
    "bands",
    "band",
    "band_defaults",
    "band_data_rates",
    "band_data_rate",
    "band_p_sizes",
    "band_p_size",
    "band_prpt_sizes",
    "band_prpt_size",
    "band_rx1_dr_ofs",
    "band_tx_power_ofs",
    "band_uplinks",
    "band_downlinks",
    "band_channel"
};

enum JSON_KEY_REGION_BAND {
    JK_NONE = 0,
    JK_REGIONALPARAMETERSVERSION = 1,
    JK_REGIONBANDS = 2,

    JK_NAME = 3,
    JK_SUPPORTSEXTRACHANNELS = 4,
    JK_BANDDEFAULTS = 5,
    JK_DATA_RATES = 6,
    JK_UPLINKCHANNELS = 7,
    JK_DOWNLINKCHANNELS = 8,
    JK_MAXPAYLOADSIZEPERDATARATE = 9,
    JK_MAXPAYLOADSIZEPERDATARATEREPEATOR = 10,
    JK_RX1DATARATEOFFSETS = 11,
    JK_TXPOWEROFFSETS = 12,

    JK_RX2FREQUENCY = 13,
    JK_RX2DATARATE = 14,
    JK_RECEIVEDELAY1 = 15,
    JK_RECEIVEDELAY2 = 16,
    JK_JOINACCEPTDELAY1 = 17,
    JK_JOINACCEPTDELAY2 = 18,

    JK_UPLINK = 19,
    JK_DOWNLINK = 20,
    JK_MODULATION = 21,
    JK_BANDWIDTH = 22,
    JK_SPREADINGFACTOR = 23,
    JK_BPS = 24,

    JK_M = 25,
    JK_N = 26,

    JK_FREQUENCY = 27,
    JK_MINDR = 28,
    JK_MAXDR = 29,
    JK_ENABLED = 30,
    JK_CUSTOM = 31

};

static const char *ATTR_REGION_BAND_NAMES[ATTRS_REGION_BAND_COUNT] = {
        "",
        "regionalParametersVersion",         // 1
        "RegionBands",                       // 2

        "name",                              // 3
        "supportsExtraChannels",             // 4
        "bandDefaults",                      // 5
        "dataRates",                         // 6
        "uplinkChannels",                    // 7
        "downlinkChannels",                  // 8
        "maxPayloadSizePerDataRate",         // 9
        "maxPayloadSizePerDataRateRepeater", // 10
        "rx1DataRateOffsets",                // 11
        "txPowerOffsets",                    // 12

        "RX2Frequency",                      // 13
        "RX2DataRate",                       // 14
        "ReceiveDelay1",                     // 15
        "ReceiveDelay2",                     // 16
        "JoinAcceptDelay1",                  // 17
        "JoinAcceptDelay2",                  // 18

        "uplink",                            // 19
        "downlink",                          // 20
        "modulation",                        // 21
        "bandwidth",                         // 22
        "spreadingFactor",                   // 23
        "bps",                               // 24

        "m",                                 // 25
        "n",                                 // 26

        "frequency",                        // 27
        "minDR",                            // 28
        "maxDR",                            // 29
        "enabled",                          // 30
        "custom"                            // 31
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
	: path(""), errcode(0), errMessage("")
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
                    applyErrorDescription("Bool JK_SUPPORTSEXTRACHANNELS");
                    return false;
                case JK_UPLINK:
                    if (state == JRB_BAND_DATA_RATE) {
                        if (!value->storage.bands.empty() && dataRateCount) {
                            value->storage.bands.back().dataRates[dataRateCount - 1].uplink = b;
                            return true;
                        }
                    }
                    applyErrorDescription("Bool JK_UPLINK");
                    return false;
                case JK_DOWNLINK:
                    if (state == JRB_BAND_DATA_RATE) {
                        if (!value->storage.bands.empty() && dataRateCount) {
                            value->storage.bands.back().dataRates[dataRateCount - 1].downlink = b;
                            return true;
                        }
                    }
                    applyErrorDescription("Bool JK_DOWNLINK");
                    return false;
                case JK_ENABLED:
                case JK_CUSTOM: {
                    if (state != JRB_BAND_CHANNEL) {
                        applyErrorDescription("Bool JRB_BAND_CHANNEL no channel");
                        return false;
                    }
                    if (value->storage.bands.empty()) {
                        applyErrorDescription("Bool JRB_BAND_CHANNEL no band");
                        return false;
                    }
                    switch (prevState) {
                        case JRB_BAND_UPLINKS:
                            if (value->storage.bands.back().uplinkChannels.empty()) {
                                applyErrorDescription("Bool JRB_BAND_CHANNEL no band uplink channels");
                                return false;
                            }
                        case JRB_BAND_DOWNLINKS:
                            if (value->storage.bands.back().uplinkChannels.empty()) {
                                applyErrorDescription("Bool JRB_BAND_CHANNEL no band downlink channels");
                                return false;
                            }
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
                    applyErrorDescription("Bool default");
                    return false;
            }
        }

		bool Int(int val) {
            switch (state) {
                case JRB_BAND_TX_POWER_OFS:
                    if (state == JRB_BAND_TX_POWER_OFS) {
                        if (!value->storage.bands.empty()) {
                            value->storage.bands.back().txPowerOffsets[txPowerOffsetCount] = val;
                            txPowerOffsetCount++;
                            return true;
                        }
                    }
            }
            applyErrorDescription("Int default");
            return false;
		}

		bool Uint(unsigned val) {
            switch (state) {
                case JRB_BAND_DEFAULTS:
                    if (value->storage.bands.empty()) {
                        applyErrorDescription("UInt JRB_BAND_DEFAULTS no band");
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
                            applyErrorDescription("UInt JRB_BAND_DEFAULTS default");
                            return false;
                    }
                case JRB_BAND_RX1_DR_OFS:
                    return true;
                case JRB_BAND_CHANNEL: {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription("UInt JRB_BAND_CHANNEL no band");
                        return false;
                    }
                    switch (prevState) {
                        case JRB_BAND_UPLINKS:
                            if (value->storage.bands.back().uplinkChannels.empty()) {
                                applyErrorDescription("UInt JRB_BAND_CHANNEL no band uplink channels");
                                return false;
                            }
                        case JRB_BAND_DOWNLINKS:
                            if (value->storage.bands.back().uplinkChannels.empty()) {
                                applyErrorDescription("UInt JRB_BAND_CHANNEL no band downlink channels");
                                return false;
                            }
                    }
                    Channel &channel = value->storage.bands.back().uplinkChannels.back();

                    switch (keyIndex) {
                        case JK_FREQUENCY:
                            channel.frequency = val;
                            return true;
                        case JK_MINDR:
                            channel.minDR = val;
                            return true;
                        case JK_MAXDR:
                            channel.maxDR = val;
                            return true;
                    }
                    applyErrorDescription("UInt JRB_BAND_CHANNEL");
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
                        applyErrorDescription("UInt default");
                        return false;
                }
            }
		}

		bool Int64(int64_t val) {
            std::cerr << "=== Int64 " << val << ", state: " << state << std::endl;
			return true;
		}

		bool Uint64(uint64_t val) {
            std::cerr << "=== UInt64 " << val << ", state: " << state << std::endl;
			return true;
		}

		bool Double(double d) {
			return true;
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
                    maxPayloadSizePerDataRateCount++;
                    return true;
                case JRB_BAND_PRPT_SIZES:
                    state = JRB_BAND_PRPT_SIZE;
                    maxPayloadSizePerDataRateRepeaterCount++;
                    return true;
                case JRB_BAND_RX1_DR_OFS:
                    rx1DataRateOffsetCount++;
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
                    applyErrorDescription("Object default");
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
                case JRB_BAND_DEFAULTS:
                    state = JRB_BAND;
                    return true;
                case JRB_BAND_DATA_RATE:
                    state = JRB_BAND_DATA_RATES;
                    return true;
                case JRB_BAND_P_SIZE:
                    state = JRB_BAND_P_SIZES;
                    return true;
                case JRB_BAND_PRPT_SIZE:
                    state = JRB_BAND_PRPT_SIZES;
                    return true;
                case JRB_BAND_CHANNEL:
                    state = prevState;
                    return true;
                default:
                    applyErrorDescription("EndObject default");
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
                            applyErrorDescription("Array JRB_ROOT default");
                            return false;
                    }
                case JRB_BAND:                            // Band
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
                            state = JRB_BAND_RX1_DR_OFS;
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
                            applyErrorDescription("Array JRB_BAND default");
                            return false;
                    }
                default:
                    applyErrorDescription("Array default");
                    return false;
            }
		}

		bool EndArray(rapidjson::SizeType elementCount) {
            switch (state) {
                case JRB_BANDS:
                    state = JRB_ROOT;
                    return true;
                case JRB_BAND_DATA_RATES:
                case JRB_BAND_P_SIZES:
                case JRB_BAND_PRPT_SIZES:
                case JRB_BAND_RX1_DR_OFS:
                case JRB_BAND_TX_POWER_OFS:
                case JRB_BAND_UPLINKS:
                case JRB_BAND_DOWNLINKS:
                    state = JRB_BAND;
                    return true;
                default:
                    applyErrorDescription("EndArray default");
                    return false;
            }
		}
};

void RegionBandsFileJson::clear()
{
    storage.bands.clear();
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
