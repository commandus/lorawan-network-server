#include <sstream>
#include <cstdarg>
#include <iostream>

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

DataRate::DataRate(BANDWIDTH aBandwidth, SPREADING_FACTOR aSpreadingFactor)
    : uplink(true), downlink(true), modulation(LORA),
    bandwidth(aBandwidth), spreadingFactor(aSpreadingFactor), bps(0)
{

}

DataRate::DataRate(uint32_t aBps)
    : uplink(true), downlink(true), modulation(FSK),
    bandwidth(BW_7KHZ), spreadingFactor(DRLORA_SF5), bps(aBps)
{

}

void DataRate::setLora(BANDWIDTH aBandwidth, SPREADING_FACTOR aSpreadingFactor)
{
    uplink = true;
    downlink = true;
    modulation = LORA;
    bandwidth = aBandwidth;
    spreadingFactor = aSpreadingFactor;
    bps = 0;
}

void DataRate::setFSK(uint32_t aBps)
{
    uplink = true;
    downlink = true;
    modulation = FSK;
    bandwidth = BW_7KHZ;
    spreadingFactor = DRLORA_SF5;
    bps = aBps;
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
    ss << "{\"frequency\": " << frequency  // frequency in Hz
       << ", \"minDR\": " << minDR
       << ", \"maxDR\": " << maxDR
       << ", \"enabled\": " << (enabled ? STR_TRUE_FALSE)
       << ", \"custom\": " << (custom ? STR_TRUE_FALSE)
       << "}";
    return ss.str();
}

void Channel::setValue(int aFrequency, int aMinDR, int aMaxDR, bool aEnabled, bool aCustom)
{
    frequency = aFrequency;
    minDR = aMinDR;
    maxDR = aMaxDR;
    enabled = aEnabled;
    custom = aCustom;
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

void BandDefaults::setValue(
    int aRX2Frequency,
    int aRX2DataRate,
    int aReceiveDelay1,
    int aReceiveDelay2,
    int aJoinAcceptDelay1,
    int aJoinAcceptDelay2
) {
    RX2Frequency = aRX2Frequency;
    RX2DataRate = aRX2DataRate;
    ReceiveDelay1 = aReceiveDelay1;
    ReceiveDelay2 = aReceiveDelay2;
    JoinAcceptDelay1 = aJoinAcceptDelay1;
    JoinAcceptDelay2 = aJoinAcceptDelay2;
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
    ss << "{\"m\": " << (int) m
       << ", \"n\": " << (int) n
       << "}";
    return ss.str();
}

void MaxPayloadSize::setValue(uint8_t am, uint8_t an) {
    m = am;
    n = an;
}

RegionBand::RegionBand()
    : id(0), name(""), supportsExtraChannels(false), defaultRegion(false), txPowerOffsetsSize(0)
{
    for (int i = 0; i < TX_POWER_OFFSET_MAX_SIZE; i++) {
        txPowerOffsets[i] = 0;
    }
}

RegionBand::RegionBand(const RegionBand &value)
    : id(value.id), name(value.name), supportsExtraChannels(value.supportsExtraChannels), defaultRegion(value.defaultRegion),
    bandDefaults(value.bandDefaults),
    uplinkChannels(value.uplinkChannels), downlinkChannels(value.downlinkChannels),
    txPowerOffsetsSize(value.txPowerOffsetsSize)
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
    for (int i = 0; i < TX_POWER_OFFSET_MAX_SIZE; i++ ) {
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
        strm << (int) value[i];
    }
    strm << "]";
}

static void txPowerOffsetsAppendJSON(std::ostream &strm,
  uint8_t txPowerOffsetsSize, const int8_t (&value)[TX_POWER_OFFSET_MAX_SIZE])
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < txPowerOffsetsSize; i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << (int) value[i];
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

static void rx1DataRateOffsetsAppendJSON(std::ostream &strm, const std::vector<uint8_t> (&value)[DATA_RATE_SIZE])
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        bool hasPrev2 = false;
        strm << "[";
        const std::vector<uint8_t> &v = value[i];
        for (std::vector<uint8_t>::const_iterator it(v.begin()); it != v.end(); it++) {
            if (hasPrev2)
                strm << ", ";
            else
                hasPrev2 = true;
            strm << (int) *it;
        }
        strm << "]";
    }
    strm << "]";
}

std::string RegionBand::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"id\": " << (int) id
       << ", \"name\": \"" << name
       << "\", \"supportsExtraChannels\": " << (supportsExtraChannels ? STR_TRUE_FALSE)
       << ", \"defaultRegion\": " << (defaultRegion ? STR_TRUE_FALSE)
       << ", \"bandDefaults\": " << bandDefaults.toJsonString()
       << ", \"dataRates\": ";
    arrayAppendJSON(ss, dataRates);
    ss << ", \"uplinkChannels\": ";

    vectorAppendJSON(ss, uplinkChannels);
    ss << ", \"downlinkChannels\": ";
    vectorAppendJSON(ss, downlinkChannels);

    ss << ", \"maxPayloadSizePerDataRate\": ";
    arrayAppendJSON(ss, maxPayloadSizePerDataRate);
    ss << ", \"maxPayloadSizePerDataRateRepeater\": ";
    arrayAppendJSON(ss, maxPayloadSizePerDataRateRepeater);
    ss << ", \"rx1DataRateOffsets\": ";
    // ss << "[]";
    rx1DataRateOffsetsAppendJSON(ss, rx1DataRateOffsets);
    ss << ", \"txPowerOffsets\": ";
    txPowerOffsetsAppendJSON(ss, txPowerOffsetsSize, txPowerOffsets);

    ss << "}";
    return ss.str();
}

void RegionBand::setTxPowerOffsets(int count, ...)
{
    if (count >= TX_POWER_OFFSET_MAX_SIZE)
        return;
    txPowerOffsetsSize = count;
    va_list ap;
    va_start(ap, count);
    for (int i = 0; i < count; i++) {
        txPowerOffsets[i] = va_arg(ap, int);
    }
    va_end(ap);
}

void RegionBand::setRx1DataRateOffsets(int dataRateIndex, int count, ...)
{
    if (dataRateIndex >= DATA_RATE_SIZE)
        return;
    va_list ap;
    va_start(ap, count);
    rx1DataRateOffsets[dataRateIndex].clear();
    for (int i = 0; i < count; i++) {
        rx1DataRateOffsets[dataRateIndex].push_back(va_arg(ap, int));
    }
    va_end(ap);
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
    vectorAppendJSON(ss, bands);
    ss << "}";
    return ss.str();
}

bool RegionBands::setRegionalParametersVersion(const std::string &value)
{
    regionalParametersVersion = string2REGIONAL_PARAMETERS_VERSION(value);
    return true;
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

