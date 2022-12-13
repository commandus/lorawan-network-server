// disable rapidjson's assert
#define NDEBUG

#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>

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

#define DEFAULT_BEACON_POWER        14
#define DEFAULT_BEACON_BW_HZ        125000
#define DEFAULT_BEACON_FREQ_HZ      869525000
#define DEFAULT_BEACON_DATARATE     9
#define DEFAULT_BEACON_FREQ_NB      1
#define DEFAULT_KEEPALIVE           5   // default time interval for downstream keep-alive packet

GatewayJsonConfig::GatewayJsonConfig()
    : errorCode(0), errorOffset(0)
{

}

int GatewayJsonConfig::parseString(
    const std::string &json
)
{
    rapidjson::Document doc;
    rapidjson::ParseResult r = doc.Parse<rapidjson::kParseCommentsFlag>(json.c_str());
    if (!r) {
        errorCode = r.Code();
        errorOffset  = r.Offset();
        errorDescription = std::string(GetParseError_En(r.Code()));
        return ERR_CODE_INVALID_JSON;
    }
    return parse(doc);
}

std::string GatewayJsonConfig::toString()
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document doc;
    toJSON(doc, doc.GetAllocator());
    doc.Accept(writer);
    return std::string(buffer.GetString());
}

std::string GatewayJsonConfig::toCppString(const std::string &name)
{
    std::stringstream ss;
    toCpp(ss, name);
    return ss.str();
}

GatewaySX1261Config::GatewaySX1261Config()
{
    reset();
}

void GatewaySX1261Config::reset()
{
    memset(&value.sx1261, 0, sizeof(struct lgw_conf_sx1261_s));
    memset(&value.spectralScan, 0, sizeof(spectral_scan_t));
    memset(&value.lbt, 0, sizeof(struct lgw_conf_lbt_s));
}

bool GatewaySX1261Config::operator==(
    const GatewaySX1261Config &b
) const
{
    if (value.lbt.nb_channel != b.value.lbt.nb_channel)
        return false;
    return (memcmp(&value.sx1261, &b.value.sx1261, sizeof(struct lgw_conf_sx1261_s)) == 0)
        && (memcmp(&value.spectralScan, &b.value.spectralScan, sizeof(spectral_scan_t)) == 0)
        && (memcmp(&value.lbt.channels, &b.value.lbt.channels, value.lbt.nb_channel * sizeof(struct lgw_conf_chan_lbt_s)) == 0);
}

