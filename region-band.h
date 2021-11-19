#ifndef REGION_BAND_H_
#define REGION_BAND_H_	1

#include <string>
#include <vector>
#include <inttypes.h>
#include "utillora.h"
#include "lora-radio.h"

class RBJsonIntf {
public:
    virtual std::string toJsonString() const = 0;
};

// DataRate defines a data rate
class DataRate : public RBJsonIntf {
public:
    bool uplink;                        // data-rate can be used for uplink
    bool downlink;                      // data-rate can be used for downlink

    MODULATION modulation;
    BANDWIDTH bandwidth;                // in kHz, used for LoRa
    SPREADING_FACTOR spreadingFactor;   // used for LoRa
    uint32_t bps;       				// FSK bits per second

    DataRate();
    DataRate(const DataRate &value);
    // Lora modulation
    DataRate(BANDWIDTH bandwidth, SPREADING_FACTOR spreadingFactor);
    // FSK modulation
    DataRate(uint32_t bps);
    void setLora(BANDWIDTH bandwidth, SPREADING_FACTOR spreadingFactor);
    void setFSK(uint32_t bps);
    std::string toJsonString() const override;
};

// Channel
class Channel : public RBJsonIntf {
public:
    int frequency;  // frequency in Hz
    int minDR;
    int maxDR;
    bool enabled;
    bool custom;    // this channel was configured by the user
    Channel();
    Channel(const Channel &value);
    std::string toJsonString() const override;

    void setValue(int frequency, int minDR, int maxDR, bool enabled, bool custom);
};

// BandDefaults defines the default bands defined by a band.
class BandDefaults : public RBJsonIntf {
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
    void setValue(
        int RX2Frequency,
        int RX2DataRate,
        int ReceiveDelay1,
        int ReceiveDelay2,
        int JoinAcceptDelay1,
        int JoinAcceptDelay2);
    std::string toJsonString() const override;
};

// MaxPayloadSize defines the max payload size
class MaxPayloadSize : public RBJsonIntf {
public:
    uint8_t m;  // The maximum MACPayload size length
    uint8_t n;  // The maximum application payload length in the absence of the optional FOpt control field
    MaxPayloadSize();
    MaxPayloadSize(const MaxPayloadSize &value);
    std::string toJsonString() const override;

    void setValue(uint8_t m, uint8_t n);
};

class RegionBand : public RBJsonIntf {
public:
    std::string name;
    bool defaultRegion;
    bool supportsExtraChannels;
    BandDefaults bandDefaults;
    DataRate dataRates[DATA_RATE_SIZE];
    MaxPayloadSize maxPayloadSizePerDataRate[DATA_RATE_SIZE];
    MaxPayloadSize maxPayloadSizePerDataRateRepeater[DATA_RATE_SIZE];    // if repeater is used
    std::vector<uint8_t> rx1DataRateOffsets[DATA_RATE_SIZE];
    int8_t txPowerOffsets[DATA_RATE_SIZE];

    std::vector<Channel> uplinkChannels;
    std::vector<Channel> downlinkChannels;

    RegionBand();
    RegionBand(const RegionBand &value);
    std::string toJsonString() const override;

    void setTxPowerOffsets(int8_t v0, int8_t v1, int8_t v2, int8_t v3, int8_t v4, int8_t v5, int8_t v6, int8_t v7);

    void setRx1DataRateOffsets(int dataRateIndex, int count, ...);
};

class RegionBands : public RBJsonIntf {
public:
    REGIONAL_PARAMETERS_VERSION regionalParametersVersion;  // since specified LoraWAN regional parameters version, if version 0.0.0- any(default) version
    std::vector<RegionBand> bands;
    RegionBands();
    RegionBands(const RegionBands &value);
    const RegionBand* get(const std::string &name) const;
    std::string toJsonString() const override;
    bool setRegionalParametersVersion(const std::string &value);
};

#endif
