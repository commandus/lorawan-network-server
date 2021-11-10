#include <sstream>

#include "region-band.h"
#include "errlist.h"

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

std::string DataRate::toJsonString()
{
    std::stringstream ss;
    // std::boolalpha
    ss << "{\"uplink\": " << (uplink ? "true" : "false")
        << ", \"downlink\": " << (downlink ? "true" : "false")
        << ", \"modulation\": \"" << MODULATION2String(modulation)
        << "\", \"bandwidth\": " <<  BANDWIDTH2String(bandwidth)
        << ", \"spreadingFactor\": " << spreadingFactor
        << ", \"bps\": " <<  bps;
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

std::string Channel::toJsonString()
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

std::string BandDefaults::toJsonString()
{
    std::stringstream ss;
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

std::string MaxPayloadSize::toJsonString()
{
    std::stringstream ss;
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
        maxPayloadSizePerDataRateRepeator[i] = value.maxPayloadSizePerDataRateRepeator[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        rx1DataRateOffsets[i] = value.rx1DataRateOffsets[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        txPowerOffsets[i] = value.txPowerOffsets[i];
    }
}

std::string RegionBand::toJsonString()
{
    std::stringstream ss;
    return ss.str();
}

static bool isAnyLorawanVersion(LORAWAN_VERSION value) {
    return *(uint8_t*) &value == 0;
}

static bool isAnyRegionalParametersVersion(REGIONAL_PARAMETERS_VERSION value) {
    return *(uint8_t*) &value == 0;
}