int GatewaySX1261Config::parse(rapidjson::Value &jsonValue)
{
    if (jsonValue.HasMember("spi_path")) {
        rapidjson::Value &jSpiPath = jsonValue["spi_path"];
        if (jSpiPath.IsString()) {
            std::string s = jSpiPath.GetString();
            size_t sz = s.size();
            if (sz < 64) {
                strncpy(&value.sx1261.spi_path[0], s.c_str(), 64);
                value.sx1261.spi_path[sz] = 0;
            }
        }
    }
    if (jsonValue.HasMember("rssi_offset")) {
        rapidjson::Value &jRssiOffset = jsonValue["rssi_offset"];
        if (jRssiOffset.IsInt()) {
            value.sx1261.rssi_offset = jRssiOffset.GetInt();
        }
    }
    if (jsonValue.HasMember("spectral_scan")) {
        rapidjson::Value &jSpectralScan = jsonValue["spectral_scan"];
        if (jSpectralScan.IsObject()) {
            if (jSpectralScan.HasMember("enable")) {
                rapidjson::Value &jValue = jSpectralScan["enable"];
                if (jValue.IsBool()) {
                    value.spectralScan.enable = jValue.GetBool();
                }
                if (value.spectralScan.enable) {
                    value.sx1261.enable = true;
                }
                if (jSpectralScan.HasMember("freq_start")) {
                    rapidjson::Value &jFreqStart = jSpectralScan["freq_start"];
                    if (jFreqStart.IsUint()) {
                        value.spectralScan.freq_hz_start = jFreqStart.GetUint();
                    }
                }
                if (jSpectralScan.HasMember("nb_chan")) {
                    rapidjson::Value &jNbChan = jSpectralScan["nb_chan"];
                    if (jNbChan.IsUint()) {
                        value.spectralScan.nb_chan = jNbChan.GetUint();
                    }
                }
                if (jSpectralScan.HasMember("nb_scan")) {
                    rapidjson::Value &jNbScan = jSpectralScan["nb_scan"];
                    if (jNbScan.IsUint()) {
                        value.spectralScan.nb_scan = jNbScan.GetUint();
                    }
                }
                if (jSpectralScan.HasMember("pace_s")) {
                    rapidjson::Value &jPaceS = jSpectralScan["pace_s"];
                    if (jPaceS.IsUint()) {
                        value.spectralScan.pace_s = jPaceS.GetUint();
                    }
                }
            }
        }
    }
    // set LBT channels configuration
    if (jsonValue.HasMember("lbt")) {
        rapidjson::Value &jLbt = jsonValue["lbt"];
        if (jLbt.IsObject()) {
            if (jLbt.HasMember("enable")) {
                rapidjson::Value &jLbtEnable = jLbt["enable"];
                if (jLbtEnable.IsBool()) {
                    value.lbt.enable = jLbtEnable.GetBool();
                }

                // Enable the sx1261 radio hardware configuration to allow spectral scan
                if (value.lbt.enable)
                    value.sx1261.enable = true;

                if (jLbt.HasMember("rssi_target")) {
                    rapidjson::Value &RssiTarget = jLbt["rssi_target"];
                    if (RssiTarget.IsInt()) {
                        value.lbt.rssi_target = RssiTarget.GetInt();
                    }
                }

                // set LBT channels configuration
                if (jLbt.HasMember("channels")) {
                    rapidjson::Value &jChannels = jLbt["channels"];
                    if (jChannels.IsArray()) {
                        value.lbt.nb_channel = jChannels.Size();
                        if (value.lbt.nb_channel > LGW_LBT_CHANNEL_NB_MAX)
                            value.lbt.nb_channel = LGW_LBT_CHANNEL_NB_MAX;
                        for (int i = 0; i < value.lbt.nb_channel; i++) {
                            rapidjson::Value &jChannel = jChannels[i];
                            if (jChannel.HasMember("freq_hz")) {
                                rapidjson::Value &jValue = jChannel["freq_hz"];
                                if (jValue.IsUint()) {
                                    value.lbt.channels[i].freq_hz = jValue.GetUint();
                                }
                            }
                            if (jChannel.HasMember("bandwidth")) {
                                rapidjson::Value &jValue = jChannel["bandwidth"];
                                if (jValue.IsUint()) {
                                    uint32_t bw = jValue.GetUint();
                                    switch(bw) {
                                        case 500000:
                                            value.lbt.channels[i].bandwidth = BW_500KHZ;
                                            break;
                                        case 250000:
                                            value.lbt.channels[i].bandwidth = BW_250KHZ;
                                            break;
                                        case 125000:
                                            value.lbt.channels[i].bandwidth = BW_125KHZ;
                                            break;
                                        default:
                                            value.lbt.channels[i].bandwidth = BW_UNDEFINED;
                                    }
                                }
                            }
                            if (jChannel.HasMember("scan_time_us")) {
                                rapidjson::Value &jValue = jChannel["scan_time_us"];
                                if (jValue.IsUint()) {
                                    switch (jValue.GetUint()) {
                                        case LGW_LBT_SCAN_TIME_128_US:
                                            value.lbt.channels[i].scan_time_us = LGW_LBT_SCAN_TIME_128_US;
                                            break;
                                        default:
                                            value.lbt.channels[i].scan_time_us = LGW_LBT_SCAN_TIME_5000_US;
                                    }
                                }
                            }
                            if (jChannel.HasMember("transmit_time_ms")) {
                                rapidjson::Value &jValue = jChannel["transmit_time_ms"];
                                if (jValue.IsUint()) {
                                    value.lbt.channels[i].transmit_time_ms = jValue.GetUint();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static int bandwidthIndex2hz(
        uint8_t bandwidthIndex
) {
    switch(bandwidthIndex) {
        case BW_500KHZ:
            return 500000;
        case BW_250KHZ:
            return 250000;
        case BW_125KHZ:
            return 125000;
            // TODO ?!!
        case 3:
            return 62000;
        case 2:
            return 41000;
        case 1:
            return 31000;
        case 0:
            return 20000;
        default:
            return 15000;
    }
}

#define MAKE_STR2(x) #x
#define ADD_STRING(V, SRC, VAL) retVal << V  << "." << MAKE_STR2(VAL) << " = \"" << SRC.VAL << "\";" << std::endl;
#define ADD_PCHAR(V, SRC, VAL) retVal << "strcpy(" << V  << "." << MAKE_STR2(VAL) << ", \"" << SRC.VAL << "\");" << std::endl;

#define ADD_CAST(V, SRC, VAL, CAST_TYPE) retVal << V << "." << MAKE_STR2(VAL) << " = (" << MAKE_STR2(CAST_TYPE) << ") " << SRC.VAL << ";" << std::endl;
#define ADD_VAL(V, SRC, VAL) retVal << V  << "." << MAKE_STR2(VAL) << " = " << SRC.VAL << ";" << std::endl;
#define ADD_INT(V, SRC, VAL) retVal << V  << "." << MAKE_STR2(VAL) << " = " << (int64_t) SRC.VAL << ";" << std::endl;
#define ADD_BOOL(V, SRC, VAL) retVal << V << "." << MAKE_STR2(VAL) << " = " << (SRC.VAL ? "true" : "false") << ";" << std::endl;

void GatewaySX1261Config::toCpp(
    std::ostream &retVal,
    const std::string &name
) const
{
    retVal << std::endl << "// SX1261 " << std::endl << std::endl;
    ADD_PCHAR(name, value, sx1261.spi_path)
    ADD_INT(name, value, sx1261.rssi_offset)
    ADD_BOOL(name, value, spectralScan.enable)
    ADD_VAL(name, value, spectralScan.freq_hz_start)
    ADD_INT(name, value, spectralScan.nb_chan)
    ADD_CAST(name, value, spectralScan.nb_scan, int)
    ADD_VAL(name, value, spectralScan.pace_s)
    ADD_BOOL(name, value, lbt.enable)
    ADD_INT(name, value, lbt.nb_channel)
    for (int i = 0; i < value.lbt.nb_channel; i++) {
        ADD_VAL(name, value, lbt.channels[i].freq_hz)
        ADD_VAL(name, value, lbt.channels[i].bandwidth)
        ADD_VAL(name, value, lbt.channels[i].scan_time_us)
        ADD_VAL(name, value, lbt.channels[i].transmit_time_ms)
    }
}

void GatewaySX1261Config::toJSON(
    rapidjson::Value &jsonValue,
    rapidjson::Document::AllocatorType& allocator
) const {
    jsonValue.SetObject();
    rapidjson::Value jSpiPath;
    jSpiPath.SetString(value.sx1261.spi_path, allocator);
    jsonValue.AddMember("spi_path", jSpiPath, allocator);

    rapidjson::Value jRssiOffset;
    jRssiOffset.SetInt(value.sx1261.rssi_offset);
    jsonValue.AddMember("rssi_offset", jRssiOffset, allocator);

    rapidjson::Value jSpectralScan;
    jSpectralScan.SetObject();

    rapidjson::Value jSpectralScanEnable;
    jSpectralScanEnable.SetBool(value.spectralScan.enable);
    jSpectralScan.AddMember("enable", jSpectralScanEnable, allocator);

    rapidjson::Value jFreqStart;

    jFreqStart.SetUint(value.spectralScan.freq_hz_start);
    jSpectralScan.AddMember("freq_start", jFreqStart, allocator);

    rapidjson::Value jFreqNbChan;
    jFreqNbChan.SetUint(value.spectralScan.nb_chan);
    jSpectralScan.AddMember("nb_chan", jFreqNbChan, allocator);

    rapidjson::Value jFreqNbScan;
    jFreqNbScan.SetUint(value.spectralScan.nb_scan);
    jSpectralScan.AddMember("nb_scan", jFreqNbScan, allocator);

    rapidjson::Value jPaceS;
    jPaceS.SetUint(value.spectralScan.pace_s);
    jSpectralScan.AddMember("pace_s", jPaceS, allocator);

    jsonValue.AddMember("spectral_scan", jSpectralScan, allocator);

    rapidjson::Value jLbt;
    jLbt.SetObject();

    rapidjson::Value jLbtEnable;
    jLbtEnable.SetBool(value.lbt.enable);
    jLbt.AddMember("enable", jLbtEnable, allocator);


    rapidjson::Value jRssiTarget;
    jRssiTarget.SetInt(value.lbt.rssi_target);
    jLbt.AddMember("rssi_target", jRssiTarget, allocator);

    rapidjson::Value jChannels;
    jChannels.SetArray();

    for (int i = 0; i < value.lbt.nb_channel; i++) {
        rapidjson::Value jChannel;
        jChannel.SetObject();

        rapidjson::Value jFreq;
        jFreq.SetUint(value.lbt.channels[i].freq_hz);
        jChannel.AddMember("freq_hz", jFreq, allocator);

        rapidjson::Value jBandwidth;
        jBandwidth.SetUint(bandwidthIndex2hz(value.lbt.channels[i].bandwidth));
        jChannel.AddMember("bandwidth", jBandwidth, allocator);

        rapidjson::Value jScanTimeUs;
        jScanTimeUs.SetUint(value.lbt.channels[i].scan_time_us);
        jChannel.AddMember("scan_time_us", jScanTimeUs, allocator);

        rapidjson::Value jTransmitTimeMs;
        jTransmitTimeMs.SetUint(value.lbt.channels[i].transmit_time_ms);
        jChannel.AddMember("transmit_time_ms", jTransmitTimeMs, allocator);

        jChannels.PushBack(jChannel, allocator);

    }
    jLbt.AddMember("channels", jChannels, allocator);

    jsonValue.AddMember("lbt", jLbt, allocator);
}

GatewaySX130xConfig::GatewaySX130xConfig()
{
    reset();
}

void GatewaySX130xConfig::reset()
{
    sx1261Config.reset();
    value.antennaGain = 0;
    memset(&value.boardConf, 0, sizeof(struct lgw_conf_board_s));
    memset(&value.tsConf, 0, sizeof(struct lgw_conf_ftime_s));
    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        value.tx_freq_min[i] = 0;
        value.tx_freq_max[i] = 0;
        memset(&value.rfConfs[i], 0, sizeof(struct lgw_conf_rxrf_s));
        memset(&value.txLut[i], 0, sizeof(struct lgw_tx_gain_lut_s));
    }
    for (int i = 0; i < LGW_MULTI_NB; i++) {
        memset(&value.ifConfs[i], 0, sizeof(struct lgw_conf_rxif_s));
    }
    memset(&value.ifStdConf, 0, sizeof(struct lgw_conf_rxif_s));
    memset(&value.ifFSKConf, 0, sizeof(struct lgw_conf_rxif_s));
    memset(&value.demodConf, 0, sizeof(struct lgw_conf_demod_s));
}

bool GatewaySX130xConfig::operator==(
    const GatewaySX130xConfig &b
) const
{
    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        if (memcmp(&value.rfConfs[i], &b.value.rfConfs[i], sizeof(struct lgw_conf_rxrf_s)))
            return false;

        if (memcmp(&value.txLut[i], &b.value.txLut[i], sizeof(struct lgw_tx_gain_lut_s)))
            return false;
        if (value.tx_freq_min[i] != b.value.tx_freq_min[i])
            return false;
        if (value.tx_freq_max[i] != b.value.tx_freq_max[i])
            return false;
    }

    for (int i = 0; i < LGW_MULTI_NB; i++) {
        if (memcmp(&value.ifConfs[i], &b.value.ifConfs[i], sizeof(struct lgw_conf_rxif_s)))
            return false;
    }

    return
        (sx1261Config == b.sx1261Config)
        && (value.antennaGain == b.value.antennaGain)
        // && (memcmp(&boardConf, &b.boardConf, sizeof(struct lgw_conf_board_s)) == 0)
        && value.boardConf.lorawan_public == b.value.boardConf.lorawan_public
        && value.boardConf.clksrc == b.value.boardConf.clksrc
        && value.boardConf.full_duplex == b.value.boardConf.full_duplex
        && value.boardConf.com_type == b.value.boardConf.com_type
        && strncmp(value.boardConf.com_path, b.value.boardConf.com_path, 64) == 0

        && (memcmp(&value.ifStdConf, &b.value.ifStdConf, sizeof(struct lgw_conf_rxif_s)) == 0)
        && (memcmp(&value.ifFSKConf, &b.value.ifFSKConf, sizeof(struct lgw_conf_rxif_s)) == 0)
        && (memcmp(&value.demodConf, &b.value.demodConf, sizeof(struct lgw_conf_demod_s)) == 0)
        && (memcmp(&value.tsConf, &b.value.tsConf, sizeof(struct lgw_conf_ftime_s)) == 0);
}

std::string GatewaySX130xConfig::getUsbPath()
{
    if (value.boardConf.com_path[63] == '\0')
        return std::string(value.boardConf.com_path);
    else
        return std::string(value.boardConf.com_path, 64);
}

void GatewaySX130xConfig::setUsbPath(const std::string &val)
{
    size_t sz = val.size();
    if (sz > 64)
        sz = 64;
    strncpy(value.boardConf.com_path, val.c_str(), sz);
}

int GatewaySX130xConfig::parse(rapidjson::Value &jsonValue) {
    reset();
    if (jsonValue.HasMember("sx1261_conf")) {
        rapidjson::Value &jSx1261 = jsonValue["sx1261_conf"];
        int r = sx1261Config.parse(jSx1261);
        if (r)
            return r;
    }

    value.boardConf.com_type = LGW_COM_UNKNOWN;
    if (jsonValue.HasMember("com_type")) {
        rapidjson::Value &jComType = jsonValue["com_type"];
        if (jComType.IsString()) {
            std::string s = jComType.GetString();
            if (s == "SPI" || s == "spi")
                value.boardConf.com_type = LGW_COM_SPI;
            if (s == "USB" || s == "usb")
                value.boardConf.com_type = LGW_COM_USB;
        }
    }
    if (value.boardConf.com_type == LGW_COM_UNKNOWN)
        return ERR_CODE_INVALID_JSON;

    if (jsonValue.HasMember("com_path")) {
        rapidjson::Value &jComPath = jsonValue["com_path"];
        if (jComPath.IsString()) {
            std::string s = jComPath.GetString();
            size_t sz = s.size();
            if (sz < 64) {
                strncpy(&value.boardConf.com_path[0], s.c_str(), 64);
                value.boardConf.com_path[sz] = 0;
            }
        }
    }

    if (jsonValue.HasMember("lorawan_public")) {
        rapidjson::Value &jLorawanPublic = jsonValue["lorawan_public"];
        if (jLorawanPublic.IsBool()) {
            value.boardConf.lorawan_public = jLorawanPublic.GetBool();
        }
    }

    if (jsonValue.HasMember("clksrc")) {
        rapidjson::Value &jClcSrc = jsonValue["clksrc"];
        if (jClcSrc.IsUint()) {
            value.boardConf.clksrc = jClcSrc.GetUint();
        }
    }

    if (jsonValue.HasMember("full_duplex")) {
        rapidjson::Value &jFullDuplex = jsonValue["full_duplex"];
        if (jFullDuplex.IsBool()) {
            value.boardConf.full_duplex = jFullDuplex.GetBool();
        }
    }

    if (jsonValue.HasMember("antenna_gain")) {
        rapidjson::Value &jAntennaGain = jsonValue["antenna_gain"];
        if (jAntennaGain.IsInt()) {
            value.antennaGain = jAntennaGain.GetInt();
        }
    }

    if (jsonValue.HasMember("fine_timestamp")) {
        rapidjson::Value &jFineTimestamp = jsonValue["fine_timestamp"];
        if (jFineTimestamp.IsObject()) {
            if (jFineTimestamp.HasMember("enable")) {
                rapidjson::Value &jValue = jFineTimestamp["enable"];
                if (jValue.IsBool()) {
                    value.tsConf.enable = jValue.GetBool();
                }
                value.tsConf.mode = LGW_FTIME_MODE_HIGH_CAPACITY;
                if (value.tsConf.enable) {
                    if (jFineTimestamp.HasMember("mode")) {
                        rapidjson::Value &jMode = jFineTimestamp["mode"];
                        if (jMode.IsString()) {
                            std::string s = jMode.GetString();
                            if (s == "all_sf" || s == "ALL_SF") {
                                value.tsConf.mode = LGW_FTIME_MODE_ALL_SF;
                            }
                            if (s == "high_capacity" || s == "HIGH_CAPACITY") {
                                value.tsConf.mode = LGW_FTIME_MODE_HIGH_CAPACITY;
                            }
                        }
                    }
                }
            }
        }
    }

    std::string rn = "radio_0";
    for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
        std::stringstream ssRadioName;
        rn[6] = '0' + radioIndex;
        if (jsonValue.HasMember(rn.c_str())) {
            rapidjson::Value &jRadio = jsonValue[rn.c_str()];
            if (jRadio.IsObject()) {
                if (jRadio.HasMember("enable")) {
                    rapidjson::Value &jRadioEnable = jRadio["enable"];
                    if (jRadioEnable.IsBool()) {
                        value.rfConfs[radioIndex].enable = jRadioEnable.GetBool();
                    }
                    value.rfConfs[radioIndex].type = LGW_RADIO_TYPE_NONE;
                    if (value.rfConfs[radioIndex].enable) {
                        if (jRadio.HasMember("type")) {
                            rapidjson::Value &jType = jRadio["type"];
                            if (jType.IsString()) {
                                std::string s = jType.GetString();
                                if (s == "SX1255") {
                                    value.rfConfs[radioIndex].type = LGW_RADIO_TYPE_SX1255;
                                }
                                if (s == "SX1257") {
                                    value.rfConfs[radioIndex].type = LGW_RADIO_TYPE_SX1257;
                                }
                                if (s == "SX1250") {
                                    value.rfConfs[radioIndex].type = LGW_RADIO_TYPE_SX1250;
                                }
                            }
                        }
                        if (jRadio.HasMember("freq")) {
                            rapidjson::Value &jFreq = jRadio["freq"];
                            if (jFreq.IsUint()) {
                                value.rfConfs[radioIndex].freq_hz = jFreq.GetUint();
                            }
                        }
                        if (jRadio.HasMember("rssi_offset")) {
                            rapidjson::Value &jRssiOffset = jRadio["rssi_offset"];
                            if (jRssiOffset.IsInt()) {
                                value.rfConfs[radioIndex].rssi_offset = jRssiOffset.GetInt();
                            }
                            if (jRssiOffset.IsDouble()) {
                                value.rfConfs[radioIndex].rssi_offset = jRssiOffset.GetDouble();
                            }
                        }

                        if (jRadio.HasMember("rssi_tcomp")) {
                            rapidjson::Value &jTComp = jRadio["rssi_tcomp"];
                            if (jTComp.IsObject()) {
                                if (jTComp.HasMember("coeff_a")) {
                                    rapidjson::Value &jCoeff = jTComp["coeff_a"];
                                    if (jCoeff.IsInt()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_a = jCoeff.GetInt();
                                    }
                                    if (jCoeff.IsDouble()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_a = jCoeff.GetDouble();
                                    }
                                }
                                if (jTComp.HasMember("coeff_b")) {
                                    rapidjson::Value &jCoeff = jTComp["coeff_b"];
                                    if (jCoeff.IsInt()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_b = jCoeff.GetInt();
                                    }
                                    if (jCoeff.IsDouble()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_b = jCoeff.GetDouble();
                                    }
                                }
                                if (jTComp.HasMember("coeff_c")) {
                                    rapidjson::Value &jCoeff = jTComp["coeff_c"];
                                    if (jCoeff.IsInt()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_c = jCoeff.GetInt();
                                    }
                                    if (jCoeff.IsDouble()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_c = jCoeff.GetDouble();
                                    }
                                }
                                if (jTComp.HasMember("coeff_d")) {
                                    rapidjson::Value &jCoeff = jTComp["coeff_d"];
                                    if (jCoeff.IsInt()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_d = jCoeff.GetInt();
                                    }
                                    if (jCoeff.IsDouble()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_d = jCoeff.GetDouble();
                                    }
                                }
                                if (jTComp.HasMember("coeff_e")) {
                                    rapidjson::Value &jCoeff = jTComp["coeff_e"];
                                    if (jCoeff.IsInt()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_e = jCoeff.GetInt();
                                    }
                                    if (jCoeff.IsDouble()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_e = jCoeff.GetDouble();
                                    }
                                }
                            }
                        }
                        if (jRadio.HasMember("single_input_mode")) {
                            rapidjson::Value &jValue = jRadio["single_input_mode"];
                            if (jValue.IsBool()) {
                                value.rfConfs[radioIndex].single_input_mode = jValue.GetBool();
                            }
                        }
                        if (jRadio.HasMember("tx_enable")) {
                            rapidjson::Value &jValue = jRadio["tx_enable"];
                            if (jValue.IsBool()) {
                                value.rfConfs[radioIndex].tx_enable = jValue.GetBool();
                            }
                        }
                        if (jRadio.HasMember("tx_freq_min")) {
                            rapidjson::Value &jValue = jRadio["tx_freq_min"];
                            if (jValue.IsUint()) {
                                value.tx_freq_min[radioIndex] = jValue.GetUint();
                            }
                        }
                        if (jRadio.HasMember("tx_freq_max")) {
                            rapidjson::Value &jValue = jRadio["tx_freq_max"];
                            if (jValue.IsUint()) {
                                value.tx_freq_max[radioIndex] = jValue.GetUint();
                            }
                        }
                        if (jRadio.HasMember("tx_gain_lut")) {
                            rapidjson::Value &jValue = jRadio["tx_gain_lut"];
                            if (jValue.IsArray()) {
                                size_t sz = jValue.Size();
                                if (sz > 16)
                                    sz = 16;
                                value.txLut[radioIndex].size = sz;

                                for (int i = 0; i < sz; i++) {
                                    rapidjson::Value &jGain = jValue[i];
                                    bool sx1250_tx_lut = false;
                                    if (jGain.IsObject()) {
                                        if (jGain.HasMember("pwr_idx")) {
                                            rapidjson::Value &jPwrIdx = jGain["pwr_idx"];
                                            if (jPwrIdx.IsUint()) {
                                                value.txLut[radioIndex].lut[i].pwr_idx = jPwrIdx.GetUint();
                                                sx1250_tx_lut = true;
                                            }
                                        }
                                        if (sx1250_tx_lut) {
                                            value.txLut[radioIndex].lut[i].mix_gain = 5;
                                        }
                                        // else {
                                        if (jGain.HasMember("rf_power")) {
                                            rapidjson::Value &jPower = jGain["rf_power"];
                                            if (jPower.IsInt()) {
                                                value.txLut[radioIndex].lut[i].rf_power = jPower.GetInt();
                                            }
                                        }
                                        if (jGain.HasMember("pa_gain")) {
                                            rapidjson::Value &jPaGain = jGain["pa_gain"];
                                            if (jPaGain.IsUint()) {
                                                value.txLut[radioIndex].lut[i].pa_gain = jPaGain.GetUint();
                                            }
                                        }
                                        if (jGain.HasMember("dig_gain")) {
                                            rapidjson::Value &jDigGain = jGain["dig_gain"];
                                            if (jDigGain.IsUint()) {
                                                value.txLut[radioIndex].lut[i].dig_gain = jDigGain.GetUint();
                                            }
                                        }
                                        if (jGain.HasMember("dac_gain")) {
                                            rapidjson::Value &jDacGain = jGain["dac_gain"];
                                            if (jDacGain.IsUint()) {
                                                value.txLut[radioIndex].lut[i].dac_gain = jDacGain.GetUint();
                                            }
                                        }
                                        if (jGain.HasMember("mix_gain")) {
                                            rapidjson::Value &jMixGain = jGain["mix_gain"];
                                            if (jMixGain.IsUint()) {
                                                value.txLut[radioIndex].lut[i].mix_gain = jMixGain.GetUint();
                                            }
                                        }
                                        // } // else
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
    }
    // demodulator
    // chan_multiSF_All
    bool hasSf = false;
    if (jsonValue.HasMember("chan_multiSF_All")) {
        rapidjson::Value &jDemod = jsonValue["chan_multiSF_All"];
        if (jDemod.IsObject()) {
            if (jDemod.HasMember("spreading_factor_enable")) {
                rapidjson::Value &jSFEnables = jDemod["spreading_factor_enable"];
                if (jSFEnables.IsArray()) {
                    size_t sz = jSFEnables.Size();
                    value.demodConf.multisf_datarate = 0;
                    hasSf = true;
                    for (int i = 0; i < sz; i++) {
                        uint8_t n = jSFEnables[i].GetUint();
                        value.demodConf.multisf_datarate |= (1 << (n - 5));
                    }
                }
            }
        }
    }
    if (!hasSf)
        value.demodConf.multisf_datarate = 0xff;  // all

    // set configuration for Lora multi-SF channels (bandwidth cannot be set)
    std::string cmsf = "chan_multiSF_0";
    for (int ch = 0; ch < LGW_MULTI_NB; ch++) {
        std::stringstream ssChannelName;
        cmsf[13] = '0' + ch;
        rapidjson::Value &jChannelSF = jsonValue[cmsf.c_str()];
        if (jChannelSF.IsObject()) {
            if (jChannelSF.HasMember("enable")) {
                rapidjson::Value &jChannelEnable = jChannelSF["enable"];
                if (jChannelEnable.IsBool()) {
                    value.ifConfs[ch].enable = jChannelEnable.GetBool();
                }

                if (jChannelSF.HasMember("radio")) {
                    rapidjson::Value &jChannelRadio = jChannelSF["radio"];
                    if (jChannelRadio.IsUint()) {
                        value.ifConfs[ch].rf_chain = jChannelRadio.GetUint();
                    }
                }
                if (jChannelSF.HasMember("if")) {
                    rapidjson::Value &jChain = jChannelSF["if"];
                    if (jChain.IsInt()) {
                        value.ifConfs[ch].freq_hz = jChain.GetInt();
                    }
                }
            }
        }
    }
    // set configuration for Lora standard channel
    value.ifStdConf.enable = false;
    if (jsonValue.HasMember("chan_Lora_std")) {
        rapidjson::Value &jChannelStd = jsonValue["chan_Lora_std"];
        if (jChannelStd.IsObject()) {
            if (jChannelStd.HasMember("enable")) {
                rapidjson::Value &jChannelEnable = jChannelStd["enable"];
                if (jChannelEnable.IsBool()) {
                    value.ifStdConf.enable = jChannelEnable.GetBool();
                }

                if (jChannelStd.HasMember("radio")) {
                    rapidjson::Value &jChannelRadio = jChannelStd["radio"];
                    if (jChannelRadio.IsUint()) {
                        value.ifStdConf.rf_chain = jChannelRadio.GetUint();
                    }
                }
                if (jChannelStd.HasMember("if")) {
                    rapidjson::Value &jFrequency = jChannelStd["if"];
                    if (jFrequency.IsInt()) {
                        value.ifStdConf.freq_hz = jFrequency.GetInt();
                    }
                }
                if (jChannelStd.HasMember("bandwidth")) {
                    rapidjson::Value &jBw = jChannelStd["bandwidth"];
                    if (jBw.IsUint()) {
                        uint32_t bw = jBw.GetUint();
                        switch(bw) {
                            case 500000:
                                value.ifStdConf.bandwidth = BW_500KHZ;
                                break;
                            case 250000:
                                value.ifStdConf.bandwidth = BW_250KHZ;
                                break;
                            case 125000:
                                value.ifStdConf.bandwidth = BW_125KHZ;
                                break;
                            default:
                                value.ifStdConf.bandwidth = BW_UNDEFINED;
                        }
                    }
                }
                if (jChannelStd.HasMember("spread_factor")) {
                    rapidjson::Value &jSF = jChannelStd["spread_factor"];
                    if (jSF.IsUint()) {
                        value.ifStdConf.datarate = jSF.GetUint();
                    }
                }
                if (jChannelStd.HasMember("implicit_hdr")) {
                    rapidjson::Value &jImplicitHeader = jChannelStd["implicit_hdr"];
                    if (jImplicitHeader.IsBool()) {
                        value.ifStdConf.implicit_hdr = jImplicitHeader.GetBool();
                    }
                    if (jChannelStd.HasMember("implicit_payload_length")) {
                        rapidjson::Value &jImplicitPayloadLength = jChannelStd["implicit_payload_length"];
                        if (jImplicitPayloadLength.IsUint()) {
                            value.ifStdConf.implicit_payload_length = jImplicitPayloadLength.GetUint();
                        }
                    }
                    if (jChannelStd.HasMember("implicit_crc_en")) {
                        rapidjson::Value &jImplicitCrcEn = jChannelStd["implicit_crc_en"];
                        if (jImplicitCrcEn.IsBool()) {
                            value.ifStdConf.implicit_crc_en = jImplicitCrcEn.GetBool();
                        }
                    }
                    if (jChannelStd.HasMember("implicit_coderate")) {
                        rapidjson::Value &jImplicitCrcEn = jChannelStd["implicit_coderate"];
                        if (jImplicitCrcEn.IsUint()) {
                            value.ifStdConf.implicit_coderate = jImplicitCrcEn.GetUint();
                        }
                    }
                }

            }
        }
    }

    // set configuration for FSK channel
    value.ifFSKConf.enable = false;
    if (jsonValue.HasMember("chan_FSK")) {
        rapidjson::Value &jChannelFSK = jsonValue["chan_FSK"];
        if (jChannelFSK.IsObject()) {
            if (jChannelFSK.HasMember("enable")) {
                rapidjson::Value &jChannelEnable = jChannelFSK["enable"];
                if (jChannelEnable.IsBool()) {
                    value.ifFSKConf.enable = jChannelEnable.GetBool();
                }

                if (jChannelFSK.HasMember("radio")) {
                    rapidjson::Value &jChannelRadio = jChannelFSK["radio"];
                    if (jChannelRadio.IsInt()) {
                        value.ifFSKConf.rf_chain = jChannelRadio.GetInt();
                    }
                }
                if (jChannelFSK.HasMember("if")) {
                    rapidjson::Value &jChain = jChannelFSK["if"];
                    if (jChain.IsUint()) {
                        value.ifFSKConf.freq_hz = jChain.GetUint();
                    }
                }

                uint32_t bw = 0;
                if (jChannelFSK.HasMember("bandwidth")) {
                    rapidjson::Value &jBw = jChannelFSK["bandwidth"];
                    if (jBw.IsUint()) {
                        bw = jBw.GetUint();
                    }
                }

                uint32_t frequencyDeviation = 0;
                if (jChannelFSK.HasMember("freq_deviation")) {
                    rapidjson::Value &jFreqDeviation = jChannelFSK["freq_deviation"];
                    if (jFreqDeviation.IsUint()) {
                        frequencyDeviation = jFreqDeviation.GetUint();
                    }
                }

                if (jChannelFSK.HasMember("datarate")) {
                    rapidjson::Value &jDataRate = jChannelFSK["datarate"];
                    if (jDataRate.IsUint()) {
                        value.ifFSKConf.datarate = jDataRate.GetUint();
                    }
                }

                if ((bw == 0) && (frequencyDeviation != 0)) {
                    bw = 2 * frequencyDeviation + value.ifFSKConf.datarate;
                }
                if (bw == 0)
                    value.ifFSKConf.bandwidth = BW_UNDEFINED;
                else
                    if (bw <= 125000)
                        value.ifFSKConf.bandwidth = BW_125KHZ;
                    else
                        if (bw <= 250000)
                            value.ifFSKConf.bandwidth = BW_250KHZ;
                        else
                            if (bw <= 500000)
                                value.ifFSKConf.bandwidth = BW_500KHZ;
                            else
                                value.ifFSKConf.bandwidth = BW_UNDEFINED;
            }
        }
    }
    return 0;
}

static const char* lgw_com_type_t2string(lgw_com_type_t typ) {
    switch (typ) {
        case LGW_COM_SPI:
            return "SPI";
        case LGW_COM_USB:
            return "USB";
        default:
            return "unknown";
    }
}

static const char* lgw_ftime_mode_t2string(
    lgw_ftime_mode_t mode
) {
    switch (mode) {
        case LGW_FTIME_MODE_HIGH_CAPACITY:
            return "high_capacity";
        default:
            return "all_sf";
    }
}

static const char *lgw_radio_type_t2string(
    lgw_radio_type_t typ
) {
    switch (typ) {
        case LGW_RADIO_TYPE_SX1255:
            return "SX1255";
        case LGW_RADIO_TYPE_SX1257:
            return "SX1257";
        case LGW_RADIO_TYPE_SX1272:
            return "SX1272";
        case LGW_RADIO_TYPE_SX1276:
            return "SX1276";
        case LGW_RADIO_TYPE_SX1250:
            return "SX1250";
        default:
            return "none";
    }
}

void GatewaySX130xConfig::toCpp(
    std::ostream &retVal,
    const std::string &name
) const
{
    retVal << std::endl << "// SX130x " << std::endl << std::endl;
    ADD_CAST(name, value, boardConf.com_type, lgw_com_type_t)
    ADD_PCHAR(name, value, boardConf.com_path)
    ADD_BOOL(name, value, boardConf.lorawan_public)
    ADD_INT(name, value, boardConf.clksrc)
    ADD_INT(name, value, antennaGain)
    ADD_BOOL(name, value, boardConf.full_duplex)
    ADD_BOOL(name, value, tsConf.enable)
    ADD_CAST(name, value, tsConf.mode, lgw_ftime_mode_t)
    for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
        retVal << std::endl << "// Radio " << radioIndex << std::endl << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].enable = " << (value.rfConfs[radioIndex].enable ? "true" : "false") << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].type = (lgw_radio_type_t) " << value.rfConfs[radioIndex].type << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].freq_hz = " << value.rfConfs[radioIndex].freq_hz << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].rssi_offset = " << value.rfConfs[radioIndex].rssi_offset << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].rssi_tcomp.coeff_a = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_a << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].rssi_tcomp.coeff_b = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_b << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].rssi_tcomp.coeff_c = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_c << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].rssi_tcomp.coeff_d = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_d << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].rssi_tcomp.coeff_e = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_e << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].tx_enable = " << (value.rfConfs[radioIndex].tx_enable ? "true" : "false") << ";" << std::endl;
        retVal << name << ".rfConfs[" << radioIndex << "].single_input_mode = " << (value.rfConfs[radioIndex].single_input_mode ? "true" : "false") << ";" << std::endl;
        retVal << name << ".tx_freq_min[" << radioIndex << "] = " << value.tx_freq_min[radioIndex] << ";" << std::endl;
        retVal << name << ".tx_freq_max[" << radioIndex << "] = " << value.tx_freq_max[radioIndex] << ";" << std::endl;

        retVal << name << ".txLut[" << radioIndex << "].size = " << (int) value.txLut[radioIndex].size << ";" << std::endl;
        retVal << std::endl << "// TX LUT radio " << radioIndex << ", count: " << (int) value.txLut[radioIndex].size  << std::endl << std::endl;
        for (int i = 0; i < value.txLut[radioIndex].size; i++) {
            retVal << name << ".txLut[" << radioIndex << "].lut[" << i << "].rf_power = " << (int) value.txLut[radioIndex].lut[i].rf_power << ";" << std::endl;
            retVal << name << ".txLut[" << radioIndex << "].lut[" << i << "].pa_gain = " << (int) value.txLut[radioIndex].lut[i].pa_gain << ";" << std::endl;
            retVal << name << ".txLut[" << radioIndex << "].lut[" << i << "].pwr_idx = " << (int) value.txLut[radioIndex].lut[i].pwr_idx << ";" << std::endl;
            retVal << name << ".txLut[" << radioIndex << "].lut[" << i << "].dig_gain = " << (int) value.txLut[radioIndex].lut[i].dig_gain << ";" << std::endl;
            retVal << name << ".txLut[" << radioIndex << "].lut[" << i << "].dac_gain = " << (int) value.txLut[radioIndex].lut[i].dac_gain << ";" << std::endl;
            retVal << name << ".txLut[" << radioIndex << "].lut[" << i << "].mix_gain = " << (int) value.txLut[radioIndex].lut[i].mix_gain << ";" << std::endl;
        }
    }

    retVal << std::endl << "// chan_multiSF_All spreading_factor_enable bit field" << std::endl << std::endl;
    retVal << name << ".demodConf.multisf_datarate = 0x" << std::hex << (int) value.demodConf.multisf_datarate
        << ";\t // " << std::dec << (int) value.demodConf.multisf_datarate << std::endl;

    for (unsigned char ch = 0; ch < LGW_MULTI_NB; ch++) {
        retVal  << "// chan_multiSF_" << (int) ch << std::endl;
        retVal << name << ".ifConfs[" << (int) ch << "].enable = " << (value.ifConfs[ch].enable ? "true" : "false") << ";" << std::endl;
        retVal << name << ".ifConfs[" << (int) ch << "].rf_chain = " << (int) value.ifConfs[ch].rf_chain  << ";" << std::endl;
        retVal << name << ".ifConfs[" << (int) ch << "].freq_hz = " << value.ifConfs[ch].freq_hz  << ";" << std::endl;
    }

    retVal  << "// Lora std " << std::endl;

    rapidjson::Value jChanLoraStd;
    jChanLoraStd.SetObject();

    retVal << name << ".ifStdConf.enable = " << (value.ifStdConf.enable ? "true" : "false") << ";" << std::endl;
    retVal << name << ".ifStdConf.rf_chain = " << (int) value.ifStdConf.rf_chain << ";" << std::endl;
    retVal << name << ".ifStdConf.freq_hz = " << value.ifStdConf.freq_hz << ";" << std::endl;
    retVal << name << ".ifStdConf.bandwidth = " << (int) value.ifStdConf.bandwidth << ";" << std::endl;
    retVal << name << ".ifStdConf.datarate = " << value.ifStdConf.datarate << ";" << std::endl;
    retVal << name << ".ifStdConf.implicit_hdr = " << (value.ifStdConf.implicit_hdr ? "true" : "false") << ";" << std::endl;
    retVal << name << ".ifStdConf.implicit_payload_length = " << (int) value.ifStdConf.implicit_payload_length << ";" << std::endl;
    retVal << name << ".ifStdConf.implicit_crc_en = " << (value.ifStdConf.implicit_crc_en ? "true" : "false") << ";" << std::endl;
    retVal << name << ".ifStdConf.implicit_coderate = " << (int) value.ifStdConf.implicit_coderate << ";" << std::endl;

    retVal  << "// FSK " << std::endl;
    retVal << name << ".ifFSKConf.enable = " << (value.ifFSKConf.enable ? "true" : "false") << ";" << std::endl;
    retVal << name << ".ifFSKConf.rf_chain = " << (int) value.ifFSKConf.rf_chain << ";" << std::endl;
    retVal << name << ".ifFSKConf.freq_hz = " << value.ifFSKConf.freq_hz << ";" << std::endl;
    retVal << name << ".ifFSKConf.bandwidth = " << (int) value.ifFSKConf.bandwidth << ";" << std::endl;
    retVal << name << ".ifFSKConf.datarate = " << value.ifFSKConf.datarate << ";" << std::endl;
}

void GatewaySX130xConfig::toJSON(
    rapidjson::Value &jsonValue,
    rapidjson::Document::AllocatorType& allocator
) const {
    jsonValue.SetObject();

    rapidjson::Value jSx1261;
    sx1261Config.toJSON(jSx1261, allocator);
    jsonValue.AddMember("sx1261_conf", jSx1261, allocator);

    rapidjson::Value jComType;
    std::string sComType = lgw_com_type_t2string(value.boardConf.com_type);
    jComType.SetString(sComType.c_str(), sComType.size(), allocator);
    jsonValue.AddMember("com_type", jComType, allocator);

    rapidjson::Value jComPath;
    jComPath.SetString(value.boardConf.com_path, allocator);
    jsonValue.AddMember("com_path", jComPath, allocator);

    rapidjson::Value jLorawanPublic;
    jLorawanPublic.SetBool(value.boardConf.lorawan_public);
    jsonValue.AddMember("lorawan_public", jLorawanPublic, allocator);

    rapidjson::Value jClcSrc;
    jClcSrc.SetUint(value.boardConf.clksrc);
    jsonValue.AddMember("clksrc", jClcSrc, allocator);

    rapidjson::Value jAntennaGain;
    jAntennaGain.SetInt(value.antennaGain);
    jsonValue.AddMember("antenna_gain", jAntennaGain, allocator);

    rapidjson::Value jFullDuplex;
    jFullDuplex.SetBool(value.boardConf.full_duplex);
    jsonValue.AddMember("full_duplex", jFullDuplex, allocator);

    rapidjson::Value jFineTimestamp;
    jFineTimestamp.SetObject();
    rapidjson::Value jFineTimestampEnable;
    jFineTimestampEnable.SetBool(value.tsConf.enable);
    jFineTimestamp.AddMember("enable", jFineTimestampEnable, allocator);
    rapidjson::Value jMode;
    std::string sMode = lgw_ftime_mode_t2string(value.tsConf.mode);
    jMode.SetString(sMode.c_str(), sMode.size(), allocator);
    jFineTimestamp.AddMember("mode", jMode, allocator);
    jsonValue.AddMember("fine_timestamp", jFineTimestamp, allocator);

    std::string ridx = "radio_0";
    for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
        rapidjson::Value jRadio;
        jRadio.SetObject();

        rapidjson::Value jRadioEnable;
        jRadioEnable.SetBool(value.rfConfs[radioIndex].enable);
        jRadio.AddMember("enable", jRadioEnable, allocator);

        rapidjson::Value jRadioType;
        std::string sType = lgw_radio_type_t2string(value.rfConfs[radioIndex].type);
        jRadioType.SetString(sType.c_str(), sType.size(), allocator);
        jRadio.AddMember("type", jRadioType, allocator);

        rapidjson::Value jRadioFreq;
        jRadioFreq.SetUint(value.rfConfs[radioIndex].freq_hz);
        jRadio.AddMember("freq", jRadioFreq, allocator);

        rapidjson::Value jRadioRssiOffset;
        jRadioRssiOffset.SetDouble(value.rfConfs[radioIndex].rssi_offset);
        jRadio.AddMember("rssi_offset", jRadioRssiOffset, allocator);

        rapidjson::Value jRadioRssiTcomp;
        jRadioRssiTcomp.SetObject();

        rapidjson::Value jRadioRssiTcompCoeff_a;
        jRadioRssiTcompCoeff_a.SetDouble(value.rfConfs[radioIndex].rssi_tcomp.coeff_a);
        jRadioRssiTcomp.AddMember("coeff_a", jRadioRssiTcompCoeff_a, allocator);
        rapidjson::Value jRadioRssiTcompCoeff_b;
        jRadioRssiTcompCoeff_b.SetDouble(value.rfConfs[radioIndex].rssi_tcomp.coeff_b);
        jRadioRssiTcomp.AddMember("coeff_b", jRadioRssiTcompCoeff_b, allocator);
        rapidjson::Value jRadioRssiTcompCoeff_c;
        jRadioRssiTcompCoeff_c.SetDouble(value.rfConfs[radioIndex].rssi_tcomp.coeff_c);
        jRadioRssiTcomp.AddMember("coeff_c", jRadioRssiTcompCoeff_c, allocator);
        rapidjson::Value jRadioRssiTcompCoeff_d;
        jRadioRssiTcompCoeff_d.SetDouble(value.rfConfs[radioIndex].rssi_tcomp.coeff_d);
        jRadioRssiTcomp.AddMember("coeff_d", jRadioRssiTcompCoeff_d, allocator);
        rapidjson::Value jRadioRssiTcompCoeff_e;
        jRadioRssiTcompCoeff_e.SetDouble(value.rfConfs[radioIndex].rssi_tcomp.coeff_e);
        jRadioRssiTcomp.AddMember("coeff_e", jRadioRssiTcompCoeff_e, allocator);

        jRadio.AddMember("rssi_tcomp", jRadioRssiTcomp, allocator);

        rapidjson::Value jRadioTxEnable;
        jRadioTxEnable.SetBool(value.rfConfs[radioIndex].tx_enable);
        jRadio.AddMember("tx_enable", jRadioTxEnable, allocator);

        rapidjson::Value jRadioSingleInputMode;
        jRadioSingleInputMode.SetBool(value.rfConfs[radioIndex].single_input_mode);
        jRadio.AddMember("single_input_mode", jRadioSingleInputMode, allocator);

        rapidjson::Value jRadioFreqMin;
        jRadioFreqMin.SetUint(value.tx_freq_min[radioIndex]);
        jRadio.AddMember("tx_freq_min", jRadioFreqMin, allocator);

        rapidjson::Value jRadioFreqMax;
        jRadioFreqMax.SetUint(value.tx_freq_max[radioIndex]);
        jRadio.AddMember("tx_freq_max", jRadioFreqMax, allocator);

        rapidjson::Value jRadioTxGainLuts;
        jRadioTxGainLuts.SetArray();
        for (int i = 0; i < value.txLut[radioIndex].size; i++) {
            rapidjson::Value jRadioTxGainLut;
            jRadioTxGainLut.SetObject();

            rapidjson::Value jRadioTxGainLutRfPower;
            jRadioTxGainLutRfPower.SetInt(value.txLut[radioIndex].lut[i].rf_power);
            jRadioTxGainLut.AddMember("rf_power", jRadioTxGainLutRfPower, allocator);

            rapidjson::Value jRadioTxGainLutPaGain;
            jRadioTxGainLutPaGain.SetUint(value.txLut[radioIndex].lut[i].pa_gain);
            jRadioTxGainLut.AddMember("pa_gain", jRadioTxGainLutPaGain, allocator);

            rapidjson::Value jRadioTxGainLutPwrIdx;
            jRadioTxGainLutPwrIdx.SetUint(value.txLut[radioIndex].lut[i].pwr_idx);
            jRadioTxGainLut.AddMember("pwr_idx", jRadioTxGainLutPwrIdx, allocator);

            rapidjson::Value jRadioTxGainLutDigGain;
            jRadioTxGainLutDigGain.SetUint(value.txLut[radioIndex].lut[i].dig_gain);
            jRadioTxGainLut.AddMember("dig_gain", jRadioTxGainLutDigGain, allocator);

            rapidjson::Value jRadioTxGainLutDacGain;
            jRadioTxGainLutDacGain.SetUint(value.txLut[radioIndex].lut[i].dac_gain);
            jRadioTxGainLut.AddMember("dac_gain", jRadioTxGainLutDacGain, allocator);

            rapidjson::Value jRadioTxGainLutMixGain;
            jRadioTxGainLutMixGain.SetUint(value.txLut[radioIndex].lut[i].mix_gain);
            jRadioTxGainLut.AddMember("mix_gain", jRadioTxGainLutMixGain, allocator);

            jRadioTxGainLuts.PushBack(jRadioTxGainLut, allocator);
        }
        jRadio.AddMember("tx_gain_lut", jRadioTxGainLuts, allocator);

        ridx[6] = '0' + radioIndex;
        rapidjson::Value nRadio(ridx.c_str(),ridx.length(), allocator);
        jsonValue.AddMember(nRadio, jRadio, allocator);
    }

    rapidjson::Value jChanMultiSFAll;
    jChanMultiSFAll.SetObject();
    rapidjson::Value jSpreadingFactorEnables;
    jSpreadingFactorEnables.SetArray();
    for (int n = 0; n < 8; n++) {
        if (value.demodConf.multisf_datarate & (1 << n)) {
            rapidjson::Value jN5;
            jN5.SetUint(n + 5);
            jSpreadingFactorEnables.PushBack(jN5, allocator);
        }
    }
    jChanMultiSFAll.AddMember("spreading_factor_enable", jSpreadingFactorEnables, allocator);

    std::string cmsfn = "chan_multiSF_0";
    for (unsigned char ch = 0; ch < LGW_MULTI_NB; ch++) {
        cmsfn[13] = ch + '0';

        rapidjson::Value jChannelSF;
        jChannelSF.SetObject();

        rapidjson::Value jChannelEnable;
        jChannelEnable.SetBool(value.ifConfs[ch].enable);
        jChannelSF.AddMember("enable", jChannelEnable, allocator);

        rapidjson::Value jChannelRadio;
        jChannelRadio.SetUint(value.ifConfs[ch].rf_chain);
        jChannelSF.AddMember("radio", jChannelRadio, allocator);

        rapidjson::Value jChannelIf;
        jChannelIf.SetInt(value.ifConfs[ch].freq_hz);
        jChannelSF.AddMember("if", jChannelIf, allocator);

        rapidjson::Value n;
        n.SetString(cmsfn.c_str(), cmsfn.size(), allocator);
        jsonValue.AddMember(n, jChannelSF, allocator);
    }

    jsonValue.AddMember("chan_multiSF_All", jChanMultiSFAll, allocator);

    // Lora std
    rapidjson::Value jChanLoraStd;
    jChanLoraStd.SetObject();

    rapidjson::Value jChannelEnable;
    jChannelEnable.SetBool(value.ifStdConf.enable);
    jChanLoraStd.AddMember("enable", jChannelEnable, allocator);

    rapidjson::Value jChannelRadio;
    jChannelRadio.SetUint(value.ifStdConf.rf_chain);
    jChanLoraStd.AddMember("radio", jChannelRadio, allocator);

    rapidjson::Value jChannelIf;
    jChannelIf.SetInt(value.ifStdConf.freq_hz);
    jChanLoraStd.AddMember("if", jChannelIf, allocator);

    rapidjson::Value jChannelBandwidth;
    jChannelBandwidth.SetInt(bandwidthIndex2hz(value.ifStdConf.bandwidth));
    jChanLoraStd.AddMember("bandwidth", jChannelBandwidth, allocator);

    rapidjson::Value jChannelSf;
    jChannelSf.SetUint(value.ifStdConf.datarate);
    jChanLoraStd.AddMember("spread_factor", jChannelSf, allocator);

    rapidjson::Value jImplicitHdr;
    jImplicitHdr.SetBool(value.ifStdConf.implicit_hdr);
    jChanLoraStd.AddMember("implicit_hdr", jImplicitHdr, allocator);

    rapidjson::Value jImplicitPayloadLength;
    jImplicitPayloadLength.SetUint(value.ifStdConf.implicit_payload_length);
    jChanLoraStd.AddMember("implicit_payload_length", jImplicitPayloadLength, allocator);

    rapidjson::Value jImplicitCrcEn;
    jImplicitCrcEn.SetBool(value.ifStdConf.implicit_crc_en);
    jChanLoraStd.AddMember("implicit_crc_en", jImplicitCrcEn, allocator);

    rapidjson::Value jImplicitCodeRate;
    jImplicitCodeRate.SetUint(value.ifStdConf.implicit_coderate);
    jChanLoraStd.AddMember("implicit_coderate", jImplicitCodeRate, allocator);

    jsonValue.AddMember("chan_Lora_std", jChanLoraStd, allocator);

    rapidjson::Value jChannelFSK;
    jChannelFSK.SetObject();

    rapidjson::Value jChannelFSKEnable;
    jChannelFSKEnable.SetBool(value.ifFSKConf.enable);
    jChannelFSK.AddMember("enable", jChannelFSKEnable, allocator);

    rapidjson::Value jChannelFSKRadio;
    jChannelFSKRadio.SetUint(value.ifFSKConf.rf_chain);
    jChannelFSK.AddMember("radio", jChannelFSKRadio, allocator);

    rapidjson::Value jChannelFSKIf;
    jChannelFSKIf.SetInt(value.ifFSKConf.freq_hz);
    jChannelFSK.AddMember("if", jChannelFSKIf, allocator);

    rapidjson::Value jChannelFSKBandwidth;
    jChannelFSKBandwidth.SetInt(bandwidthIndex2hz(value.ifFSKConf.bandwidth));
    jChannelFSK.AddMember("bandwidth", jChannelFSKBandwidth, allocator);

    rapidjson::Value jChannelFSKDatarate;
    jChannelFSKDatarate.SetUint(value.ifFSKConf.datarate);
    jChannelFSK.AddMember("datarate", jChannelFSKDatarate, allocator);

    jsonValue.AddMember("chan_FSK", jChannelFSK, allocator);
}

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

GatewayGatewayConfig::GatewayGatewayConfig()
{
    reset();
}

void GatewayGatewayConfig::reset()
{
    value.gatewayId = 0;
    value.serverPortUp = 0;
    value.serverPortDown = 0;
    value.keepaliveInterval = 0;
    value.statInterval = DEFAULT_KEEPALIVE;
    value.forwardCRCValid = false;
    value.forwardCRCError = false;
    value.forwardCRCDisabled = false;
    value.gpsEnabled = false;
    value.fakeGPS = false;
    value.beaconPeriod = 0;
    value.beaconFreqHz = DEFAULT_BEACON_FREQ_HZ;
    value.beaconFreqNb = DEFAULT_BEACON_FREQ_NB;
    value.beaconFreqStep = 0;
    value.beaconDataRate = DEFAULT_BEACON_DATARATE;
    value.beaconBandwidthHz = DEFAULT_BEACON_BW_HZ;
    value.beaconPower = DEFAULT_BEACON_POWER;
    value.beaconInfoDesc = 0;
    value.autoQuitThreshold = 0;

    value.pushTimeoutMs.tv_sec = 0;
    value.pushTimeoutMs.tv_usec = 0;
    value.refGeoCoordinates.lat = 0.0;
    value.refGeoCoordinates.lon = 0.0;
    value.refGeoCoordinates.alt = 0;
}

int GatewayGatewayConfig::parse(rapidjson::Value &jsonValue)
{
    reset();
    if (jsonValue.HasMember("gateway_ID")) {
        rapidjson::Value &jGatewayId = jsonValue["gateway_ID"];
        if (jGatewayId.IsString()) {
            value.gatewayId = std::stoull(jGatewayId.GetString(), nullptr, 16);
        }
    }
    if (jsonValue.HasMember("server_address")) {
        rapidjson::Value &jServerAddress = jsonValue["server_address"];
        if (jServerAddress.IsString()) {
            serverAddr = jServerAddress.GetString();
        }
    }
    if (jsonValue.HasMember("serv_port_up")) {
        rapidjson::Value &jServerPortUp = jsonValue["serv_port_up"];
        if (jServerPortUp.IsUint()) {
            value.serverPortUp = jServerPortUp.GetUint();
        }
    }
    if (jsonValue.HasMember("serv_port_down")) {
        rapidjson::Value &jServerPortDown = jsonValue["serv_port_down"];
        if (jServerPortDown.IsUint()) {
            value.serverPortDown = jServerPortDown.GetUint();
        }
    }
    value.keepaliveInterval = DEFAULT_KEEPALIVE;
    if (jsonValue.HasMember("keepalive_interval")) {
        rapidjson::Value &jKeepaliveInterval = jsonValue["keepalive_interval"];
        if (jKeepaliveInterval.IsInt()) {
            value.keepaliveInterval = jKeepaliveInterval.GetInt();
        }
    }
    if (jsonValue.HasMember("stat_interval")) {
        rapidjson::Value &jStatInterval = jsonValue["stat_interval"];
        if (jStatInterval.IsInt()) {
            value.statInterval = jStatInterval.GetInt();
        }
    }
    if (jsonValue.HasMember("push_timeout_ms")) {
        rapidjson::Value &jPushTimeoutMs = jsonValue["push_timeout_ms"];
        if (jPushTimeoutMs.IsInt()) {
            value.pushTimeoutMs.tv_sec = 0;
            value.pushTimeoutMs.tv_usec = jPushTimeoutMs.GetInt() * 500;
        }
    }
    if (jsonValue.HasMember("forward_crc_valid")) {
        rapidjson::Value &jForwardCRCValid = jsonValue["forward_crc_valid"];
        if (jForwardCRCValid.IsBool()) {
            value.forwardCRCValid = jForwardCRCValid.GetBool();
        }
    }
    if (jsonValue.HasMember("forward_crc_error")) {
        rapidjson::Value &jForwardCRCError = jsonValue["forward_crc_error"];
        if (jForwardCRCError.IsBool()) {
            value.forwardCRCError = jForwardCRCError.GetBool();
        }
    }
    if (jsonValue.HasMember("forward_crc_disabled")) {
        rapidjson::Value &jForwardCRCDisabled = jsonValue["forward_crc_disabled"];
        if (jForwardCRCDisabled.IsBool()) {
            value.forwardCRCDisabled = jForwardCRCDisabled.GetBool();
        }
    }
    if (jsonValue.HasMember("gps_tty_path")) {
        rapidjson::Value &jGpsTTYPath = jsonValue["gps_tty_path"];
        if (jGpsTTYPath.IsString()) {
            gpsTtyPath = jGpsTTYPath.GetString();
            value.gpsEnabled = !gpsTtyPath.empty();
        }
    }
    if (jsonValue.HasMember("ref_latitude")) {
        rapidjson::Value &jRefCoord = jsonValue["ref_latitude"];
        if (jRefCoord.IsDouble()) {
            value.refGeoCoordinates.lat = jRefCoord.GetDouble();
        }
    }
    if (jsonValue.HasMember("ref_longitude")) {
        rapidjson::Value &jRefCoord = jsonValue["ref_longitude"];
        if (jRefCoord.IsDouble()) {
            value.refGeoCoordinates.lon = jRefCoord.GetDouble();
        }
    }
    if (jsonValue.HasMember("ref_altitude")) {
        rapidjson::Value &jRefCoord = jsonValue["ref_altitude"];
        if (jRefCoord.IsInt()) {
            value.refGeoCoordinates.alt = jRefCoord.GetInt();
        }
    }
    if (jsonValue.HasMember("fake_gps")) {
        rapidjson::Value &jForwardCRCDisabled = jsonValue["fake_gps"];
        if (jForwardCRCDisabled.IsBool()) {
            value.fakeGPS = jForwardCRCDisabled.GetBool();
        }
    }
    if (jsonValue.HasMember("beacon_period")) {
        rapidjson::Value &jValue = jsonValue["beacon_period"];
        if (jValue.IsInt()) {
            value.beaconPeriod = jValue.GetInt();
        }
    }
    value.beaconFreqHz = DEFAULT_BEACON_FREQ_HZ;
    if (jsonValue.HasMember("beacon_freq_hz")) {
        rapidjson::Value &jValue = jsonValue["beacon_freq_hz"];
        if (jValue.IsInt()) {
            value.beaconFreqHz = jValue.GetInt();
        }
    }
    value.beaconFreqNb = DEFAULT_BEACON_FREQ_NB;
    if (jsonValue.HasMember("beacon_freq_nb")) {
        rapidjson::Value &jValue = jsonValue["beacon_freq_nb"];
        if (jValue.IsInt()) {
            value.beaconFreqNb = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beacon_freq_step")) {
        rapidjson::Value &jValue = jsonValue["beacon_freq_step"];
        if (jValue.IsInt()) {
            value.beaconFreqStep = jValue.GetInt();
        }
    }
    value.beaconDataRate = DEFAULT_BEACON_DATARATE;
    if (jsonValue.HasMember("beacon_datarate")) {
        rapidjson::Value &jValue = jsonValue["beacon_datarate"];
        if (jValue.IsInt()) {
            value.beaconDataRate = jValue.GetInt();
        }
    }
    value.beaconBandwidthHz = DEFAULT_BEACON_BW_HZ;
    if (jsonValue.HasMember("beacon_bw_hz")) {
        rapidjson::Value &jValue = jsonValue["beacon_bw_hz"];
        if (jValue.IsInt()) {
            value.beaconBandwidthHz = jValue.GetInt();
        }
    }
    value.beaconPower = DEFAULT_BEACON_POWER;
    if (jsonValue.HasMember("beacon_power")) {
        rapidjson::Value &jValue = jsonValue["beacon_power"];
        if (jValue.IsInt()) {
            value.beaconPower = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("beacon_infodesc")) {
        rapidjson::Value &jValue = jsonValue["beacon_infodesc"];
        if (jValue.IsInt()) {
            value.beaconInfoDesc = jValue.GetInt();
        }
    }
    if (jsonValue.HasMember("autoquit_threshold")) {
        rapidjson::Value &jValue = jsonValue["autoquit_threshold"];
        if (jValue.IsInt()) {
            value.autoQuitThreshold = jValue.GetInt();
        }
    }
    return LORA_OK;
}

void GatewayGatewayConfig::toCpp(
        std::ostream &retVal,
        const std::string &name
) const
{
    std::stringstream ssGatewayId;
    retVal << std::endl << "// Gateway " << std::endl << std::endl;
    retVal << name << ".gatewayId = 0x" << std::hex << value.gatewayId << ";" << std::dec << std::endl;
    ADD_VAL(name, value, serverPortUp)
    ADD_VAL(name, value, serverPortDown)
    ADD_VAL(name, value, keepaliveInterval)
    ADD_VAL(name, value, statInterval)
    ADD_VAL(name, value, pushTimeoutMs.tv_sec)
    ADD_VAL(name, value, pushTimeoutMs.tv_usec)
    ADD_BOOL(name, value, forwardCRCValid)
    ADD_BOOL(name, value, forwardCRCError)
    ADD_BOOL(name, value, forwardCRCDisabled)
    ADD_VAL(name, value, refGeoCoordinates.lat)
    ADD_VAL(name, value, refGeoCoordinates.lon)
    ADD_VAL(name, value, refGeoCoordinates.alt)
    ADD_BOOL(name, value, fakeGPS)
    ADD_VAL(name, value, beaconPeriod)
    ADD_VAL(name, value, beaconFreqHz)
    ADD_INT(name, value, beaconFreqNb)
    ADD_VAL(name, value, beaconFreqStep)
    ADD_INT(name, value, beaconDataRate)
    ADD_VAL(name, value, beaconBandwidthHz)
    ADD_INT(name, value, beaconInfoDesc)
    ADD_VAL(name, value, autoQuitThreshold)
}

void GatewayGatewayConfig::toJSON(
    rapidjson::Value &jsonValue,
    rapidjson::Document::AllocatorType& allocator
) const
{
    jsonValue.SetObject();
    rapidjson::Value jgatewayId;
    std::stringstream ssGatewayId;
    ssGatewayId << std::hex << std::setw(16) << std::setfill('0') << value.gatewayId << std::dec;
    std::string sGatewayId = ssGatewayId.str();
    jgatewayId.SetString(sGatewayId.c_str(), sGatewayId.size(), allocator);
    jsonValue.AddMember("gateway_ID", jgatewayId, allocator);

    rapidjson::Value jserverAddress;
    jserverAddress.SetString(serverAddr.c_str(), serverAddr.size(), allocator);
    jsonValue.AddMember("server_address", jserverAddress, allocator);

    rapidjson::Value jserverPortUp;
    jserverPortUp.SetUint(value.serverPortUp);
    jsonValue.AddMember("serv_port_up", jserverPortUp, allocator);

    rapidjson::Value jserverPortDown;
    jserverPortDown.SetUint(value.serverPortDown);
    jsonValue.AddMember("serv_port_down", jserverPortDown, allocator);

    rapidjson::Value jkeepaliveInterval;
    jkeepaliveInterval.SetUint(value.keepaliveInterval);
    jsonValue.AddMember("keepalive_interval", jkeepaliveInterval, allocator);

    rapidjson::Value jstatInterval;
    jstatInterval.SetUint(value.statInterval);
    jsonValue.AddMember("stat_interval", jstatInterval, allocator);

    rapidjson::Value jpushTimeoutMs;
    jpushTimeoutMs.SetUint(value.pushTimeoutMs.tv_usec / 500);
    jsonValue.AddMember("push_timeout_ms", jpushTimeoutMs, allocator);

    rapidjson::Value jforwardCRCValid;
    jforwardCRCValid.SetBool(value.forwardCRCValid);
    jsonValue.AddMember("forward_crc_valid", jforwardCRCValid, allocator);

    rapidjson::Value jforwardCRCError;
    jforwardCRCError.SetBool(value.forwardCRCError);
    jsonValue.AddMember("forward_crc_error", jforwardCRCError, allocator);

    rapidjson::Value jforwardCRCDisabled;
    jforwardCRCDisabled.SetBool(value.forwardCRCDisabled);
    jsonValue.AddMember("forward_crc_disabled", jforwardCRCDisabled, allocator);

    rapidjson::Value jgpsTTYPath;
    jgpsTTYPath.SetString(gpsTtyPath.c_str(), gpsTtyPath.size(),allocator);
    jsonValue.AddMember("gps_tty_path", jgpsTTYPath, allocator);

    rapidjson::Value jrefGeoCoordinatesLat;
    jrefGeoCoordinatesLat.SetDouble(value.refGeoCoordinates.lat);
    jsonValue.AddMember("ref_latitude", jrefGeoCoordinatesLat, allocator);

    rapidjson::Value jrefGeoCoordinatesLon;
    jrefGeoCoordinatesLon.SetDouble(value.refGeoCoordinates.lon);
    jsonValue.AddMember("ref_longitude", jrefGeoCoordinatesLon, allocator);

    rapidjson::Value jrefGeoCoordinatesAlt;
    jrefGeoCoordinatesAlt.SetInt(value.refGeoCoordinates.alt);
    jsonValue.AddMember("ref_altitude", jrefGeoCoordinatesAlt, allocator);

    rapidjson::Value jfakeGPS;
    jfakeGPS.SetBool(value.fakeGPS);
    jsonValue.AddMember("fake_gps", jfakeGPS, allocator);

    rapidjson::Value jbeaconPeriod;
    jbeaconPeriod.SetUint(value.beaconPeriod);
    jsonValue.AddMember("beacon_period", jbeaconPeriod, allocator);

    rapidjson::Value jbeaconFreqHz;
    jbeaconFreqHz.SetUint(value.beaconFreqHz);
    jsonValue.AddMember("beacon_freq_hz", jbeaconFreqHz, allocator);

    rapidjson::Value jbeaconFreqNb;
    jbeaconFreqNb.SetUint(value.beaconFreqNb);
    jsonValue.AddMember("beacon_freq_nb", jbeaconFreqNb, allocator);

    rapidjson::Value jbeaconFreqStep;
    jbeaconFreqStep.SetUint(value.beaconFreqStep);
    jsonValue.AddMember("beacon_freq_step", jbeaconFreqStep, allocator);

    rapidjson::Value jbeaconDatarate;
    jbeaconDatarate.SetUint(value.beaconDataRate);
    jsonValue.AddMember("beacon_datarate", jbeaconDatarate, allocator);

    rapidjson::Value jbeaconBandwidthHz;
    jbeaconBandwidthHz.SetUint(value.beaconBandwidthHz);
    jsonValue.AddMember("beacon_bw_hz", jbeaconBandwidthHz, allocator);

    rapidjson::Value jbeaconPower;
    jbeaconPower.SetUint(value.beaconPower);
    jsonValue.AddMember("beacon_power", jbeaconPower, allocator);

    rapidjson::Value jbeaconInfodesc;
    jbeaconInfodesc.SetUint(value.beaconInfoDesc);
    jsonValue.AddMember("beacon_infodesc", jbeaconInfodesc, allocator);

    rapidjson::Value jautoquitThreshold;
    jautoquitThreshold.SetUint(value.autoQuitThreshold);
    jsonValue.AddMember("autoquit_threshold", jautoquitThreshold, allocator);
}

bool GatewayGatewayConfig::operator==(const GatewayGatewayConfig &b) const
{
    return value.gatewayId == b.value.gatewayId
        && serverAddr == b.serverAddr
        && value.serverPortUp == b.value.serverPortUp
        && value.serverPortDown == b.value.serverPortDown
        && value.keepaliveInterval == b.value.keepaliveInterval
        && value.statInterval == b.value.statInterval
        && (memcmp(&value.pushTimeoutMs, &b.value.pushTimeoutMs, sizeof(timeval)) == 0)
        && value.forwardCRCValid == b.value.forwardCRCValid
        && value.forwardCRCError == b.value.forwardCRCError
        && value.forwardCRCDisabled == b.value.forwardCRCDisabled
        && gpsTtyPath == b.gpsTtyPath
        && (fabs(value.refGeoCoordinates.lat - b.value.refGeoCoordinates.lat) < 0.00001)
        && (fabs(value.refGeoCoordinates.lon - b.value.refGeoCoordinates.lon) < 0.00001)
        && (abs(value.refGeoCoordinates.alt - b.value.refGeoCoordinates.alt)  == 0)
        && value.fakeGPS == b.value.fakeGPS
        && value.beaconPeriod == b.value.beaconPeriod
        && value.beaconFreqHz == b.value.beaconFreqHz
        && value.beaconFreqNb == b.value.beaconFreqNb
        && value.beaconFreqStep == b.value.beaconFreqStep
        && value.beaconDataRate == b.value.beaconDataRate
        && value.beaconBandwidthHz == b.value.beaconBandwidthHz
        && value.beaconPower == b.value.beaconPower
        && value.beaconInfoDesc == b.value.beaconInfoDesc
        && value.autoQuitThreshold == b.value.autoQuitThreshold;
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
    reset();
}

void GatewayDebugConfig::reset()
{
    memset(&value, 0, sizeof(value));
}

int GatewayDebugConfig::parse(rapidjson::Value &jsonValue)
{
    reset();
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

void GatewayDebugConfig::toCpp(
    std::ostream &retVal,
    const std::string &name
) const
{
    retVal << std::endl << "// Debug nb_ref_payload, count: " << (int) value.nb_ref_payload << std::endl << std::endl;
    retVal << name << ".nb_ref_payload = " << (int) value.nb_ref_payload << ";" << std::endl;
    // identifiers
    for (int i = 0; i < value.nb_ref_payload; i++) {
        retVal << name << ".ref_payload[" << i << "].id = 0x" << std::hex << value.ref_payload[i].id << std::dec << ";" << std::endl;
    }
    // log file name
    ADD_PCHAR(name, value, log_file_name)
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
        jrp.SetObject();
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(8) << value.ref_payload[i].id << std::dec;
        std::string s = ss.str();
        rapidjson::Value jrpId;
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

bool GatewayDebugConfig::operator==(const GatewayDebugConfig &b) const
{
    return memcmp(&value, &b.value, sizeof(struct lgw_conf_debug_s)) == 0;
}

GatewayConfigFileJson::GatewayConfigFileJson()
{

}

GatewayConfigFileJson::~GatewayConfigFileJson()
{

}

void GatewayConfigFileJson::reset()
{
    sx130xConf.reset();
    gatewayConf.reset();
    debugConf.reset();
}

int GatewayConfigFileJson::parse(
    rapidjson::Value &jsonValue
)
{
    int r = 0;
    if (jsonValue.HasMember("SX130x_conf")) {
        rapidjson::Value &jSx130x = jsonValue["SX130x_conf"];
        r = sx130xConf.parse(jSx130x);
        if (r)
            return r;
    }
    if (jsonValue.HasMember("gateway_conf")) {
        rapidjson::Value &jGateway = jsonValue["gateway_conf"];
        r = gatewayConf.parse(jGateway);
        if (r)
            return r;
    }
    if (jsonValue.HasMember("debug_conf")) {
        rapidjson::Value &jDebug = jsonValue["debug_conf"];
        r = debugConf.parse(jDebug);
    }
    return r;
}

void GatewayConfigFileJson::toCpp(
        std::ostream &retVal,
        const std::string &name
) const
{
    sx130xConf.sx1261Config.toCpp(retVal, name + ".sx1261");
    sx130xConf.toCpp(retVal, name + ".sx130x");
    gatewayConf.toCpp(retVal, name + ".gateway");
    retVal << name << ".serverAddr = \"" << gatewayConf.serverAddr << "\";" << std::endl;
    retVal << name << ".gpsTtyPath = \"" << gatewayConf.gpsTtyPath << "\";" << std::endl;
    std::string niceName(name);
    std::replace(niceName.begin(), niceName.end(), '_', ' ');
    retVal << name << ".name = \"" << niceName << "\";" << std::endl;
    debugConf.toCpp(retVal, name + ".debug");
}

void GatewayConfigFileJson::toJSON(
    rapidjson::Value &jsonValue,
    rapidjson::Document::AllocatorType& allocator
) const {
    jsonValue.SetObject();
    rapidjson::Value jSx130x;
    sx130xConf.toJSON(jSx130x, allocator);
    jsonValue.AddMember("SX130x_conf", jSx130x, allocator);
    rapidjson::Value jGateway;
    gatewayConf.toJSON(jGateway, allocator);
    jsonValue.AddMember("gateway_conf", jGateway, allocator);
    rapidjson::Value jDebug;
    debugConf.toJSON(jDebug, allocator);
    jsonValue.AddMember("debug_conf", jDebug, allocator);
}

bool GatewayConfigFileJson::operator==(
    const GatewayConfigFileJson &b
) const
{
    return (sx130xConf == b.sx130xConf)
        && (gatewayConf == b.gatewayConf)
        && (debugConf == b.debugConf);
}

std::string GatewayConfigFileJson::toString() const {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document doc;
    toJSON(doc, doc.GetAllocator());
    doc.Accept(writer);
    return std::string(buffer.GetString());
}

sx1261_config_t *GatewayConfigFileJson::sx1261()
{
    return &sx130xConf.sx1261Config.value;
}

sx130x_config_t *GatewayConfigFileJson::sx130x()
{
    return &sx130xConf.value;
}

gateway_t *GatewayConfigFileJson::gateway()
{
    return &gatewayConf.value;
}

struct lgw_conf_debug_s *GatewayConfigFileJson::debug()
{
    return &debugConf.value;
}

std::string *GatewayConfigFileJson::serverAddress()
{
    return &gatewayConf.serverAddr;
}

std::string *GatewayConfigFileJson::gpsTTYPath()
{
    return &gatewayConf.gpsTtyPath;
}
