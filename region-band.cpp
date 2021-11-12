#include <sstream>

#include "region-band.h"
#include "errlist.h"

static std::string STR_TRUE("true");
static std::string STR_FALSE("false");

#define STR_TRUE_FALSE STR_TRUE : STR_FALSE

DataRate::DataRate()
    : uplink(true), downlink(true), modulation(LORA),
    bandwidth(BW_125KHZ), spreadingFactor(DRLORA_SF11), bps(0)
{

}

DataRate::DataRate(const DataRate &value)
    : uplink(value.uplink), downlink(value.downlink), modulation(value.modulation),
    bandwidth(value.bandwidth), spreadingFactor(value.spreadingFactor), bps(value.bps)
{

}

std::string DataRate::toJsonString() const
{
    std::stringstream ss;
    // std::boolalpha
    ss << "{\"uplink\": " << (uplink ? STR_TRUE_FALSE)
        << ", \"downlink\": " << (downlink ? STR_TRUE_FALSE)
        << ", \"modulation\": \"" << MODULATION2String(modulation)
        << "\", \"bandwidth\": " <<  BANDWIDTH2String(bandwidth)
        << ", \"spreadingFactor\": " << spreadingFactor
        << ", \"bps\": " <<  bps
        << "}";
    return ss.str();
}

Channel::Channel()
    : frequency(0), minDR(0), maxDR(0),
    enabled(true), custom(false)
{

}

Channel::Channel(const Channel &value)
    : frequency(value.frequency), minDR(value.minDR), maxDR(value.maxDR),
    enabled(value.enabled), custom(value.custom)
{

}

std::string Channel::toJsonString() const
{
    std::stringstream ss;
    return ss.str();
}

BandDefaults::BandDefaults()
    : RX2Frequency(0), RX2DataRate(0), ReceiveDelay1(0),
    ReceiveDelay2(0), JoinAcceptDelay1(0), JoinAcceptDelay2(0)
{

}

BandDefaults::BandDefaults(const BandDefaults& value)
    : RX2Frequency(value.RX2Frequency), RX2DataRate(value.RX2DataRate), ReceiveDelay1(value.ReceiveDelay1),
    ReceiveDelay2(value.ReceiveDelay2), JoinAcceptDelay1(value.JoinAcceptDelay1), JoinAcceptDelay2(value.JoinAcceptDelay2)
{

}

std::string BandDefaults::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"RX2Frequency\": " << RX2Frequency
        << ", \"RX2DataRate\": " << RX2DataRate
        << ", \"ReceiveDelay1\": " << ReceiveDelay1
        << ", \"ReceiveDelay2\": " << ReceiveDelay2
        << ", \"JoinAcceptDelay1\": " << JoinAcceptDelay1
        << ", \"JoinAcceptDelay2\": " << JoinAcceptDelay2
        << "}";
    return ss.str();
}

MaxPayloadSize::MaxPayloadSize()
    : m(0), n(0)
{

}

MaxPayloadSize::MaxPayloadSize(const MaxPayloadSize &value)
    : m(value.m), n(value.n)
{

}

std::string MaxPayloadSize::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"m\": " << m
       << ", \"n\": " << n
       << "}";
    return ss.str();
}

static void clearTxPowerOffsets(int8_t* value)
{
    memset(value, 0, sizeof(int8_t[DATA_RATE_SIZE]));
}

RegionBand::RegionBand()
    : name(""), supportsExtraChannels(false)
{
    clearTxPowerOffsets(&txPowerOffsets[0]);
}

RegionBand::RegionBand(const RegionBand &value)
    : name(value.name), supportsExtraChannels(value.supportsExtraChannels),
    bandDefaults(value.bandDefaults),
    uplinkChannels(value.uplinkChannels), downlinkChannels(value.downlinkChannels)
{
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        dataRates[i] = value.dataRates[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        maxPayloadSizePerDataRate[i] = value.maxPayloadSizePerDataRate[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        maxPayloadSizePerDataRateRepeater[i] = value.maxPayloadSizePerDataRateRepeater[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        rx1DataRateOffsets[i] = value.rx1DataRateOffsets[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        txPowerOffsets[i] = value.txPowerOffsets[i];
    }
}

template <typename T, typename A>
static void vectorAppendJSON(std::ostream &strm, std::vector<T, A> const &value)
{
    strm << "[";
    bool hasPrev = false;
    for (typename std::vector<T, A>::const_iterator it(value.begin()); it != value.end(); it++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << it->toJsonString();
    }
    strm << "]";
}

template <typename T>
static void intsAppendJSON(std::ostream &strm, T &value)
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << (uint) value[i];
    }
    strm << "]";
}

template <typename T>
static void arrayAppendJSON(std::ostream &strm, T &value)
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << value[i].toJsonString();
    }
    strm << "]";
}

std::string RegionBand::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"name\": \"" << name
       << "\", \"supportsExtraChannels\": " << (supportsExtraChannels ? STR_TRUE_FALSE)
       << ", \"supportsExtraChannels\": " << (supportsExtraChannels ? STR_TRUE_FALSE)
       << ", \"bandDefaults\": " << bandDefaults.toJsonString()
       << ", \"uplinkChannels\": ";
    vectorAppendJSON(ss, uplinkChannels);
    ss << ", \"downlinkChannels\": ";
    vectorAppendJSON(ss, downlinkChannels);

    ss << ", \"maxPayloadSizePerDataRate\": ";
    arrayAppendJSON(ss, maxPayloadSizePerDataRate);
    ss << ", \"maxPayloadSizePerDataRateRepeater\": ";
    arrayAppendJSON(ss, maxPayloadSizePerDataRateRepeater);
    ss << ", \"rx1DataRateOffsets\": ";
    intsAppendJSON(ss, rx1DataRateOffsets);
    ss << ", \"txPowerOffsets\": ";
    intsAppendJSON(ss, txPowerOffsets);

    ss << "}";
    return ss.str();
}

const RegionBand* RegionBands::get(const std::string &name) const
{
    for (std::vector<RegionBand>::const_iterator it(bands.begin()); it != bands.end(); it++) {
        if (it->name == name) {
            return &*it;
        }
    }
    return nullptr;
}

std::string RegionBands::toJsonString() const {
    std::stringstream ss;
    ss << "{\"regionalParametersVersion\": \"" << REGIONAL_PARAMETERS_VERSION2string(regionalParametersVersion)
       << "\", \"RegionBands\": ";
    arrayAppendJSON(ss, bands);
    ss << "}";
    return ss.str();
}

static bool isAnyLorawanVersion(LORAWAN_VERSION value) {
    return *(uint8_t*) &value == 0;
}

static bool isAnyRegionalParametersVersion(REGIONAL_PARAMETERS_VERSION value) {
    return *(uint8_t*) &value == 0;
}

RegionBands::RegionBands()
{
    regionalParametersVersion = { 0, 0, 0 };
}

RegionBands::RegionBands(const RegionBands &value)
    : regionalParametersVersion(value.regionalParametersVersion), bands(value.bands)
{
}

