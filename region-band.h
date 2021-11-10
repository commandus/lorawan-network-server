#ifndef REGION_BAND_H_
#define REGION_BAND_H_	1

#include <string>
#include <map>
#include <vector>
#include <inttypes.h>
#include "utillora.h"
#include "lora-radio.h"

// DataRate defines a data rate
class DataRate {
public:
    bool uplink;                        // data-rate can be used for uplink
    bool downlink;                      // data-rate can be used for downlink

    MODULATION modulation;
    BANDWIDTH bandwidth;                // in kHz, used for LoRa
    SPREADING_FACTOR spreadingFactor;   // used for LoRa
    uint32_t bps;       				// FSK bits per second

    DataRate();
    DataRate(const DataRate &value);
    std::string toJsonString();
};

// Channel
class Channel {
public:
    int frequency;  // frequency in Hz
    int minDR;
    int maxDR;
    bool enabled;
    bool custom;    // this channel was configured by the user
    Channel();
    Channel(const Channel &value);
    std::string toJsonString();
};

// BandDefaults defines the default values defined by a band.
class BandDefaults {
public:
    // fixed frequency for the RX2 receive window
    int RX2Frequency;
    // fixed data-rate for the RX2 receive window
    int RX2DataRate;
    // RECEIVE_DELAY1 default value
    int ReceiveDelay1;
    // RECEIVE_DELAY2 default value
    int ReceiveDelay2;
    // JOIN_ACCEPT_DELAY1 default value
    int JoinAcceptDelay1;
    // JOIN_ACCEPT_DELAY2 default value.
    int JoinAcceptDelay2;
    BandDefaults();
    BandDefaults(const BandDefaults &value);
    std::string toJsonString();
};

// MaxPayloadSize defines the max payload size
class MaxPayloadSize {
public:
    uint8_t m;  // The maximum MACPayload size length
    uint8_t n;  // The maximum application payload length in the absence of the optional FOpt control field
    MaxPayloadSize();
    MaxPayloadSize(const MaxPayloadSize &value);
    std::string toJsonString();
};

class RegionBand
{
public:
    std::string name;
    bool supportsExtraChannels;
    BandDefaults bandDefaults;
    DataRate dataRates[DATA_RATE_SIZE];
    MaxPayloadSize maxPayloadSizePerDataRate[DATA_RATE_SIZE];
    MaxPayloadSize maxPayloadSizePerDataRateRepeator[DATA_RATE_SIZE];    // if repeater is used
    uint8_t rx1DataRateOffsets[DATA_RATE_SIZE];
    int8_t txPowerOffsets[DATA_RATE_SIZE];

    std::vector<Channel> uplinkChannels;
    std::vector<Channel> downlinkChannels;

    RegionBand();
    RegionBand(const RegionBand &value);
    std::string toJsonString();
};

class RegionBands
{
public:
    REGIONAL_PARAMETERS_VERSION regionalParametersVersion;  // since specified LoraWAN regional parameters version, if version 0.0.0- any(default) version
    std::map<std::string, RegionBand> values;
    RegionBands();
    RegionBands(const RegionBands &config);
    std::string toJsonString();
};

#endif
