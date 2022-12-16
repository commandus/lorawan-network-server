#include <string>
#include <cstring>
#include "gateway-settings.h"

class MemGatewaySettingsStorage {
public:
    sx1261_config_t sx1261;
    sx130x_config_t sx130x;
    gateway_t gateway;
    struct lgw_conf_debug_s debug;
    std::string serverAddr;
    std::string gpsTtyPath;
    std::string name;
};

typedef void (*setupMemGatewaySettingsStorageFunc)(MemGatewaySettingsStorage &value);

typedef struct {
    std::string name;
    setupMemGatewaySettingsStorageFunc setup;
} setupMemGatewaySettingsStorage;

class MemGatewaySettings : public GatewaySettings {
public:
    MemGatewaySettingsStorage storage;

    sx1261_config_t *sx1261() override { return &storage.sx1261; };
    sx130x_config_t *sx130x() override { return &storage.sx130x; };
    gateway_t *gateway() override { return &storage.gateway; };
    struct lgw_conf_debug_s *debug() override { return &storage.debug; };
    std::string *serverAddress() override { return &storage.serverAddr; };
    std::string *gpsTTYPath() override { return &storage.gpsTtyPath; };
};



void setup_as_915_921(MemGatewaySettingsStorage &as_915_921) {

// SX1261 

strcpy(as_915_921.sx1261.sx1261.spi_path, "");
as_915_921.sx1261.sx1261.rssi_offset = 0;
as_915_921.sx1261.spectralScan.enable = false;
as_915_921.sx1261.spectralScan.freq_hz_start = 0;
as_915_921.sx1261.spectralScan.nb_chan = 0;
as_915_921.sx1261.spectralScan.nb_scan = (int) 0;
as_915_921.sx1261.spectralScan.pace_s = 0;
as_915_921.sx1261.lbt.enable = false;
as_915_921.sx1261.lbt.nb_channel = 0;

// SX130x 

as_915_921.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(as_915_921.sx130x.boardConf.com_path, "/dev/ttyACM0");
as_915_921.sx130x.boardConf.lorawan_public = true;
as_915_921.sx130x.boardConf.clksrc = 0;
as_915_921.sx130x.antennaGain = 0;
as_915_921.sx130x.boardConf.full_duplex = false;
as_915_921.sx130x.tsConf.enable = false;
as_915_921.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

as_915_921.sx130x.rfConfs[0].enable = true;
as_915_921.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
as_915_921.sx130x.rfConfs[0].freq_hz = 917000000;
as_915_921.sx130x.rfConfs[0].rssi_offset = -215.4;
as_915_921.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
as_915_921.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
as_915_921.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
as_915_921.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
as_915_921.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
as_915_921.sx130x.rfConfs[0].tx_enable = true;
as_915_921.sx130x.rfConfs[0].single_input_mode = false;
as_915_921.sx130x.tx_freq_min[0] = 915000000;
as_915_921.sx130x.tx_freq_max[0] = 921000000;
as_915_921.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

as_915_921.sx130x.txLut[0].lut[0].rf_power = 12;
as_915_921.sx130x.txLut[0].lut[0].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[0].pwr_idx = 6;
as_915_921.sx130x.txLut[0].lut[0].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[0].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[0].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[1].rf_power = 13;
as_915_921.sx130x.txLut[0].lut[1].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[1].pwr_idx = 7;
as_915_921.sx130x.txLut[0].lut[1].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[1].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[1].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[2].rf_power = 14;
as_915_921.sx130x.txLut[0].lut[2].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[2].pwr_idx = 8;
as_915_921.sx130x.txLut[0].lut[2].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[2].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[2].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[3].rf_power = 15;
as_915_921.sx130x.txLut[0].lut[3].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[3].pwr_idx = 9;
as_915_921.sx130x.txLut[0].lut[3].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[3].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[3].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[4].rf_power = 16;
as_915_921.sx130x.txLut[0].lut[4].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[4].pwr_idx = 10;
as_915_921.sx130x.txLut[0].lut[4].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[4].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[4].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[5].rf_power = 17;
as_915_921.sx130x.txLut[0].lut[5].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[5].pwr_idx = 11;
as_915_921.sx130x.txLut[0].lut[5].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[5].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[5].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[6].rf_power = 18;
as_915_921.sx130x.txLut[0].lut[6].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[6].pwr_idx = 12;
as_915_921.sx130x.txLut[0].lut[6].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[6].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[6].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[7].rf_power = 19;
as_915_921.sx130x.txLut[0].lut[7].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[7].pwr_idx = 13;
as_915_921.sx130x.txLut[0].lut[7].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[7].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[7].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[8].rf_power = 20;
as_915_921.sx130x.txLut[0].lut[8].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[8].pwr_idx = 14;
as_915_921.sx130x.txLut[0].lut[8].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[8].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[8].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[9].rf_power = 21;
as_915_921.sx130x.txLut[0].lut[9].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[9].pwr_idx = 15;
as_915_921.sx130x.txLut[0].lut[9].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[9].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[9].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[10].rf_power = 22;
as_915_921.sx130x.txLut[0].lut[10].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[10].pwr_idx = 16;
as_915_921.sx130x.txLut[0].lut[10].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[10].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[10].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[11].rf_power = 23;
as_915_921.sx130x.txLut[0].lut[11].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[11].pwr_idx = 17;
as_915_921.sx130x.txLut[0].lut[11].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[11].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[11].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[12].rf_power = 24;
as_915_921.sx130x.txLut[0].lut[12].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[12].pwr_idx = 18;
as_915_921.sx130x.txLut[0].lut[12].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[12].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[12].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[13].rf_power = 25;
as_915_921.sx130x.txLut[0].lut[13].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[13].pwr_idx = 19;
as_915_921.sx130x.txLut[0].lut[13].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[13].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[13].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[14].rf_power = 26;
as_915_921.sx130x.txLut[0].lut[14].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[14].pwr_idx = 21;
as_915_921.sx130x.txLut[0].lut[14].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[14].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[14].mix_gain = 5;
as_915_921.sx130x.txLut[0].lut[15].rf_power = 27;
as_915_921.sx130x.txLut[0].lut[15].pa_gain = 1;
as_915_921.sx130x.txLut[0].lut[15].pwr_idx = 22;
as_915_921.sx130x.txLut[0].lut[15].dig_gain = 0;
as_915_921.sx130x.txLut[0].lut[15].dac_gain = 0;
as_915_921.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

as_915_921.sx130x.rfConfs[1].enable = true;
as_915_921.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
as_915_921.sx130x.rfConfs[1].freq_hz = 917800000;
as_915_921.sx130x.rfConfs[1].rssi_offset = -215.4;
as_915_921.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
as_915_921.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
as_915_921.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
as_915_921.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
as_915_921.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
as_915_921.sx130x.rfConfs[1].tx_enable = false;
as_915_921.sx130x.rfConfs[1].single_input_mode = false;
as_915_921.sx130x.tx_freq_min[1] = 0;
as_915_921.sx130x.tx_freq_max[1] = 0;
as_915_921.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

as_915_921.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
as_915_921.sx130x.ifConfs[0].enable = true;
as_915_921.sx130x.ifConfs[0].rf_chain = 0;
as_915_921.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
as_915_921.sx130x.ifConfs[1].enable = true;
as_915_921.sx130x.ifConfs[1].rf_chain = 0;
as_915_921.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
as_915_921.sx130x.ifConfs[2].enable = true;
as_915_921.sx130x.ifConfs[2].rf_chain = 0;
as_915_921.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
as_915_921.sx130x.ifConfs[3].enable = true;
as_915_921.sx130x.ifConfs[3].rf_chain = 0;
as_915_921.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
as_915_921.sx130x.ifConfs[4].enable = true;
as_915_921.sx130x.ifConfs[4].rf_chain = 1;
as_915_921.sx130x.ifConfs[4].freq_hz = -400000;
// chan_multiSF_5
as_915_921.sx130x.ifConfs[5].enable = true;
as_915_921.sx130x.ifConfs[5].rf_chain = 1;
as_915_921.sx130x.ifConfs[5].freq_hz = -200000;
// chan_multiSF_6
as_915_921.sx130x.ifConfs[6].enable = true;
as_915_921.sx130x.ifConfs[6].rf_chain = 1;
as_915_921.sx130x.ifConfs[6].freq_hz = 0;
// chan_multiSF_7
as_915_921.sx130x.ifConfs[7].enable = true;
as_915_921.sx130x.ifConfs[7].rf_chain = 1;
as_915_921.sx130x.ifConfs[7].freq_hz = 200000;
// Lora std 
as_915_921.sx130x.ifStdConf.enable = true;
as_915_921.sx130x.ifStdConf.rf_chain = 1;
as_915_921.sx130x.ifStdConf.freq_hz = -100000;
as_915_921.sx130x.ifStdConf.bandwidth = 5;
as_915_921.sx130x.ifStdConf.datarate = 7;
as_915_921.sx130x.ifStdConf.implicit_hdr = false;
as_915_921.sx130x.ifStdConf.implicit_payload_length = 17;
as_915_921.sx130x.ifStdConf.implicit_crc_en = false;
as_915_921.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
as_915_921.sx130x.ifFSKConf.enable = true;
as_915_921.sx130x.ifFSKConf.rf_chain = 1;
as_915_921.sx130x.ifFSKConf.freq_hz = 200000;
as_915_921.sx130x.ifFSKConf.bandwidth = 4;
as_915_921.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

as_915_921.gateway.gatewayId = 0xaa555a0000000000;
as_915_921.gateway.serverPortUp = 1700;
as_915_921.gateway.serverPortDown = 1700;
as_915_921.gateway.keepaliveInterval = 10;
as_915_921.gateway.statInterval = 30;
as_915_921.gateway.pushTimeoutMs.tv_sec = 0;
as_915_921.gateway.pushTimeoutMs.tv_usec = 50000;
as_915_921.gateway.forwardCRCValid = true;
as_915_921.gateway.forwardCRCError = false;
as_915_921.gateway.forwardCRCDisabled = false;
as_915_921.gateway.refGeoCoordinates.lat = 0;
as_915_921.gateway.refGeoCoordinates.lon = 0;
as_915_921.gateway.refGeoCoordinates.alt = 0;
as_915_921.gateway.fakeGPS = false;
as_915_921.gateway.beaconPeriod = 0;
as_915_921.gateway.beaconFreqHz = 916800000;
as_915_921.gateway.beaconFreqNb = 1;
as_915_921.gateway.beaconFreqStep = 0;
as_915_921.gateway.beaconDataRate = 9;
as_915_921.gateway.beaconBandwidthHz = 125000;
as_915_921.gateway.beaconInfoDesc = 0;
as_915_921.gateway.autoQuitThreshold = 0;
as_915_921.serverAddr = "nam1.cloud.thethings.network";
as_915_921.gpsTtyPath = "/dev/ttyAMA0";
as_915_921.name = "as 915 921";

// Debug nb_ref_payload, count: 2

as_915_921.debug.nb_ref_payload = 2;
as_915_921.debug.ref_payload[0].id = 0xcafe1234;
as_915_921.debug.ref_payload[1].id = 0xcafe2345;
strcpy(as_915_921.debug.log_file_name, "loragw_hal.log");

};   // as_915_921

void setup_as_915_928(MemGatewaySettingsStorage &as_915_928) {

// SX1261 

strcpy(as_915_928.sx1261.sx1261.spi_path, "");
as_915_928.sx1261.sx1261.rssi_offset = 0;
as_915_928.sx1261.spectralScan.enable = false;
as_915_928.sx1261.spectralScan.freq_hz_start = 0;
as_915_928.sx1261.spectralScan.nb_chan = 0;
as_915_928.sx1261.spectralScan.nb_scan = (int) 0;
as_915_928.sx1261.spectralScan.pace_s = 0;
as_915_928.sx1261.lbt.enable = false;
as_915_928.sx1261.lbt.nb_channel = 0;

// SX130x 

as_915_928.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(as_915_928.sx130x.boardConf.com_path, "/dev/ttyACM0");
as_915_928.sx130x.boardConf.lorawan_public = true;
as_915_928.sx130x.boardConf.clksrc = 0;
as_915_928.sx130x.antennaGain = 0;
as_915_928.sx130x.boardConf.full_duplex = false;
as_915_928.sx130x.tsConf.enable = false;
as_915_928.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

as_915_928.sx130x.rfConfs[0].enable = true;
as_915_928.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
as_915_928.sx130x.rfConfs[0].freq_hz = 923600000;
as_915_928.sx130x.rfConfs[0].rssi_offset = -215.4;
as_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
as_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
as_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
as_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
as_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
as_915_928.sx130x.rfConfs[0].tx_enable = true;
as_915_928.sx130x.rfConfs[0].single_input_mode = false;
as_915_928.sx130x.tx_freq_min[0] = 915000000;
as_915_928.sx130x.tx_freq_max[0] = 928000000;
as_915_928.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

as_915_928.sx130x.txLut[0].lut[0].rf_power = 12;
as_915_928.sx130x.txLut[0].lut[0].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[0].pwr_idx = 6;
as_915_928.sx130x.txLut[0].lut[0].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[0].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[0].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[1].rf_power = 13;
as_915_928.sx130x.txLut[0].lut[1].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[1].pwr_idx = 7;
as_915_928.sx130x.txLut[0].lut[1].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[1].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[1].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[2].rf_power = 14;
as_915_928.sx130x.txLut[0].lut[2].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[2].pwr_idx = 8;
as_915_928.sx130x.txLut[0].lut[2].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[2].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[2].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[3].rf_power = 15;
as_915_928.sx130x.txLut[0].lut[3].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[3].pwr_idx = 9;
as_915_928.sx130x.txLut[0].lut[3].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[3].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[3].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[4].rf_power = 16;
as_915_928.sx130x.txLut[0].lut[4].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[4].pwr_idx = 10;
as_915_928.sx130x.txLut[0].lut[4].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[4].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[4].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[5].rf_power = 17;
as_915_928.sx130x.txLut[0].lut[5].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[5].pwr_idx = 11;
as_915_928.sx130x.txLut[0].lut[5].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[5].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[5].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[6].rf_power = 18;
as_915_928.sx130x.txLut[0].lut[6].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[6].pwr_idx = 12;
as_915_928.sx130x.txLut[0].lut[6].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[6].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[6].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[7].rf_power = 19;
as_915_928.sx130x.txLut[0].lut[7].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[7].pwr_idx = 13;
as_915_928.sx130x.txLut[0].lut[7].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[7].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[7].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[8].rf_power = 20;
as_915_928.sx130x.txLut[0].lut[8].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[8].pwr_idx = 14;
as_915_928.sx130x.txLut[0].lut[8].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[8].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[8].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[9].rf_power = 21;
as_915_928.sx130x.txLut[0].lut[9].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[9].pwr_idx = 15;
as_915_928.sx130x.txLut[0].lut[9].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[9].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[9].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[10].rf_power = 22;
as_915_928.sx130x.txLut[0].lut[10].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[10].pwr_idx = 16;
as_915_928.sx130x.txLut[0].lut[10].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[10].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[10].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[11].rf_power = 23;
as_915_928.sx130x.txLut[0].lut[11].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[11].pwr_idx = 17;
as_915_928.sx130x.txLut[0].lut[11].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[11].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[11].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[12].rf_power = 24;
as_915_928.sx130x.txLut[0].lut[12].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[12].pwr_idx = 18;
as_915_928.sx130x.txLut[0].lut[12].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[12].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[12].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[13].rf_power = 25;
as_915_928.sx130x.txLut[0].lut[13].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[13].pwr_idx = 19;
as_915_928.sx130x.txLut[0].lut[13].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[13].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[13].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[14].rf_power = 26;
as_915_928.sx130x.txLut[0].lut[14].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[14].pwr_idx = 21;
as_915_928.sx130x.txLut[0].lut[14].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[14].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[14].mix_gain = 5;
as_915_928.sx130x.txLut[0].lut[15].rf_power = 27;
as_915_928.sx130x.txLut[0].lut[15].pa_gain = 1;
as_915_928.sx130x.txLut[0].lut[15].pwr_idx = 22;
as_915_928.sx130x.txLut[0].lut[15].dig_gain = 0;
as_915_928.sx130x.txLut[0].lut[15].dac_gain = 0;
as_915_928.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

as_915_928.sx130x.rfConfs[1].enable = true;
as_915_928.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
as_915_928.sx130x.rfConfs[1].freq_hz = 924400000;
as_915_928.sx130x.rfConfs[1].rssi_offset = -215.4;
as_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
as_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
as_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
as_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
as_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
as_915_928.sx130x.rfConfs[1].tx_enable = false;
as_915_928.sx130x.rfConfs[1].single_input_mode = false;
as_915_928.sx130x.tx_freq_min[1] = 0;
as_915_928.sx130x.tx_freq_max[1] = 0;
as_915_928.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

as_915_928.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
as_915_928.sx130x.ifConfs[0].enable = true;
as_915_928.sx130x.ifConfs[0].rf_chain = 0;
as_915_928.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
as_915_928.sx130x.ifConfs[1].enable = true;
as_915_928.sx130x.ifConfs[1].rf_chain = 0;
as_915_928.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
as_915_928.sx130x.ifConfs[2].enable = true;
as_915_928.sx130x.ifConfs[2].rf_chain = 0;
as_915_928.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
as_915_928.sx130x.ifConfs[3].enable = true;
as_915_928.sx130x.ifConfs[3].rf_chain = 0;
as_915_928.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
as_915_928.sx130x.ifConfs[4].enable = true;
as_915_928.sx130x.ifConfs[4].rf_chain = 1;
as_915_928.sx130x.ifConfs[4].freq_hz = -400000;
// chan_multiSF_5
as_915_928.sx130x.ifConfs[5].enable = true;
as_915_928.sx130x.ifConfs[5].rf_chain = 1;
as_915_928.sx130x.ifConfs[5].freq_hz = -200000;
// chan_multiSF_6
as_915_928.sx130x.ifConfs[6].enable = true;
as_915_928.sx130x.ifConfs[6].rf_chain = 1;
as_915_928.sx130x.ifConfs[6].freq_hz = 0;
// chan_multiSF_7
as_915_928.sx130x.ifConfs[7].enable = true;
as_915_928.sx130x.ifConfs[7].rf_chain = 1;
as_915_928.sx130x.ifConfs[7].freq_hz = 200000;
// Lora std 
as_915_928.sx130x.ifStdConf.enable = true;
as_915_928.sx130x.ifStdConf.rf_chain = 1;
as_915_928.sx130x.ifStdConf.freq_hz = -100000;
as_915_928.sx130x.ifStdConf.bandwidth = 5;
as_915_928.sx130x.ifStdConf.datarate = 7;
as_915_928.sx130x.ifStdConf.implicit_hdr = false;
as_915_928.sx130x.ifStdConf.implicit_payload_length = 17;
as_915_928.sx130x.ifStdConf.implicit_crc_en = false;
as_915_928.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
as_915_928.sx130x.ifFSKConf.enable = true;
as_915_928.sx130x.ifFSKConf.rf_chain = 1;
as_915_928.sx130x.ifFSKConf.freq_hz = 200000;
as_915_928.sx130x.ifFSKConf.bandwidth = 4;
as_915_928.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

as_915_928.gateway.gatewayId = 0xaa555a0000000000;
as_915_928.gateway.serverPortUp = 1700;
as_915_928.gateway.serverPortDown = 1700;
as_915_928.gateway.keepaliveInterval = 10;
as_915_928.gateway.statInterval = 30;
as_915_928.gateway.pushTimeoutMs.tv_sec = 0;
as_915_928.gateway.pushTimeoutMs.tv_usec = 50000;
as_915_928.gateway.forwardCRCValid = true;
as_915_928.gateway.forwardCRCError = false;
as_915_928.gateway.forwardCRCDisabled = false;
as_915_928.gateway.refGeoCoordinates.lat = 0;
as_915_928.gateway.refGeoCoordinates.lon = 0;
as_915_928.gateway.refGeoCoordinates.alt = 0;
as_915_928.gateway.fakeGPS = false;
as_915_928.gateway.beaconPeriod = 0;
as_915_928.gateway.beaconFreqHz = 923400000;
as_915_928.gateway.beaconFreqNb = 1;
as_915_928.gateway.beaconFreqStep = 0;
as_915_928.gateway.beaconDataRate = 9;
as_915_928.gateway.beaconBandwidthHz = 125000;
as_915_928.gateway.beaconInfoDesc = 0;
as_915_928.gateway.autoQuitThreshold = 0;
as_915_928.serverAddr = "nam1.cloud.thethings.network";
as_915_928.gpsTtyPath = "/dev/ttyAMA0";
as_915_928.name = "as 915 928";

// Debug nb_ref_payload, count: 2

as_915_928.debug.nb_ref_payload = 2;
as_915_928.debug.ref_payload[0].id = 0xcafe1234;
as_915_928.debug.ref_payload[1].id = 0xcafe2345;
strcpy(as_915_928.debug.log_file_name, "loragw_hal.log");

};   // as_915_928

void setup_as_917_920(MemGatewaySettingsStorage &as_917_920) {

// SX1261 

strcpy(as_917_920.sx1261.sx1261.spi_path, "");
as_917_920.sx1261.sx1261.rssi_offset = 0;
as_917_920.sx1261.spectralScan.enable = false;
as_917_920.sx1261.spectralScan.freq_hz_start = 0;
as_917_920.sx1261.spectralScan.nb_chan = 0;
as_917_920.sx1261.spectralScan.nb_scan = (int) 0;
as_917_920.sx1261.spectralScan.pace_s = 0;
as_917_920.sx1261.lbt.enable = false;
as_917_920.sx1261.lbt.nb_channel = 0;

// SX130x 

as_917_920.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(as_917_920.sx130x.boardConf.com_path, "/dev/ttyACM0");
as_917_920.sx130x.boardConf.lorawan_public = true;
as_917_920.sx130x.boardConf.clksrc = 0;
as_917_920.sx130x.antennaGain = 0;
as_917_920.sx130x.boardConf.full_duplex = false;
as_917_920.sx130x.tsConf.enable = false;
as_917_920.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

as_917_920.sx130x.rfConfs[0].enable = true;
as_917_920.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
as_917_920.sx130x.rfConfs[0].freq_hz = 917700000;
as_917_920.sx130x.rfConfs[0].rssi_offset = -215.4;
as_917_920.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
as_917_920.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
as_917_920.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
as_917_920.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
as_917_920.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
as_917_920.sx130x.rfConfs[0].tx_enable = true;
as_917_920.sx130x.rfConfs[0].single_input_mode = false;
as_917_920.sx130x.tx_freq_min[0] = 917000000;
as_917_920.sx130x.tx_freq_max[0] = 920000000;
as_917_920.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

as_917_920.sx130x.txLut[0].lut[0].rf_power = 12;
as_917_920.sx130x.txLut[0].lut[0].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[0].pwr_idx = 6;
as_917_920.sx130x.txLut[0].lut[0].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[0].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[0].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[1].rf_power = 13;
as_917_920.sx130x.txLut[0].lut[1].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[1].pwr_idx = 7;
as_917_920.sx130x.txLut[0].lut[1].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[1].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[1].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[2].rf_power = 14;
as_917_920.sx130x.txLut[0].lut[2].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[2].pwr_idx = 8;
as_917_920.sx130x.txLut[0].lut[2].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[2].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[2].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[3].rf_power = 15;
as_917_920.sx130x.txLut[0].lut[3].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[3].pwr_idx = 9;
as_917_920.sx130x.txLut[0].lut[3].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[3].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[3].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[4].rf_power = 16;
as_917_920.sx130x.txLut[0].lut[4].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[4].pwr_idx = 10;
as_917_920.sx130x.txLut[0].lut[4].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[4].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[4].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[5].rf_power = 17;
as_917_920.sx130x.txLut[0].lut[5].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[5].pwr_idx = 11;
as_917_920.sx130x.txLut[0].lut[5].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[5].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[5].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[6].rf_power = 18;
as_917_920.sx130x.txLut[0].lut[6].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[6].pwr_idx = 12;
as_917_920.sx130x.txLut[0].lut[6].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[6].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[6].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[7].rf_power = 19;
as_917_920.sx130x.txLut[0].lut[7].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[7].pwr_idx = 13;
as_917_920.sx130x.txLut[0].lut[7].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[7].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[7].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[8].rf_power = 20;
as_917_920.sx130x.txLut[0].lut[8].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[8].pwr_idx = 14;
as_917_920.sx130x.txLut[0].lut[8].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[8].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[8].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[9].rf_power = 21;
as_917_920.sx130x.txLut[0].lut[9].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[9].pwr_idx = 15;
as_917_920.sx130x.txLut[0].lut[9].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[9].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[9].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[10].rf_power = 22;
as_917_920.sx130x.txLut[0].lut[10].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[10].pwr_idx = 16;
as_917_920.sx130x.txLut[0].lut[10].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[10].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[10].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[11].rf_power = 23;
as_917_920.sx130x.txLut[0].lut[11].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[11].pwr_idx = 17;
as_917_920.sx130x.txLut[0].lut[11].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[11].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[11].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[12].rf_power = 24;
as_917_920.sx130x.txLut[0].lut[12].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[12].pwr_idx = 18;
as_917_920.sx130x.txLut[0].lut[12].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[12].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[12].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[13].rf_power = 25;
as_917_920.sx130x.txLut[0].lut[13].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[13].pwr_idx = 19;
as_917_920.sx130x.txLut[0].lut[13].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[13].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[13].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[14].rf_power = 26;
as_917_920.sx130x.txLut[0].lut[14].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[14].pwr_idx = 21;
as_917_920.sx130x.txLut[0].lut[14].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[14].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[14].mix_gain = 5;
as_917_920.sx130x.txLut[0].lut[15].rf_power = 27;
as_917_920.sx130x.txLut[0].lut[15].pa_gain = 1;
as_917_920.sx130x.txLut[0].lut[15].pwr_idx = 22;
as_917_920.sx130x.txLut[0].lut[15].dig_gain = 0;
as_917_920.sx130x.txLut[0].lut[15].dac_gain = 0;
as_917_920.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

as_917_920.sx130x.rfConfs[1].enable = true;
as_917_920.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
as_917_920.sx130x.rfConfs[1].freq_hz = 918500000;
as_917_920.sx130x.rfConfs[1].rssi_offset = -215.4;
as_917_920.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
as_917_920.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
as_917_920.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
as_917_920.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
as_917_920.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
as_917_920.sx130x.rfConfs[1].tx_enable = false;
as_917_920.sx130x.rfConfs[1].single_input_mode = false;
as_917_920.sx130x.tx_freq_min[1] = 0;
as_917_920.sx130x.tx_freq_max[1] = 0;
as_917_920.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

as_917_920.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
as_917_920.sx130x.ifConfs[0].enable = true;
as_917_920.sx130x.ifConfs[0].rf_chain = 0;
as_917_920.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
as_917_920.sx130x.ifConfs[1].enable = true;
as_917_920.sx130x.ifConfs[1].rf_chain = 0;
as_917_920.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
as_917_920.sx130x.ifConfs[2].enable = true;
as_917_920.sx130x.ifConfs[2].rf_chain = 0;
as_917_920.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
as_917_920.sx130x.ifConfs[3].enable = true;
as_917_920.sx130x.ifConfs[3].rf_chain = 0;
as_917_920.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
as_917_920.sx130x.ifConfs[4].enable = true;
as_917_920.sx130x.ifConfs[4].rf_chain = 1;
as_917_920.sx130x.ifConfs[4].freq_hz = -400000;
// chan_multiSF_5
as_917_920.sx130x.ifConfs[5].enable = true;
as_917_920.sx130x.ifConfs[5].rf_chain = 1;
as_917_920.sx130x.ifConfs[5].freq_hz = -200000;
// chan_multiSF_6
as_917_920.sx130x.ifConfs[6].enable = true;
as_917_920.sx130x.ifConfs[6].rf_chain = 1;
as_917_920.sx130x.ifConfs[6].freq_hz = 0;
// chan_multiSF_7
as_917_920.sx130x.ifConfs[7].enable = true;
as_917_920.sx130x.ifConfs[7].rf_chain = 1;
as_917_920.sx130x.ifConfs[7].freq_hz = 200000;
// Lora std 
as_917_920.sx130x.ifStdConf.enable = true;
as_917_920.sx130x.ifStdConf.rf_chain = 1;
as_917_920.sx130x.ifStdConf.freq_hz = -100000;
as_917_920.sx130x.ifStdConf.bandwidth = 5;
as_917_920.sx130x.ifStdConf.datarate = 7;
as_917_920.sx130x.ifStdConf.implicit_hdr = false;
as_917_920.sx130x.ifStdConf.implicit_payload_length = 17;
as_917_920.sx130x.ifStdConf.implicit_crc_en = false;
as_917_920.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
as_917_920.sx130x.ifFSKConf.enable = true;
as_917_920.sx130x.ifFSKConf.rf_chain = 1;
as_917_920.sx130x.ifFSKConf.freq_hz = 200000;
as_917_920.sx130x.ifFSKConf.bandwidth = 4;
as_917_920.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

as_917_920.gateway.gatewayId = 0xaa555a0000000000;
as_917_920.gateway.serverPortUp = 1700;
as_917_920.gateway.serverPortDown = 1700;
as_917_920.gateway.keepaliveInterval = 10;
as_917_920.gateway.statInterval = 30;
as_917_920.gateway.pushTimeoutMs.tv_sec = 0;
as_917_920.gateway.pushTimeoutMs.tv_usec = 50000;
as_917_920.gateway.forwardCRCValid = true;
as_917_920.gateway.forwardCRCError = false;
as_917_920.gateway.forwardCRCDisabled = false;
as_917_920.gateway.refGeoCoordinates.lat = 0;
as_917_920.gateway.refGeoCoordinates.lon = 0;
as_917_920.gateway.refGeoCoordinates.alt = 0;
as_917_920.gateway.fakeGPS = false;
as_917_920.gateway.beaconPeriod = 0;
as_917_920.gateway.beaconFreqHz = 917500000;
as_917_920.gateway.beaconFreqNb = 1;
as_917_920.gateway.beaconFreqStep = 0;
as_917_920.gateway.beaconDataRate = 9;
as_917_920.gateway.beaconBandwidthHz = 125000;
as_917_920.gateway.beaconInfoDesc = 0;
as_917_920.gateway.autoQuitThreshold = 0;
as_917_920.serverAddr = "nam1.cloud.thethings.network";
as_917_920.gpsTtyPath = "/dev/ttyAMA0";
as_917_920.name = "as 917 920";

// Debug nb_ref_payload, count: 2

as_917_920.debug.nb_ref_payload = 2;
as_917_920.debug.ref_payload[0].id = 0xcafe1234;
as_917_920.debug.ref_payload[1].id = 0xcafe2345;
strcpy(as_917_920.debug.log_file_name, "loragw_hal.log");

};   // as_917_920

void setup_as_920_923(MemGatewaySettingsStorage &as_920_923) {

// SX1261 

strcpy(as_920_923.sx1261.sx1261.spi_path, "");
as_920_923.sx1261.sx1261.rssi_offset = 0;
as_920_923.sx1261.spectralScan.enable = false;
as_920_923.sx1261.spectralScan.freq_hz_start = 0;
as_920_923.sx1261.spectralScan.nb_chan = 0;
as_920_923.sx1261.spectralScan.nb_scan = (int) 0;
as_920_923.sx1261.spectralScan.pace_s = 0;
as_920_923.sx1261.lbt.enable = false;
as_920_923.sx1261.lbt.nb_channel = 0;

// SX130x 

as_920_923.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(as_920_923.sx130x.boardConf.com_path, "/dev/ttyACM0");
as_920_923.sx130x.boardConf.lorawan_public = true;
as_920_923.sx130x.boardConf.clksrc = 0;
as_920_923.sx130x.antennaGain = 0;
as_920_923.sx130x.boardConf.full_duplex = false;
as_920_923.sx130x.tsConf.enable = false;
as_920_923.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

as_920_923.sx130x.rfConfs[0].enable = true;
as_920_923.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
as_920_923.sx130x.rfConfs[0].freq_hz = 921800000;
as_920_923.sx130x.rfConfs[0].rssi_offset = -215.4;
as_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
as_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
as_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
as_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
as_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
as_920_923.sx130x.rfConfs[0].tx_enable = true;
as_920_923.sx130x.rfConfs[0].single_input_mode = false;
as_920_923.sx130x.tx_freq_min[0] = 920000000;
as_920_923.sx130x.tx_freq_max[0] = 923000000;
as_920_923.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

as_920_923.sx130x.txLut[0].lut[0].rf_power = 12;
as_920_923.sx130x.txLut[0].lut[0].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[0].pwr_idx = 1;
as_920_923.sx130x.txLut[0].lut[0].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[0].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[0].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[1].rf_power = 13;
as_920_923.sx130x.txLut[0].lut[1].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[1].pwr_idx = 2;
as_920_923.sx130x.txLut[0].lut[1].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[1].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[1].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[2].rf_power = 14;
as_920_923.sx130x.txLut[0].lut[2].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[2].pwr_idx = 3;
as_920_923.sx130x.txLut[0].lut[2].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[2].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[2].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[3].rf_power = 15;
as_920_923.sx130x.txLut[0].lut[3].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[3].pwr_idx = 4;
as_920_923.sx130x.txLut[0].lut[3].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[3].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[3].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[4].rf_power = 16;
as_920_923.sx130x.txLut[0].lut[4].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[4].pwr_idx = 5;
as_920_923.sx130x.txLut[0].lut[4].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[4].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[4].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[5].rf_power = 17;
as_920_923.sx130x.txLut[0].lut[5].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[5].pwr_idx = 6;
as_920_923.sx130x.txLut[0].lut[5].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[5].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[5].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[6].rf_power = 18;
as_920_923.sx130x.txLut[0].lut[6].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[6].pwr_idx = 8;
as_920_923.sx130x.txLut[0].lut[6].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[6].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[6].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[7].rf_power = 19;
as_920_923.sx130x.txLut[0].lut[7].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[7].pwr_idx = 13;
as_920_923.sx130x.txLut[0].lut[7].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[7].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[7].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[8].rf_power = 20;
as_920_923.sx130x.txLut[0].lut[8].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[8].pwr_idx = 10;
as_920_923.sx130x.txLut[0].lut[8].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[8].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[8].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[9].rf_power = 21;
as_920_923.sx130x.txLut[0].lut[9].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[9].pwr_idx = 15;
as_920_923.sx130x.txLut[0].lut[9].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[9].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[9].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[10].rf_power = 22;
as_920_923.sx130x.txLut[0].lut[10].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[10].pwr_idx = 12;
as_920_923.sx130x.txLut[0].lut[10].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[10].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[10].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[11].rf_power = 23;
as_920_923.sx130x.txLut[0].lut[11].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[11].pwr_idx = 13;
as_920_923.sx130x.txLut[0].lut[11].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[11].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[11].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[12].rf_power = 24;
as_920_923.sx130x.txLut[0].lut[12].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[12].pwr_idx = 14;
as_920_923.sx130x.txLut[0].lut[12].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[12].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[12].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[13].rf_power = 25;
as_920_923.sx130x.txLut[0].lut[13].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[13].pwr_idx = 16;
as_920_923.sx130x.txLut[0].lut[13].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[13].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[13].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[14].rf_power = 26;
as_920_923.sx130x.txLut[0].lut[14].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[14].pwr_idx = 17;
as_920_923.sx130x.txLut[0].lut[14].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[14].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[14].mix_gain = 5;
as_920_923.sx130x.txLut[0].lut[15].rf_power = 27;
as_920_923.sx130x.txLut[0].lut[15].pa_gain = 1;
as_920_923.sx130x.txLut[0].lut[15].pwr_idx = 19;
as_920_923.sx130x.txLut[0].lut[15].dig_gain = 0;
as_920_923.sx130x.txLut[0].lut[15].dac_gain = 0;
as_920_923.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

as_920_923.sx130x.rfConfs[1].enable = true;
as_920_923.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
as_920_923.sx130x.rfConfs[1].freq_hz = 922600000;
as_920_923.sx130x.rfConfs[1].rssi_offset = -215.4;
as_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
as_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
as_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
as_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
as_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
as_920_923.sx130x.rfConfs[1].tx_enable = false;
as_920_923.sx130x.rfConfs[1].single_input_mode = false;
as_920_923.sx130x.tx_freq_min[1] = 0;
as_920_923.sx130x.tx_freq_max[1] = 0;
as_920_923.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

as_920_923.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
as_920_923.sx130x.ifConfs[0].enable = true;
as_920_923.sx130x.ifConfs[0].rf_chain = 0;
as_920_923.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
as_920_923.sx130x.ifConfs[1].enable = true;
as_920_923.sx130x.ifConfs[1].rf_chain = 0;
as_920_923.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
as_920_923.sx130x.ifConfs[2].enable = true;
as_920_923.sx130x.ifConfs[2].rf_chain = 0;
as_920_923.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
as_920_923.sx130x.ifConfs[3].enable = true;
as_920_923.sx130x.ifConfs[3].rf_chain = 0;
as_920_923.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
as_920_923.sx130x.ifConfs[4].enable = true;
as_920_923.sx130x.ifConfs[4].rf_chain = 1;
as_920_923.sx130x.ifConfs[4].freq_hz = -400000;
// chan_multiSF_5
as_920_923.sx130x.ifConfs[5].enable = true;
as_920_923.sx130x.ifConfs[5].rf_chain = 1;
as_920_923.sx130x.ifConfs[5].freq_hz = -200000;
// chan_multiSF_6
as_920_923.sx130x.ifConfs[6].enable = true;
as_920_923.sx130x.ifConfs[6].rf_chain = 1;
as_920_923.sx130x.ifConfs[6].freq_hz = 0;
// chan_multiSF_7
as_920_923.sx130x.ifConfs[7].enable = true;
as_920_923.sx130x.ifConfs[7].rf_chain = 1;
as_920_923.sx130x.ifConfs[7].freq_hz = 200000;
// Lora std 
as_920_923.sx130x.ifStdConf.enable = true;
as_920_923.sx130x.ifStdConf.rf_chain = 1;
as_920_923.sx130x.ifStdConf.freq_hz = -100000;
as_920_923.sx130x.ifStdConf.bandwidth = 5;
as_920_923.sx130x.ifStdConf.datarate = 7;
as_920_923.sx130x.ifStdConf.implicit_hdr = false;
as_920_923.sx130x.ifStdConf.implicit_payload_length = 17;
as_920_923.sx130x.ifStdConf.implicit_crc_en = false;
as_920_923.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
as_920_923.sx130x.ifFSKConf.enable = true;
as_920_923.sx130x.ifFSKConf.rf_chain = 1;
as_920_923.sx130x.ifFSKConf.freq_hz = 200000;
as_920_923.sx130x.ifFSKConf.bandwidth = 4;
as_920_923.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

as_920_923.gateway.gatewayId = 0xaa555a0000000000;
as_920_923.gateway.serverPortUp = 1700;
as_920_923.gateway.serverPortDown = 1700;
as_920_923.gateway.keepaliveInterval = 10;
as_920_923.gateway.statInterval = 30;
as_920_923.gateway.pushTimeoutMs.tv_sec = 0;
as_920_923.gateway.pushTimeoutMs.tv_usec = 50000;
as_920_923.gateway.forwardCRCValid = true;
as_920_923.gateway.forwardCRCError = false;
as_920_923.gateway.forwardCRCDisabled = false;
as_920_923.gateway.refGeoCoordinates.lat = 0;
as_920_923.gateway.refGeoCoordinates.lon = 0;
as_920_923.gateway.refGeoCoordinates.alt = 0;
as_920_923.gateway.fakeGPS = false;
as_920_923.gateway.beaconPeriod = 0;
as_920_923.gateway.beaconFreqHz = 921600000;
as_920_923.gateway.beaconFreqNb = 1;
as_920_923.gateway.beaconFreqStep = 0;
as_920_923.gateway.beaconDataRate = 9;
as_920_923.gateway.beaconBandwidthHz = 125000;
as_920_923.gateway.beaconInfoDesc = 0;
as_920_923.gateway.autoQuitThreshold = 0;
as_920_923.serverAddr = "nam1.cloud.thethings.network";
as_920_923.gpsTtyPath = "/dev/ttyAMA0";
as_920_923.name = "as 920 923";

// Debug nb_ref_payload, count: 2

as_920_923.debug.nb_ref_payload = 2;
as_920_923.debug.ref_payload[0].id = 0xcafe1234;
as_920_923.debug.ref_payload[1].id = 0xcafe2345;
strcpy(as_920_923.debug.log_file_name, "loragw_hal.log");

};   // as_920_923

void setup_au_915_928(MemGatewaySettingsStorage &au_915_928) {

// SX1261 

strcpy(au_915_928.sx1261.sx1261.spi_path, "");
au_915_928.sx1261.sx1261.rssi_offset = 0;
au_915_928.sx1261.spectralScan.enable = false;
au_915_928.sx1261.spectralScan.freq_hz_start = 0;
au_915_928.sx1261.spectralScan.nb_chan = 0;
au_915_928.sx1261.spectralScan.nb_scan = (int) 0;
au_915_928.sx1261.spectralScan.pace_s = 0;
au_915_928.sx1261.lbt.enable = false;
au_915_928.sx1261.lbt.nb_channel = 0;

// SX130x 

au_915_928.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(au_915_928.sx130x.boardConf.com_path, "/dev/ttyACM0");
au_915_928.sx130x.boardConf.lorawan_public = true;
au_915_928.sx130x.boardConf.clksrc = 0;
au_915_928.sx130x.antennaGain = 0;
au_915_928.sx130x.boardConf.full_duplex = false;
au_915_928.sx130x.tsConf.enable = false;
au_915_928.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

au_915_928.sx130x.rfConfs[0].enable = true;
au_915_928.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
au_915_928.sx130x.rfConfs[0].freq_hz = 917200000;
au_915_928.sx130x.rfConfs[0].rssi_offset = -215.4;
au_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
au_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
au_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
au_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
au_915_928.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
au_915_928.sx130x.rfConfs[0].tx_enable = true;
au_915_928.sx130x.rfConfs[0].single_input_mode = false;
au_915_928.sx130x.tx_freq_min[0] = 915000000;
au_915_928.sx130x.tx_freq_max[0] = 928000000;
au_915_928.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

au_915_928.sx130x.txLut[0].lut[0].rf_power = 12;
au_915_928.sx130x.txLut[0].lut[0].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[0].pwr_idx = 6;
au_915_928.sx130x.txLut[0].lut[0].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[0].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[0].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[1].rf_power = 13;
au_915_928.sx130x.txLut[0].lut[1].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[1].pwr_idx = 7;
au_915_928.sx130x.txLut[0].lut[1].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[1].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[1].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[2].rf_power = 14;
au_915_928.sx130x.txLut[0].lut[2].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[2].pwr_idx = 8;
au_915_928.sx130x.txLut[0].lut[2].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[2].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[2].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[3].rf_power = 15;
au_915_928.sx130x.txLut[0].lut[3].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[3].pwr_idx = 9;
au_915_928.sx130x.txLut[0].lut[3].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[3].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[3].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[4].rf_power = 16;
au_915_928.sx130x.txLut[0].lut[4].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[4].pwr_idx = 10;
au_915_928.sx130x.txLut[0].lut[4].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[4].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[4].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[5].rf_power = 17;
au_915_928.sx130x.txLut[0].lut[5].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[5].pwr_idx = 11;
au_915_928.sx130x.txLut[0].lut[5].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[5].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[5].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[6].rf_power = 18;
au_915_928.sx130x.txLut[0].lut[6].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[6].pwr_idx = 12;
au_915_928.sx130x.txLut[0].lut[6].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[6].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[6].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[7].rf_power = 19;
au_915_928.sx130x.txLut[0].lut[7].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[7].pwr_idx = 13;
au_915_928.sx130x.txLut[0].lut[7].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[7].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[7].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[8].rf_power = 20;
au_915_928.sx130x.txLut[0].lut[8].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[8].pwr_idx = 14;
au_915_928.sx130x.txLut[0].lut[8].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[8].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[8].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[9].rf_power = 21;
au_915_928.sx130x.txLut[0].lut[9].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[9].pwr_idx = 15;
au_915_928.sx130x.txLut[0].lut[9].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[9].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[9].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[10].rf_power = 22;
au_915_928.sx130x.txLut[0].lut[10].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[10].pwr_idx = 16;
au_915_928.sx130x.txLut[0].lut[10].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[10].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[10].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[11].rf_power = 23;
au_915_928.sx130x.txLut[0].lut[11].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[11].pwr_idx = 17;
au_915_928.sx130x.txLut[0].lut[11].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[11].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[11].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[12].rf_power = 24;
au_915_928.sx130x.txLut[0].lut[12].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[12].pwr_idx = 18;
au_915_928.sx130x.txLut[0].lut[12].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[12].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[12].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[13].rf_power = 25;
au_915_928.sx130x.txLut[0].lut[13].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[13].pwr_idx = 19;
au_915_928.sx130x.txLut[0].lut[13].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[13].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[13].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[14].rf_power = 26;
au_915_928.sx130x.txLut[0].lut[14].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[14].pwr_idx = 21;
au_915_928.sx130x.txLut[0].lut[14].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[14].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[14].mix_gain = 5;
au_915_928.sx130x.txLut[0].lut[15].rf_power = 27;
au_915_928.sx130x.txLut[0].lut[15].pa_gain = 1;
au_915_928.sx130x.txLut[0].lut[15].pwr_idx = 22;
au_915_928.sx130x.txLut[0].lut[15].dig_gain = 0;
au_915_928.sx130x.txLut[0].lut[15].dac_gain = 0;
au_915_928.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

au_915_928.sx130x.rfConfs[1].enable = true;
au_915_928.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
au_915_928.sx130x.rfConfs[1].freq_hz = 917900000;
au_915_928.sx130x.rfConfs[1].rssi_offset = -215.4;
au_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
au_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
au_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
au_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
au_915_928.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
au_915_928.sx130x.rfConfs[1].tx_enable = false;
au_915_928.sx130x.rfConfs[1].single_input_mode = false;
au_915_928.sx130x.tx_freq_min[1] = 0;
au_915_928.sx130x.tx_freq_max[1] = 0;
au_915_928.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

au_915_928.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
au_915_928.sx130x.ifConfs[0].enable = true;
au_915_928.sx130x.ifConfs[0].rf_chain = 0;
au_915_928.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
au_915_928.sx130x.ifConfs[1].enable = true;
au_915_928.sx130x.ifConfs[1].rf_chain = 0;
au_915_928.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
au_915_928.sx130x.ifConfs[2].enable = true;
au_915_928.sx130x.ifConfs[2].rf_chain = 0;
au_915_928.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
au_915_928.sx130x.ifConfs[3].enable = true;
au_915_928.sx130x.ifConfs[3].rf_chain = 0;
au_915_928.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
au_915_928.sx130x.ifConfs[4].enable = true;
au_915_928.sx130x.ifConfs[4].rf_chain = 1;
au_915_928.sx130x.ifConfs[4].freq_hz = -300000;
// chan_multiSF_5
au_915_928.sx130x.ifConfs[5].enable = true;
au_915_928.sx130x.ifConfs[5].rf_chain = 1;
au_915_928.sx130x.ifConfs[5].freq_hz = -100000;
// chan_multiSF_6
au_915_928.sx130x.ifConfs[6].enable = true;
au_915_928.sx130x.ifConfs[6].rf_chain = 1;
au_915_928.sx130x.ifConfs[6].freq_hz = 100000;
// chan_multiSF_7
au_915_928.sx130x.ifConfs[7].enable = true;
au_915_928.sx130x.ifConfs[7].rf_chain = 1;
au_915_928.sx130x.ifConfs[7].freq_hz = 300000;
// Lora std 
au_915_928.sx130x.ifStdConf.enable = true;
au_915_928.sx130x.ifStdConf.rf_chain = 0;
au_915_928.sx130x.ifStdConf.freq_hz = 300000;
au_915_928.sx130x.ifStdConf.bandwidth = 6;
au_915_928.sx130x.ifStdConf.datarate = 8;
au_915_928.sx130x.ifStdConf.implicit_hdr = false;
au_915_928.sx130x.ifStdConf.implicit_payload_length = 17;
au_915_928.sx130x.ifStdConf.implicit_crc_en = false;
au_915_928.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
au_915_928.sx130x.ifFSKConf.enable = false;
au_915_928.sx130x.ifFSKConf.rf_chain = 1;
au_915_928.sx130x.ifFSKConf.freq_hz = 300000;
au_915_928.sx130x.ifFSKConf.bandwidth = 4;
au_915_928.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

au_915_928.gateway.gatewayId = 0xaa555a0000000000;
au_915_928.gateway.serverPortUp = 1700;
au_915_928.gateway.serverPortDown = 1700;
au_915_928.gateway.keepaliveInterval = 10;
au_915_928.gateway.statInterval = 30;
au_915_928.gateway.pushTimeoutMs.tv_sec = 0;
au_915_928.gateway.pushTimeoutMs.tv_usec = 50000;
au_915_928.gateway.forwardCRCValid = true;
au_915_928.gateway.forwardCRCError = false;
au_915_928.gateway.forwardCRCDisabled = false;
au_915_928.gateway.refGeoCoordinates.lat = 0;
au_915_928.gateway.refGeoCoordinates.lon = 0;
au_915_928.gateway.refGeoCoordinates.alt = 0;
au_915_928.gateway.fakeGPS = false;
au_915_928.gateway.beaconPeriod = 0;
au_915_928.gateway.beaconFreqHz = 923300000;
au_915_928.gateway.beaconFreqNb = 8;
au_915_928.gateway.beaconFreqStep = 600000;
au_915_928.gateway.beaconDataRate = 12;
au_915_928.gateway.beaconBandwidthHz = 500000;
au_915_928.gateway.beaconInfoDesc = 0;
au_915_928.gateway.autoQuitThreshold = 0;
au_915_928.serverAddr = "au1.cloud.thethings.network";
au_915_928.gpsTtyPath = "/dev/ttyAMA0";
au_915_928.name = "au 915 928";

// Debug nb_ref_payload, count: 2

au_915_928.debug.nb_ref_payload = 2;
au_915_928.debug.ref_payload[0].id = 0xcafe1234;
au_915_928.debug.ref_payload[1].id = 0xcafe2345;
strcpy(au_915_928.debug.log_file_name, "loragw_hal.log");

};   // au_915_928

void setup_cn_470_510(MemGatewaySettingsStorage &cn_470_510) {

// SX1261 

strcpy(cn_470_510.sx1261.sx1261.spi_path, "");
cn_470_510.sx1261.sx1261.rssi_offset = 0;
cn_470_510.sx1261.spectralScan.enable = false;
cn_470_510.sx1261.spectralScan.freq_hz_start = 0;
cn_470_510.sx1261.spectralScan.nb_chan = 0;
cn_470_510.sx1261.spectralScan.nb_scan = (int) 0;
cn_470_510.sx1261.spectralScan.pace_s = 0;
cn_470_510.sx1261.lbt.enable = false;
cn_470_510.sx1261.lbt.nb_channel = 0;

// SX130x 

cn_470_510.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(cn_470_510.sx130x.boardConf.com_path, "/dev/ttyACM0");
cn_470_510.sx130x.boardConf.lorawan_public = true;
cn_470_510.sx130x.boardConf.clksrc = 0;
cn_470_510.sx130x.antennaGain = 0;
cn_470_510.sx130x.boardConf.full_duplex = false;
cn_470_510.sx130x.tsConf.enable = false;
cn_470_510.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

cn_470_510.sx130x.rfConfs[0].enable = true;
cn_470_510.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
cn_470_510.sx130x.rfConfs[0].freq_hz = 486600000;
cn_470_510.sx130x.rfConfs[0].rssi_offset = -207;
cn_470_510.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
cn_470_510.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
cn_470_510.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
cn_470_510.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
cn_470_510.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
cn_470_510.sx130x.rfConfs[0].tx_enable = true;
cn_470_510.sx130x.rfConfs[0].single_input_mode = true;
cn_470_510.sx130x.tx_freq_min[0] = 470000000;
cn_470_510.sx130x.tx_freq_max[0] = 510000000;
cn_470_510.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

cn_470_510.sx130x.txLut[0].lut[0].rf_power = -6;
cn_470_510.sx130x.txLut[0].lut[0].pa_gain = 0;
cn_470_510.sx130x.txLut[0].lut[0].pwr_idx = 0;
cn_470_510.sx130x.txLut[0].lut[0].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[0].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[0].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[1].rf_power = -3;
cn_470_510.sx130x.txLut[0].lut[1].pa_gain = 0;
cn_470_510.sx130x.txLut[0].lut[1].pwr_idx = 1;
cn_470_510.sx130x.txLut[0].lut[1].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[1].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[1].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[2].rf_power = 0;
cn_470_510.sx130x.txLut[0].lut[2].pa_gain = 0;
cn_470_510.sx130x.txLut[0].lut[2].pwr_idx = 2;
cn_470_510.sx130x.txLut[0].lut[2].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[2].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[2].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[3].rf_power = 3;
cn_470_510.sx130x.txLut[0].lut[3].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[3].pwr_idx = 3;
cn_470_510.sx130x.txLut[0].lut[3].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[3].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[3].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[4].rf_power = 6;
cn_470_510.sx130x.txLut[0].lut[4].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[4].pwr_idx = 4;
cn_470_510.sx130x.txLut[0].lut[4].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[4].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[4].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[5].rf_power = 10;
cn_470_510.sx130x.txLut[0].lut[5].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[5].pwr_idx = 5;
cn_470_510.sx130x.txLut[0].lut[5].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[5].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[5].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[6].rf_power = 11;
cn_470_510.sx130x.txLut[0].lut[6].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[6].pwr_idx = 6;
cn_470_510.sx130x.txLut[0].lut[6].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[6].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[6].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[7].rf_power = 12;
cn_470_510.sx130x.txLut[0].lut[7].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[7].pwr_idx = 7;
cn_470_510.sx130x.txLut[0].lut[7].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[7].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[7].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[8].rf_power = 13;
cn_470_510.sx130x.txLut[0].lut[8].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[8].pwr_idx = 8;
cn_470_510.sx130x.txLut[0].lut[8].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[8].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[8].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[9].rf_power = 14;
cn_470_510.sx130x.txLut[0].lut[9].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[9].pwr_idx = 9;
cn_470_510.sx130x.txLut[0].lut[9].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[9].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[9].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[10].rf_power = 16;
cn_470_510.sx130x.txLut[0].lut[10].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[10].pwr_idx = 10;
cn_470_510.sx130x.txLut[0].lut[10].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[10].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[10].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[11].rf_power = 20;
cn_470_510.sx130x.txLut[0].lut[11].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[11].pwr_idx = 11;
cn_470_510.sx130x.txLut[0].lut[11].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[11].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[11].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[12].rf_power = 23;
cn_470_510.sx130x.txLut[0].lut[12].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[12].pwr_idx = 12;
cn_470_510.sx130x.txLut[0].lut[12].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[12].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[12].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[13].rf_power = 25;
cn_470_510.sx130x.txLut[0].lut[13].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[13].pwr_idx = 13;
cn_470_510.sx130x.txLut[0].lut[13].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[13].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[13].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[14].rf_power = 26;
cn_470_510.sx130x.txLut[0].lut[14].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[14].pwr_idx = 14;
cn_470_510.sx130x.txLut[0].lut[14].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[14].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[14].mix_gain = 5;
cn_470_510.sx130x.txLut[0].lut[15].rf_power = 27;
cn_470_510.sx130x.txLut[0].lut[15].pa_gain = 1;
cn_470_510.sx130x.txLut[0].lut[15].pwr_idx = 15;
cn_470_510.sx130x.txLut[0].lut[15].dig_gain = 0;
cn_470_510.sx130x.txLut[0].lut[15].dac_gain = 0;
cn_470_510.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

cn_470_510.sx130x.rfConfs[1].enable = true;
cn_470_510.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
cn_470_510.sx130x.rfConfs[1].freq_hz = 487400000;
cn_470_510.sx130x.rfConfs[1].rssi_offset = -207;
cn_470_510.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
cn_470_510.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
cn_470_510.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
cn_470_510.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
cn_470_510.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
cn_470_510.sx130x.rfConfs[1].tx_enable = false;
cn_470_510.sx130x.rfConfs[1].single_input_mode = true;
cn_470_510.sx130x.tx_freq_min[1] = 0;
cn_470_510.sx130x.tx_freq_max[1] = 0;
cn_470_510.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

cn_470_510.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
cn_470_510.sx130x.ifConfs[0].enable = true;
cn_470_510.sx130x.ifConfs[0].rf_chain = 1;
cn_470_510.sx130x.ifConfs[0].freq_hz = -300000;
// chan_multiSF_1
cn_470_510.sx130x.ifConfs[1].enable = true;
cn_470_510.sx130x.ifConfs[1].rf_chain = 1;
cn_470_510.sx130x.ifConfs[1].freq_hz = -100000;
// chan_multiSF_2
cn_470_510.sx130x.ifConfs[2].enable = true;
cn_470_510.sx130x.ifConfs[2].rf_chain = 1;
cn_470_510.sx130x.ifConfs[2].freq_hz = 100000;
// chan_multiSF_3
cn_470_510.sx130x.ifConfs[3].enable = true;
cn_470_510.sx130x.ifConfs[3].rf_chain = 1;
cn_470_510.sx130x.ifConfs[3].freq_hz = 300000;
// chan_multiSF_4
cn_470_510.sx130x.ifConfs[4].enable = true;
cn_470_510.sx130x.ifConfs[4].rf_chain = 0;
cn_470_510.sx130x.ifConfs[4].freq_hz = -300000;
// chan_multiSF_5
cn_470_510.sx130x.ifConfs[5].enable = true;
cn_470_510.sx130x.ifConfs[5].rf_chain = 0;
cn_470_510.sx130x.ifConfs[5].freq_hz = -100000;
// chan_multiSF_6
cn_470_510.sx130x.ifConfs[6].enable = true;
cn_470_510.sx130x.ifConfs[6].rf_chain = 0;
cn_470_510.sx130x.ifConfs[6].freq_hz = 100000;
// chan_multiSF_7
cn_470_510.sx130x.ifConfs[7].enable = true;
cn_470_510.sx130x.ifConfs[7].rf_chain = 0;
cn_470_510.sx130x.ifConfs[7].freq_hz = 300000;
// Lora std 
cn_470_510.sx130x.ifStdConf.enable = false;
cn_470_510.sx130x.ifStdConf.rf_chain = 1;
cn_470_510.sx130x.ifStdConf.freq_hz = -200000;
cn_470_510.sx130x.ifStdConf.bandwidth = 5;
cn_470_510.sx130x.ifStdConf.datarate = 7;
cn_470_510.sx130x.ifStdConf.implicit_hdr = false;
cn_470_510.sx130x.ifStdConf.implicit_payload_length = 17;
cn_470_510.sx130x.ifStdConf.implicit_crc_en = false;
cn_470_510.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
cn_470_510.sx130x.ifFSKConf.enable = false;
cn_470_510.sx130x.ifFSKConf.rf_chain = 1;
cn_470_510.sx130x.ifFSKConf.freq_hz = 300000;
cn_470_510.sx130x.ifFSKConf.bandwidth = 4;
cn_470_510.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

cn_470_510.gateway.gatewayId = 0x0;
cn_470_510.gateway.serverPortUp = 1700;
cn_470_510.gateway.serverPortDown = 1700;
cn_470_510.gateway.keepaliveInterval = 10;
cn_470_510.gateway.statInterval = 30;
cn_470_510.gateway.pushTimeoutMs.tv_sec = 0;
cn_470_510.gateway.pushTimeoutMs.tv_usec = 50000;
cn_470_510.gateway.forwardCRCValid = true;
cn_470_510.gateway.forwardCRCError = false;
cn_470_510.gateway.forwardCRCDisabled = false;
cn_470_510.gateway.refGeoCoordinates.lat = 0;
cn_470_510.gateway.refGeoCoordinates.lon = 0;
cn_470_510.gateway.refGeoCoordinates.alt = 0;
cn_470_510.gateway.fakeGPS = false;
cn_470_510.gateway.beaconPeriod = 0;
cn_470_510.gateway.beaconFreqHz = 508300000;
cn_470_510.gateway.beaconFreqNb = 8;
cn_470_510.gateway.beaconFreqStep = 200000;
cn_470_510.gateway.beaconDataRate = 10;
cn_470_510.gateway.beaconBandwidthHz = 125000;
cn_470_510.gateway.beaconInfoDesc = 0;
cn_470_510.gateway.autoQuitThreshold = 20;
cn_470_510.serverAddr = "au1.cloud.thethings.network";
cn_470_510.gpsTtyPath = "/dev/ttyAMA0";
cn_470_510.name = "cn 470 510";

// Debug nb_ref_payload, count: 2

cn_470_510.debug.nb_ref_payload = 2;
cn_470_510.debug.ref_payload[0].id = 0xcafe1234;
cn_470_510.debug.ref_payload[1].id = 0xcafe2345;
strcpy(cn_470_510.debug.log_file_name, "loragw_hal.log");

};   // cn_470_510

void setup_eu_433(MemGatewaySettingsStorage &eu_433) {

// SX1261 

strcpy(eu_433.sx1261.sx1261.spi_path, "");
eu_433.sx1261.sx1261.rssi_offset = 0;
eu_433.sx1261.spectralScan.enable = false;
eu_433.sx1261.spectralScan.freq_hz_start = 0;
eu_433.sx1261.spectralScan.nb_chan = 0;
eu_433.sx1261.spectralScan.nb_scan = (int) 0;
eu_433.sx1261.spectralScan.pace_s = 0;
eu_433.sx1261.lbt.enable = false;
eu_433.sx1261.lbt.nb_channel = 0;

// SX130x 

eu_433.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(eu_433.sx130x.boardConf.com_path, "/dev/ttyACM0");
eu_433.sx130x.boardConf.lorawan_public = true;
eu_433.sx130x.boardConf.clksrc = 0;
eu_433.sx130x.antennaGain = 0;
eu_433.sx130x.boardConf.full_duplex = false;
eu_433.sx130x.tsConf.enable = false;
eu_433.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

eu_433.sx130x.rfConfs[0].enable = true;
eu_433.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
eu_433.sx130x.rfConfs[0].freq_hz = 434375000;
eu_433.sx130x.rfConfs[0].rssi_offset = -207;
eu_433.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
eu_433.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
eu_433.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
eu_433.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
eu_433.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
eu_433.sx130x.rfConfs[0].tx_enable = true;
eu_433.sx130x.rfConfs[0].single_input_mode = true;
eu_433.sx130x.tx_freq_min[0] = 433050000;
eu_433.sx130x.tx_freq_max[0] = 434900000;
eu_433.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

eu_433.sx130x.txLut[0].lut[0].rf_power = -6;
eu_433.sx130x.txLut[0].lut[0].pa_gain = 0;
eu_433.sx130x.txLut[0].lut[0].pwr_idx = 0;
eu_433.sx130x.txLut[0].lut[0].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[0].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[0].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[1].rf_power = -3;
eu_433.sx130x.txLut[0].lut[1].pa_gain = 0;
eu_433.sx130x.txLut[0].lut[1].pwr_idx = 1;
eu_433.sx130x.txLut[0].lut[1].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[1].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[1].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[2].rf_power = 0;
eu_433.sx130x.txLut[0].lut[2].pa_gain = 0;
eu_433.sx130x.txLut[0].lut[2].pwr_idx = 2;
eu_433.sx130x.txLut[0].lut[2].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[2].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[2].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[3].rf_power = 3;
eu_433.sx130x.txLut[0].lut[3].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[3].pwr_idx = 3;
eu_433.sx130x.txLut[0].lut[3].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[3].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[3].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[4].rf_power = 6;
eu_433.sx130x.txLut[0].lut[4].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[4].pwr_idx = 4;
eu_433.sx130x.txLut[0].lut[4].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[4].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[4].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[5].rf_power = 10;
eu_433.sx130x.txLut[0].lut[5].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[5].pwr_idx = 5;
eu_433.sx130x.txLut[0].lut[5].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[5].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[5].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[6].rf_power = 11;
eu_433.sx130x.txLut[0].lut[6].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[6].pwr_idx = 6;
eu_433.sx130x.txLut[0].lut[6].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[6].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[6].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[7].rf_power = 12;
eu_433.sx130x.txLut[0].lut[7].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[7].pwr_idx = 7;
eu_433.sx130x.txLut[0].lut[7].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[7].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[7].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[8].rf_power = 13;
eu_433.sx130x.txLut[0].lut[8].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[8].pwr_idx = 8;
eu_433.sx130x.txLut[0].lut[8].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[8].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[8].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[9].rf_power = 14;
eu_433.sx130x.txLut[0].lut[9].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[9].pwr_idx = 9;
eu_433.sx130x.txLut[0].lut[9].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[9].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[9].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[10].rf_power = 16;
eu_433.sx130x.txLut[0].lut[10].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[10].pwr_idx = 10;
eu_433.sx130x.txLut[0].lut[10].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[10].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[10].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[11].rf_power = 20;
eu_433.sx130x.txLut[0].lut[11].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[11].pwr_idx = 11;
eu_433.sx130x.txLut[0].lut[11].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[11].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[11].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[12].rf_power = 23;
eu_433.sx130x.txLut[0].lut[12].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[12].pwr_idx = 12;
eu_433.sx130x.txLut[0].lut[12].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[12].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[12].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[13].rf_power = 25;
eu_433.sx130x.txLut[0].lut[13].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[13].pwr_idx = 13;
eu_433.sx130x.txLut[0].lut[13].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[13].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[13].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[14].rf_power = 26;
eu_433.sx130x.txLut[0].lut[14].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[14].pwr_idx = 14;
eu_433.sx130x.txLut[0].lut[14].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[14].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[14].mix_gain = 5;
eu_433.sx130x.txLut[0].lut[15].rf_power = 27;
eu_433.sx130x.txLut[0].lut[15].pa_gain = 1;
eu_433.sx130x.txLut[0].lut[15].pwr_idx = 15;
eu_433.sx130x.txLut[0].lut[15].dig_gain = 0;
eu_433.sx130x.txLut[0].lut[15].dac_gain = 0;
eu_433.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

eu_433.sx130x.rfConfs[1].enable = true;
eu_433.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
eu_433.sx130x.rfConfs[1].freq_hz = 433575000;
eu_433.sx130x.rfConfs[1].rssi_offset = -207;
eu_433.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
eu_433.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
eu_433.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
eu_433.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
eu_433.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
eu_433.sx130x.rfConfs[1].tx_enable = false;
eu_433.sx130x.rfConfs[1].single_input_mode = true;
eu_433.sx130x.tx_freq_min[1] = 0;
eu_433.sx130x.tx_freq_max[1] = 0;
eu_433.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

eu_433.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
eu_433.sx130x.ifConfs[0].enable = true;
eu_433.sx130x.ifConfs[0].rf_chain = 1;
eu_433.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
eu_433.sx130x.ifConfs[1].enable = true;
eu_433.sx130x.ifConfs[1].rf_chain = 1;
eu_433.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
eu_433.sx130x.ifConfs[2].enable = true;
eu_433.sx130x.ifConfs[2].rf_chain = 1;
eu_433.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
eu_433.sx130x.ifConfs[3].enable = true;
eu_433.sx130x.ifConfs[3].rf_chain = 1;
eu_433.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
eu_433.sx130x.ifConfs[4].enable = true;
eu_433.sx130x.ifConfs[4].rf_chain = 0;
eu_433.sx130x.ifConfs[4].freq_hz = -400000;
// chan_multiSF_5
eu_433.sx130x.ifConfs[5].enable = true;
eu_433.sx130x.ifConfs[5].rf_chain = 0;
eu_433.sx130x.ifConfs[5].freq_hz = -200000;
// chan_multiSF_6
eu_433.sx130x.ifConfs[6].enable = true;
eu_433.sx130x.ifConfs[6].rf_chain = 0;
eu_433.sx130x.ifConfs[6].freq_hz = 0;
// chan_multiSF_7
eu_433.sx130x.ifConfs[7].enable = true;
eu_433.sx130x.ifConfs[7].rf_chain = 0;
eu_433.sx130x.ifConfs[7].freq_hz = 200000;
// Lora std 
eu_433.sx130x.ifStdConf.enable = true;
eu_433.sx130x.ifStdConf.rf_chain = 1;
eu_433.sx130x.ifStdConf.freq_hz = -200000;
eu_433.sx130x.ifStdConf.bandwidth = 5;
eu_433.sx130x.ifStdConf.datarate = 7;
eu_433.sx130x.ifStdConf.implicit_hdr = false;
eu_433.sx130x.ifStdConf.implicit_payload_length = 17;
eu_433.sx130x.ifStdConf.implicit_crc_en = false;
eu_433.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
eu_433.sx130x.ifFSKConf.enable = true;
eu_433.sx130x.ifFSKConf.rf_chain = 1;
eu_433.sx130x.ifFSKConf.freq_hz = 300000;
eu_433.sx130x.ifFSKConf.bandwidth = 4;
eu_433.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

eu_433.gateway.gatewayId = 0x0;
eu_433.gateway.serverPortUp = 1700;
eu_433.gateway.serverPortDown = 1700;
eu_433.gateway.keepaliveInterval = 10;
eu_433.gateway.statInterval = 30;
eu_433.gateway.pushTimeoutMs.tv_sec = 0;
eu_433.gateway.pushTimeoutMs.tv_usec = 50000;
eu_433.gateway.forwardCRCValid = true;
eu_433.gateway.forwardCRCError = false;
eu_433.gateway.forwardCRCDisabled = false;
eu_433.gateway.refGeoCoordinates.lat = 0;
eu_433.gateway.refGeoCoordinates.lon = 0;
eu_433.gateway.refGeoCoordinates.alt = 0;
eu_433.gateway.fakeGPS = false;
eu_433.gateway.beaconPeriod = 0;
eu_433.gateway.beaconFreqHz = 434665000;
eu_433.gateway.beaconFreqNb = 1;
eu_433.gateway.beaconFreqStep = 0;
eu_433.gateway.beaconDataRate = 9;
eu_433.gateway.beaconBandwidthHz = 125000;
eu_433.gateway.beaconInfoDesc = 0;
eu_433.gateway.autoQuitThreshold = 20;
eu_433.serverAddr = "eu1.cloud.thethings.network";
eu_433.gpsTtyPath = "/dev/ttyAMA0";
eu_433.name = "eu 433";

// Debug nb_ref_payload, count: 2

eu_433.debug.nb_ref_payload = 2;
eu_433.debug.ref_payload[0].id = 0xcafe1234;
eu_433.debug.ref_payload[1].id = 0xcafe2345;
strcpy(eu_433.debug.log_file_name, "loragw_hal.log");

};   // eu_433

void setup_eu_863_870(MemGatewaySettingsStorage &eu_863_870) {

// SX1261 

strcpy(eu_863_870.sx1261.sx1261.spi_path, "");
eu_863_870.sx1261.sx1261.rssi_offset = 0;
eu_863_870.sx1261.spectralScan.enable = false;
eu_863_870.sx1261.spectralScan.freq_hz_start = 0;
eu_863_870.sx1261.spectralScan.nb_chan = 0;
eu_863_870.sx1261.spectralScan.nb_scan = (int) 0;
eu_863_870.sx1261.spectralScan.pace_s = 0;
eu_863_870.sx1261.lbt.enable = false;
eu_863_870.sx1261.lbt.nb_channel = 0;

// SX130x 

eu_863_870.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(eu_863_870.sx130x.boardConf.com_path, "/dev/ttyACM0");
eu_863_870.sx130x.boardConf.lorawan_public = true;
eu_863_870.sx130x.boardConf.clksrc = 0;
eu_863_870.sx130x.antennaGain = 0;
eu_863_870.sx130x.boardConf.full_duplex = false;
eu_863_870.sx130x.tsConf.enable = false;
eu_863_870.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

eu_863_870.sx130x.rfConfs[0].enable = true;
eu_863_870.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
eu_863_870.sx130x.rfConfs[0].freq_hz = 867500000;
eu_863_870.sx130x.rfConfs[0].rssi_offset = -215.4;
eu_863_870.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
eu_863_870.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
eu_863_870.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
eu_863_870.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
eu_863_870.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
eu_863_870.sx130x.rfConfs[0].tx_enable = true;
eu_863_870.sx130x.rfConfs[0].single_input_mode = false;
eu_863_870.sx130x.tx_freq_min[0] = 863000000;
eu_863_870.sx130x.tx_freq_max[0] = 870000000;
eu_863_870.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

eu_863_870.sx130x.txLut[0].lut[0].rf_power = 12;
eu_863_870.sx130x.txLut[0].lut[0].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[0].pwr_idx = 4;
eu_863_870.sx130x.txLut[0].lut[0].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[0].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[0].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[1].rf_power = 13;
eu_863_870.sx130x.txLut[0].lut[1].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[1].pwr_idx = 5;
eu_863_870.sx130x.txLut[0].lut[1].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[1].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[1].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[2].rf_power = 14;
eu_863_870.sx130x.txLut[0].lut[2].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[2].pwr_idx = 6;
eu_863_870.sx130x.txLut[0].lut[2].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[2].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[2].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[3].rf_power = 15;
eu_863_870.sx130x.txLut[0].lut[3].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[3].pwr_idx = 7;
eu_863_870.sx130x.txLut[0].lut[3].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[3].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[3].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[4].rf_power = 16;
eu_863_870.sx130x.txLut[0].lut[4].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[4].pwr_idx = 8;
eu_863_870.sx130x.txLut[0].lut[4].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[4].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[4].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[5].rf_power = 17;
eu_863_870.sx130x.txLut[0].lut[5].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[5].pwr_idx = 9;
eu_863_870.sx130x.txLut[0].lut[5].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[5].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[5].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[6].rf_power = 18;
eu_863_870.sx130x.txLut[0].lut[6].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[6].pwr_idx = 10;
eu_863_870.sx130x.txLut[0].lut[6].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[6].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[6].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[7].rf_power = 19;
eu_863_870.sx130x.txLut[0].lut[7].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[7].pwr_idx = 11;
eu_863_870.sx130x.txLut[0].lut[7].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[7].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[7].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[8].rf_power = 20;
eu_863_870.sx130x.txLut[0].lut[8].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[8].pwr_idx = 12;
eu_863_870.sx130x.txLut[0].lut[8].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[8].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[8].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[9].rf_power = 21;
eu_863_870.sx130x.txLut[0].lut[9].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[9].pwr_idx = 13;
eu_863_870.sx130x.txLut[0].lut[9].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[9].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[9].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[10].rf_power = 22;
eu_863_870.sx130x.txLut[0].lut[10].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[10].pwr_idx = 14;
eu_863_870.sx130x.txLut[0].lut[10].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[10].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[10].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[11].rf_power = 23;
eu_863_870.sx130x.txLut[0].lut[11].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[11].pwr_idx = 16;
eu_863_870.sx130x.txLut[0].lut[11].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[11].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[11].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[12].rf_power = 24;
eu_863_870.sx130x.txLut[0].lut[12].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[12].pwr_idx = 17;
eu_863_870.sx130x.txLut[0].lut[12].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[12].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[12].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[13].rf_power = 25;
eu_863_870.sx130x.txLut[0].lut[13].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[13].pwr_idx = 18;
eu_863_870.sx130x.txLut[0].lut[13].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[13].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[13].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[14].rf_power = 26;
eu_863_870.sx130x.txLut[0].lut[14].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[14].pwr_idx = 19;
eu_863_870.sx130x.txLut[0].lut[14].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[14].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[14].mix_gain = 5;
eu_863_870.sx130x.txLut[0].lut[15].rf_power = 27;
eu_863_870.sx130x.txLut[0].lut[15].pa_gain = 1;
eu_863_870.sx130x.txLut[0].lut[15].pwr_idx = 22;
eu_863_870.sx130x.txLut[0].lut[15].dig_gain = 0;
eu_863_870.sx130x.txLut[0].lut[15].dac_gain = 0;
eu_863_870.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

eu_863_870.sx130x.rfConfs[1].enable = true;
eu_863_870.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
eu_863_870.sx130x.rfConfs[1].freq_hz = 868500000;
eu_863_870.sx130x.rfConfs[1].rssi_offset = -215.4;
eu_863_870.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
eu_863_870.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
eu_863_870.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
eu_863_870.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
eu_863_870.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
eu_863_870.sx130x.rfConfs[1].tx_enable = false;
eu_863_870.sx130x.rfConfs[1].single_input_mode = false;
eu_863_870.sx130x.tx_freq_min[1] = 0;
eu_863_870.sx130x.tx_freq_max[1] = 0;
eu_863_870.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

eu_863_870.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
eu_863_870.sx130x.ifConfs[0].enable = true;
eu_863_870.sx130x.ifConfs[0].rf_chain = 1;
eu_863_870.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
eu_863_870.sx130x.ifConfs[1].enable = true;
eu_863_870.sx130x.ifConfs[1].rf_chain = 1;
eu_863_870.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
eu_863_870.sx130x.ifConfs[2].enable = true;
eu_863_870.sx130x.ifConfs[2].rf_chain = 1;
eu_863_870.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
eu_863_870.sx130x.ifConfs[3].enable = true;
eu_863_870.sx130x.ifConfs[3].rf_chain = 0;
eu_863_870.sx130x.ifConfs[3].freq_hz = -400000;
// chan_multiSF_4
eu_863_870.sx130x.ifConfs[4].enable = true;
eu_863_870.sx130x.ifConfs[4].rf_chain = 0;
eu_863_870.sx130x.ifConfs[4].freq_hz = -200000;
// chan_multiSF_5
eu_863_870.sx130x.ifConfs[5].enable = true;
eu_863_870.sx130x.ifConfs[5].rf_chain = 0;
eu_863_870.sx130x.ifConfs[5].freq_hz = 0;
// chan_multiSF_6
eu_863_870.sx130x.ifConfs[6].enable = true;
eu_863_870.sx130x.ifConfs[6].rf_chain = 0;
eu_863_870.sx130x.ifConfs[6].freq_hz = 200000;
// chan_multiSF_7
eu_863_870.sx130x.ifConfs[7].enable = true;
eu_863_870.sx130x.ifConfs[7].rf_chain = 0;
eu_863_870.sx130x.ifConfs[7].freq_hz = 400000;
// Lora std 
eu_863_870.sx130x.ifStdConf.enable = true;
eu_863_870.sx130x.ifStdConf.rf_chain = 1;
eu_863_870.sx130x.ifStdConf.freq_hz = -200000;
eu_863_870.sx130x.ifStdConf.bandwidth = 5;
eu_863_870.sx130x.ifStdConf.datarate = 7;
eu_863_870.sx130x.ifStdConf.implicit_hdr = false;
eu_863_870.sx130x.ifStdConf.implicit_payload_length = 17;
eu_863_870.sx130x.ifStdConf.implicit_crc_en = false;
eu_863_870.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
eu_863_870.sx130x.ifFSKConf.enable = true;
eu_863_870.sx130x.ifFSKConf.rf_chain = 1;
eu_863_870.sx130x.ifFSKConf.freq_hz = 300000;
eu_863_870.sx130x.ifFSKConf.bandwidth = 4;
eu_863_870.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

eu_863_870.gateway.gatewayId = 0xaa555a0000000000;
eu_863_870.gateway.serverPortUp = 1700;
eu_863_870.gateway.serverPortDown = 1700;
eu_863_870.gateway.keepaliveInterval = 10;
eu_863_870.gateway.statInterval = 30;
eu_863_870.gateway.pushTimeoutMs.tv_sec = 0;
eu_863_870.gateway.pushTimeoutMs.tv_usec = 50000;
eu_863_870.gateway.forwardCRCValid = true;
eu_863_870.gateway.forwardCRCError = false;
eu_863_870.gateway.forwardCRCDisabled = false;
eu_863_870.gateway.refGeoCoordinates.lat = 0;
eu_863_870.gateway.refGeoCoordinates.lon = 0;
eu_863_870.gateway.refGeoCoordinates.alt = 0;
eu_863_870.gateway.fakeGPS = false;
eu_863_870.gateway.beaconPeriod = 0;
eu_863_870.gateway.beaconFreqHz = 869525000;
eu_863_870.gateway.beaconFreqNb = 1;
eu_863_870.gateway.beaconFreqStep = 0;
eu_863_870.gateway.beaconDataRate = 9;
eu_863_870.gateway.beaconBandwidthHz = 125000;
eu_863_870.gateway.beaconInfoDesc = 0;
eu_863_870.gateway.autoQuitThreshold = 0;
eu_863_870.serverAddr = "eu1.cloud.thethings.network";
eu_863_870.gpsTtyPath = "/dev/ttyAMA0";
eu_863_870.name = "eu 863 870";

// Debug nb_ref_payload, count: 2

eu_863_870.debug.nb_ref_payload = 2;
eu_863_870.debug.ref_payload[0].id = 0xcafe1234;
eu_863_870.debug.ref_payload[1].id = 0xcafe2345;
strcpy(eu_863_870.debug.log_file_name, "loragw_hal.log");

};   // eu_863_870

void setup_in_865_867(MemGatewaySettingsStorage &in_865_867) {

// SX1261 

strcpy(in_865_867.sx1261.sx1261.spi_path, "");
in_865_867.sx1261.sx1261.rssi_offset = 0;
in_865_867.sx1261.spectralScan.enable = false;
in_865_867.sx1261.spectralScan.freq_hz_start = 0;
in_865_867.sx1261.spectralScan.nb_chan = 0;
in_865_867.sx1261.spectralScan.nb_scan = (int) 0;
in_865_867.sx1261.spectralScan.pace_s = 0;
in_865_867.sx1261.lbt.enable = false;
in_865_867.sx1261.lbt.nb_channel = 0;

// SX130x 

in_865_867.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(in_865_867.sx130x.boardConf.com_path, "/dev/ttyACM0");
in_865_867.sx130x.boardConf.lorawan_public = true;
in_865_867.sx130x.boardConf.clksrc = 0;
in_865_867.sx130x.antennaGain = 0;
in_865_867.sx130x.boardConf.full_duplex = false;
in_865_867.sx130x.tsConf.enable = false;
in_865_867.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

in_865_867.sx130x.rfConfs[0].enable = true;
in_865_867.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
in_865_867.sx130x.rfConfs[0].freq_hz = 865200000;
in_865_867.sx130x.rfConfs[0].rssi_offset = -215.4;
in_865_867.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
in_865_867.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
in_865_867.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
in_865_867.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
in_865_867.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
in_865_867.sx130x.rfConfs[0].tx_enable = true;
in_865_867.sx130x.rfConfs[0].single_input_mode = false;
in_865_867.sx130x.tx_freq_min[0] = 865000000;
in_865_867.sx130x.tx_freq_max[0] = 867000000;
in_865_867.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

in_865_867.sx130x.txLut[0].lut[0].rf_power = 12;
in_865_867.sx130x.txLut[0].lut[0].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[0].pwr_idx = 4;
in_865_867.sx130x.txLut[0].lut[0].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[0].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[0].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[1].rf_power = 13;
in_865_867.sx130x.txLut[0].lut[1].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[1].pwr_idx = 5;
in_865_867.sx130x.txLut[0].lut[1].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[1].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[1].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[2].rf_power = 14;
in_865_867.sx130x.txLut[0].lut[2].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[2].pwr_idx = 6;
in_865_867.sx130x.txLut[0].lut[2].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[2].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[2].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[3].rf_power = 15;
in_865_867.sx130x.txLut[0].lut[3].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[3].pwr_idx = 7;
in_865_867.sx130x.txLut[0].lut[3].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[3].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[3].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[4].rf_power = 16;
in_865_867.sx130x.txLut[0].lut[4].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[4].pwr_idx = 8;
in_865_867.sx130x.txLut[0].lut[4].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[4].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[4].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[5].rf_power = 17;
in_865_867.sx130x.txLut[0].lut[5].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[5].pwr_idx = 9;
in_865_867.sx130x.txLut[0].lut[5].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[5].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[5].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[6].rf_power = 18;
in_865_867.sx130x.txLut[0].lut[6].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[6].pwr_idx = 10;
in_865_867.sx130x.txLut[0].lut[6].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[6].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[6].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[7].rf_power = 19;
in_865_867.sx130x.txLut[0].lut[7].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[7].pwr_idx = 11;
in_865_867.sx130x.txLut[0].lut[7].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[7].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[7].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[8].rf_power = 20;
in_865_867.sx130x.txLut[0].lut[8].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[8].pwr_idx = 12;
in_865_867.sx130x.txLut[0].lut[8].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[8].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[8].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[9].rf_power = 21;
in_865_867.sx130x.txLut[0].lut[9].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[9].pwr_idx = 13;
in_865_867.sx130x.txLut[0].lut[9].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[9].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[9].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[10].rf_power = 22;
in_865_867.sx130x.txLut[0].lut[10].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[10].pwr_idx = 14;
in_865_867.sx130x.txLut[0].lut[10].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[10].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[10].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[11].rf_power = 23;
in_865_867.sx130x.txLut[0].lut[11].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[11].pwr_idx = 16;
in_865_867.sx130x.txLut[0].lut[11].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[11].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[11].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[12].rf_power = 24;
in_865_867.sx130x.txLut[0].lut[12].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[12].pwr_idx = 17;
in_865_867.sx130x.txLut[0].lut[12].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[12].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[12].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[13].rf_power = 25;
in_865_867.sx130x.txLut[0].lut[13].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[13].pwr_idx = 18;
in_865_867.sx130x.txLut[0].lut[13].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[13].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[13].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[14].rf_power = 26;
in_865_867.sx130x.txLut[0].lut[14].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[14].pwr_idx = 19;
in_865_867.sx130x.txLut[0].lut[14].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[14].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[14].mix_gain = 5;
in_865_867.sx130x.txLut[0].lut[15].rf_power = 27;
in_865_867.sx130x.txLut[0].lut[15].pa_gain = 1;
in_865_867.sx130x.txLut[0].lut[15].pwr_idx = 22;
in_865_867.sx130x.txLut[0].lut[15].dig_gain = 0;
in_865_867.sx130x.txLut[0].lut[15].dac_gain = 0;
in_865_867.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

in_865_867.sx130x.rfConfs[1].enable = true;
in_865_867.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
in_865_867.sx130x.rfConfs[1].freq_hz = 866385000;
in_865_867.sx130x.rfConfs[1].rssi_offset = -215.4;
in_865_867.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
in_865_867.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
in_865_867.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
in_865_867.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
in_865_867.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
in_865_867.sx130x.rfConfs[1].tx_enable = false;
in_865_867.sx130x.rfConfs[1].single_input_mode = false;
in_865_867.sx130x.tx_freq_min[1] = 0;
in_865_867.sx130x.tx_freq_max[1] = 0;
in_865_867.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

in_865_867.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
in_865_867.sx130x.ifConfs[0].enable = true;
in_865_867.sx130x.ifConfs[0].rf_chain = 0;
in_865_867.sx130x.ifConfs[0].freq_hz = -137500;
// chan_multiSF_1
in_865_867.sx130x.ifConfs[1].enable = true;
in_865_867.sx130x.ifConfs[1].rf_chain = 0;
in_865_867.sx130x.ifConfs[1].freq_hz = 202500;
// chan_multiSF_2
in_865_867.sx130x.ifConfs[2].enable = true;
in_865_867.sx130x.ifConfs[2].rf_chain = 1;
in_865_867.sx130x.ifConfs[2].freq_hz = -400000;
// chan_multiSF_3
in_865_867.sx130x.ifConfs[3].enable = false;
in_865_867.sx130x.ifConfs[3].rf_chain = 0;
in_865_867.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
in_865_867.sx130x.ifConfs[4].enable = false;
in_865_867.sx130x.ifConfs[4].rf_chain = 1;
in_865_867.sx130x.ifConfs[4].freq_hz = -300000;
// chan_multiSF_5
in_865_867.sx130x.ifConfs[5].enable = false;
in_865_867.sx130x.ifConfs[5].rf_chain = 1;
in_865_867.sx130x.ifConfs[5].freq_hz = -100000;
// chan_multiSF_6
in_865_867.sx130x.ifConfs[6].enable = false;
in_865_867.sx130x.ifConfs[6].rf_chain = 1;
in_865_867.sx130x.ifConfs[6].freq_hz = 100000;
// chan_multiSF_7
in_865_867.sx130x.ifConfs[7].enable = false;
in_865_867.sx130x.ifConfs[7].rf_chain = 1;
in_865_867.sx130x.ifConfs[7].freq_hz = 300000;
// Lora std 
in_865_867.sx130x.ifStdConf.enable = false;
in_865_867.sx130x.ifStdConf.rf_chain = 0;
in_865_867.sx130x.ifStdConf.freq_hz = 300000;
in_865_867.sx130x.ifStdConf.bandwidth = 6;
in_865_867.sx130x.ifStdConf.datarate = 8;
in_865_867.sx130x.ifStdConf.implicit_hdr = false;
in_865_867.sx130x.ifStdConf.implicit_payload_length = 17;
in_865_867.sx130x.ifStdConf.implicit_crc_en = false;
in_865_867.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
in_865_867.sx130x.ifFSKConf.enable = false;
in_865_867.sx130x.ifFSKConf.rf_chain = 1;
in_865_867.sx130x.ifFSKConf.freq_hz = 300000;
in_865_867.sx130x.ifFSKConf.bandwidth = 4;
in_865_867.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

in_865_867.gateway.gatewayId = 0xaa555a0000000000;
in_865_867.gateway.serverPortUp = 1700;
in_865_867.gateway.serverPortDown = 1700;
in_865_867.gateway.keepaliveInterval = 10;
in_865_867.gateway.statInterval = 30;
in_865_867.gateway.pushTimeoutMs.tv_sec = 0;
in_865_867.gateway.pushTimeoutMs.tv_usec = 50000;
in_865_867.gateway.forwardCRCValid = true;
in_865_867.gateway.forwardCRCError = false;
in_865_867.gateway.forwardCRCDisabled = false;
in_865_867.gateway.refGeoCoordinates.lat = 0;
in_865_867.gateway.refGeoCoordinates.lon = 0;
in_865_867.gateway.refGeoCoordinates.alt = 0;
in_865_867.gateway.fakeGPS = false;
in_865_867.gateway.beaconPeriod = 0;
in_865_867.gateway.beaconFreqHz = 866500000;
in_865_867.gateway.beaconFreqNb = 1;
in_865_867.gateway.beaconFreqStep = 0;
in_865_867.gateway.beaconDataRate = 8;
in_865_867.gateway.beaconBandwidthHz = 125000;
in_865_867.gateway.beaconInfoDesc = 0;
in_865_867.gateway.autoQuitThreshold = 0;
in_865_867.serverAddr = "eu1.cloud.thethings.network";
in_865_867.gpsTtyPath = "/dev/ttyAMA0";
in_865_867.name = "in 865 867";

// Debug nb_ref_payload, count: 2

in_865_867.debug.nb_ref_payload = 2;
in_865_867.debug.ref_payload[0].id = 0xcafe1234;
in_865_867.debug.ref_payload[1].id = 0xcafe2345;
strcpy(in_865_867.debug.log_file_name, "loragw_hal.log");

};   // in_865_867

void setup_kr_920_923(MemGatewaySettingsStorage &kr_920_923) {

// SX1261 

strcpy(kr_920_923.sx1261.sx1261.spi_path, "");
kr_920_923.sx1261.sx1261.rssi_offset = 0;
kr_920_923.sx1261.spectralScan.enable = false;
kr_920_923.sx1261.spectralScan.freq_hz_start = 0;
kr_920_923.sx1261.spectralScan.nb_chan = 0;
kr_920_923.sx1261.spectralScan.nb_scan = (int) 0;
kr_920_923.sx1261.spectralScan.pace_s = 0;
kr_920_923.sx1261.lbt.enable = false;
kr_920_923.sx1261.lbt.nb_channel = 0;

// SX130x 

kr_920_923.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(kr_920_923.sx130x.boardConf.com_path, "/dev/ttyACM0");
kr_920_923.sx130x.boardConf.lorawan_public = true;
kr_920_923.sx130x.boardConf.clksrc = 0;
kr_920_923.sx130x.antennaGain = 0;
kr_920_923.sx130x.boardConf.full_duplex = false;
kr_920_923.sx130x.tsConf.enable = false;
kr_920_923.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

kr_920_923.sx130x.rfConfs[0].enable = true;
kr_920_923.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
kr_920_923.sx130x.rfConfs[0].freq_hz = 922400000;
kr_920_923.sx130x.rfConfs[0].rssi_offset = -215.4;
kr_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
kr_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
kr_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
kr_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
kr_920_923.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
kr_920_923.sx130x.rfConfs[0].tx_enable = true;
kr_920_923.sx130x.rfConfs[0].single_input_mode = false;
kr_920_923.sx130x.tx_freq_min[0] = 920900000;
kr_920_923.sx130x.tx_freq_max[0] = 923300000;
kr_920_923.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

kr_920_923.sx130x.txLut[0].lut[0].rf_power = 12;
kr_920_923.sx130x.txLut[0].lut[0].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[0].pwr_idx = 6;
kr_920_923.sx130x.txLut[0].lut[0].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[0].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[0].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[1].rf_power = 13;
kr_920_923.sx130x.txLut[0].lut[1].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[1].pwr_idx = 7;
kr_920_923.sx130x.txLut[0].lut[1].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[1].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[1].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[2].rf_power = 14;
kr_920_923.sx130x.txLut[0].lut[2].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[2].pwr_idx = 8;
kr_920_923.sx130x.txLut[0].lut[2].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[2].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[2].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[3].rf_power = 15;
kr_920_923.sx130x.txLut[0].lut[3].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[3].pwr_idx = 9;
kr_920_923.sx130x.txLut[0].lut[3].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[3].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[3].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[4].rf_power = 16;
kr_920_923.sx130x.txLut[0].lut[4].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[4].pwr_idx = 10;
kr_920_923.sx130x.txLut[0].lut[4].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[4].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[4].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[5].rf_power = 17;
kr_920_923.sx130x.txLut[0].lut[5].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[5].pwr_idx = 11;
kr_920_923.sx130x.txLut[0].lut[5].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[5].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[5].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[6].rf_power = 18;
kr_920_923.sx130x.txLut[0].lut[6].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[6].pwr_idx = 12;
kr_920_923.sx130x.txLut[0].lut[6].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[6].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[6].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[7].rf_power = 19;
kr_920_923.sx130x.txLut[0].lut[7].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[7].pwr_idx = 13;
kr_920_923.sx130x.txLut[0].lut[7].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[7].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[7].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[8].rf_power = 20;
kr_920_923.sx130x.txLut[0].lut[8].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[8].pwr_idx = 14;
kr_920_923.sx130x.txLut[0].lut[8].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[8].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[8].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[9].rf_power = 21;
kr_920_923.sx130x.txLut[0].lut[9].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[9].pwr_idx = 15;
kr_920_923.sx130x.txLut[0].lut[9].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[9].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[9].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[10].rf_power = 22;
kr_920_923.sx130x.txLut[0].lut[10].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[10].pwr_idx = 16;
kr_920_923.sx130x.txLut[0].lut[10].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[10].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[10].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[11].rf_power = 23;
kr_920_923.sx130x.txLut[0].lut[11].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[11].pwr_idx = 17;
kr_920_923.sx130x.txLut[0].lut[11].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[11].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[11].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[12].rf_power = 24;
kr_920_923.sx130x.txLut[0].lut[12].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[12].pwr_idx = 18;
kr_920_923.sx130x.txLut[0].lut[12].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[12].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[12].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[13].rf_power = 25;
kr_920_923.sx130x.txLut[0].lut[13].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[13].pwr_idx = 19;
kr_920_923.sx130x.txLut[0].lut[13].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[13].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[13].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[14].rf_power = 26;
kr_920_923.sx130x.txLut[0].lut[14].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[14].pwr_idx = 21;
kr_920_923.sx130x.txLut[0].lut[14].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[14].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[14].mix_gain = 5;
kr_920_923.sx130x.txLut[0].lut[15].rf_power = 27;
kr_920_923.sx130x.txLut[0].lut[15].pa_gain = 1;
kr_920_923.sx130x.txLut[0].lut[15].pwr_idx = 22;
kr_920_923.sx130x.txLut[0].lut[15].dig_gain = 0;
kr_920_923.sx130x.txLut[0].lut[15].dac_gain = 0;
kr_920_923.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

kr_920_923.sx130x.rfConfs[1].enable = true;
kr_920_923.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
kr_920_923.sx130x.rfConfs[1].freq_hz = 923000000;
kr_920_923.sx130x.rfConfs[1].rssi_offset = -215.4;
kr_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
kr_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
kr_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
kr_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
kr_920_923.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
kr_920_923.sx130x.rfConfs[1].tx_enable = false;
kr_920_923.sx130x.rfConfs[1].single_input_mode = false;
kr_920_923.sx130x.tx_freq_min[1] = 0;
kr_920_923.sx130x.tx_freq_max[1] = 0;
kr_920_923.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

kr_920_923.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
kr_920_923.sx130x.ifConfs[0].enable = true;
kr_920_923.sx130x.ifConfs[0].rf_chain = 0;
kr_920_923.sx130x.ifConfs[0].freq_hz = -300000;
// chan_multiSF_1
kr_920_923.sx130x.ifConfs[1].enable = true;
kr_920_923.sx130x.ifConfs[1].rf_chain = 0;
kr_920_923.sx130x.ifConfs[1].freq_hz = -100000;
// chan_multiSF_2
kr_920_923.sx130x.ifConfs[2].enable = true;
kr_920_923.sx130x.ifConfs[2].rf_chain = 0;
kr_920_923.sx130x.ifConfs[2].freq_hz = 100000;
// chan_multiSF_3
kr_920_923.sx130x.ifConfs[3].enable = true;
kr_920_923.sx130x.ifConfs[3].rf_chain = 0;
kr_920_923.sx130x.ifConfs[3].freq_hz = 300000;
// chan_multiSF_4
kr_920_923.sx130x.ifConfs[4].enable = true;
kr_920_923.sx130x.ifConfs[4].rf_chain = 1;
kr_920_923.sx130x.ifConfs[4].freq_hz = -100000;
// chan_multiSF_5
kr_920_923.sx130x.ifConfs[5].enable = true;
kr_920_923.sx130x.ifConfs[5].rf_chain = 1;
kr_920_923.sx130x.ifConfs[5].freq_hz = 100000;
// chan_multiSF_6
kr_920_923.sx130x.ifConfs[6].enable = true;
kr_920_923.sx130x.ifConfs[6].rf_chain = 1;
kr_920_923.sx130x.ifConfs[6].freq_hz = 300000;
// chan_multiSF_7
kr_920_923.sx130x.ifConfs[7].enable = false;
kr_920_923.sx130x.ifConfs[7].rf_chain = 1;
kr_920_923.sx130x.ifConfs[7].freq_hz = 300000;
// Lora std 
kr_920_923.sx130x.ifStdConf.enable = false;
kr_920_923.sx130x.ifStdConf.rf_chain = 0;
kr_920_923.sx130x.ifStdConf.freq_hz = 300000;
kr_920_923.sx130x.ifStdConf.bandwidth = 6;
kr_920_923.sx130x.ifStdConf.datarate = 8;
kr_920_923.sx130x.ifStdConf.implicit_hdr = false;
kr_920_923.sx130x.ifStdConf.implicit_payload_length = 17;
kr_920_923.sx130x.ifStdConf.implicit_crc_en = false;
kr_920_923.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
kr_920_923.sx130x.ifFSKConf.enable = false;
kr_920_923.sx130x.ifFSKConf.rf_chain = 1;
kr_920_923.sx130x.ifFSKConf.freq_hz = 300000;
kr_920_923.sx130x.ifFSKConf.bandwidth = 4;
kr_920_923.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

kr_920_923.gateway.gatewayId = 0xaa555a0000000000;
kr_920_923.gateway.serverPortUp = 1700;
kr_920_923.gateway.serverPortDown = 1700;
kr_920_923.gateway.keepaliveInterval = 10;
kr_920_923.gateway.statInterval = 30;
kr_920_923.gateway.pushTimeoutMs.tv_sec = 0;
kr_920_923.gateway.pushTimeoutMs.tv_usec = 50000;
kr_920_923.gateway.forwardCRCValid = true;
kr_920_923.gateway.forwardCRCError = false;
kr_920_923.gateway.forwardCRCDisabled = false;
kr_920_923.gateway.refGeoCoordinates.lat = 0;
kr_920_923.gateway.refGeoCoordinates.lon = 0;
kr_920_923.gateway.refGeoCoordinates.alt = 0;
kr_920_923.gateway.fakeGPS = false;
kr_920_923.gateway.beaconPeriod = 0;
kr_920_923.gateway.beaconFreqHz = 923100000;
kr_920_923.gateway.beaconFreqNb = 1;
kr_920_923.gateway.beaconFreqStep = 0;
kr_920_923.gateway.beaconDataRate = 9;
kr_920_923.gateway.beaconBandwidthHz = 125000;
kr_920_923.gateway.beaconInfoDesc = 0;
kr_920_923.gateway.autoQuitThreshold = 0;
kr_920_923.serverAddr = "nam1.cloud.thethings.network";
kr_920_923.gpsTtyPath = "/dev/ttyAMA0";
kr_920_923.name = "kr 920 923";

// Debug nb_ref_payload, count: 2

kr_920_923.debug.nb_ref_payload = 2;
kr_920_923.debug.ref_payload[0].id = 0xcafe1234;
kr_920_923.debug.ref_payload[1].id = 0xcafe2345;
strcpy(kr_920_923.debug.log_file_name, "loragw_hal.log");

};   // kr_920_923

void setup_ru_864_870(MemGatewaySettingsStorage &ru_864_870) {

// SX1261 

strcpy(ru_864_870.sx1261.sx1261.spi_path, "");
ru_864_870.sx1261.sx1261.rssi_offset = 0;
ru_864_870.sx1261.spectralScan.enable = false;
ru_864_870.sx1261.spectralScan.freq_hz_start = 0;
ru_864_870.sx1261.spectralScan.nb_chan = 0;
ru_864_870.sx1261.spectralScan.nb_scan = (int) 0;
ru_864_870.sx1261.spectralScan.pace_s = 0;
ru_864_870.sx1261.lbt.enable = false;
ru_864_870.sx1261.lbt.nb_channel = 0;

// SX130x 

ru_864_870.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(ru_864_870.sx130x.boardConf.com_path, "/dev/ttyACM0");
ru_864_870.sx130x.boardConf.lorawan_public = true;
ru_864_870.sx130x.boardConf.clksrc = 0;
ru_864_870.sx130x.antennaGain = 0;
ru_864_870.sx130x.boardConf.full_duplex = false;
ru_864_870.sx130x.tsConf.enable = false;
ru_864_870.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

ru_864_870.sx130x.rfConfs[0].enable = true;
ru_864_870.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
ru_864_870.sx130x.rfConfs[0].freq_hz = 864500000;
ru_864_870.sx130x.rfConfs[0].rssi_offset = -215.4;
ru_864_870.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
ru_864_870.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
ru_864_870.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
ru_864_870.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
ru_864_870.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
ru_864_870.sx130x.rfConfs[0].tx_enable = true;
ru_864_870.sx130x.rfConfs[0].single_input_mode = false;
ru_864_870.sx130x.tx_freq_min[0] = 863000000;
ru_864_870.sx130x.tx_freq_max[0] = 870000000;
ru_864_870.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

ru_864_870.sx130x.txLut[0].lut[0].rf_power = 12;
ru_864_870.sx130x.txLut[0].lut[0].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[0].pwr_idx = 4;
ru_864_870.sx130x.txLut[0].lut[0].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[0].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[0].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[1].rf_power = 13;
ru_864_870.sx130x.txLut[0].lut[1].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[1].pwr_idx = 5;
ru_864_870.sx130x.txLut[0].lut[1].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[1].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[1].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[2].rf_power = 14;
ru_864_870.sx130x.txLut[0].lut[2].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[2].pwr_idx = 6;
ru_864_870.sx130x.txLut[0].lut[2].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[2].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[2].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[3].rf_power = 15;
ru_864_870.sx130x.txLut[0].lut[3].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[3].pwr_idx = 7;
ru_864_870.sx130x.txLut[0].lut[3].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[3].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[3].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[4].rf_power = 16;
ru_864_870.sx130x.txLut[0].lut[4].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[4].pwr_idx = 8;
ru_864_870.sx130x.txLut[0].lut[4].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[4].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[4].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[5].rf_power = 17;
ru_864_870.sx130x.txLut[0].lut[5].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[5].pwr_idx = 9;
ru_864_870.sx130x.txLut[0].lut[5].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[5].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[5].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[6].rf_power = 18;
ru_864_870.sx130x.txLut[0].lut[6].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[6].pwr_idx = 10;
ru_864_870.sx130x.txLut[0].lut[6].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[6].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[6].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[7].rf_power = 19;
ru_864_870.sx130x.txLut[0].lut[7].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[7].pwr_idx = 11;
ru_864_870.sx130x.txLut[0].lut[7].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[7].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[7].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[8].rf_power = 20;
ru_864_870.sx130x.txLut[0].lut[8].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[8].pwr_idx = 12;
ru_864_870.sx130x.txLut[0].lut[8].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[8].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[8].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[9].rf_power = 21;
ru_864_870.sx130x.txLut[0].lut[9].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[9].pwr_idx = 13;
ru_864_870.sx130x.txLut[0].lut[9].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[9].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[9].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[10].rf_power = 22;
ru_864_870.sx130x.txLut[0].lut[10].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[10].pwr_idx = 14;
ru_864_870.sx130x.txLut[0].lut[10].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[10].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[10].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[11].rf_power = 23;
ru_864_870.sx130x.txLut[0].lut[11].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[11].pwr_idx = 16;
ru_864_870.sx130x.txLut[0].lut[11].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[11].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[11].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[12].rf_power = 24;
ru_864_870.sx130x.txLut[0].lut[12].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[12].pwr_idx = 17;
ru_864_870.sx130x.txLut[0].lut[12].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[12].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[12].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[13].rf_power = 25;
ru_864_870.sx130x.txLut[0].lut[13].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[13].pwr_idx = 18;
ru_864_870.sx130x.txLut[0].lut[13].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[13].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[13].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[14].rf_power = 26;
ru_864_870.sx130x.txLut[0].lut[14].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[14].pwr_idx = 19;
ru_864_870.sx130x.txLut[0].lut[14].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[14].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[14].mix_gain = 5;
ru_864_870.sx130x.txLut[0].lut[15].rf_power = 27;
ru_864_870.sx130x.txLut[0].lut[15].pa_gain = 1;
ru_864_870.sx130x.txLut[0].lut[15].pwr_idx = 22;
ru_864_870.sx130x.txLut[0].lut[15].dig_gain = 0;
ru_864_870.sx130x.txLut[0].lut[15].dac_gain = 0;
ru_864_870.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

ru_864_870.sx130x.rfConfs[1].enable = true;
ru_864_870.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
ru_864_870.sx130x.rfConfs[1].freq_hz = 869000000;
ru_864_870.sx130x.rfConfs[1].rssi_offset = -215.4;
ru_864_870.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
ru_864_870.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
ru_864_870.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
ru_864_870.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
ru_864_870.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
ru_864_870.sx130x.rfConfs[1].tx_enable = false;
ru_864_870.sx130x.rfConfs[1].single_input_mode = false;
ru_864_870.sx130x.tx_freq_min[1] = 0;
ru_864_870.sx130x.tx_freq_max[1] = 0;
ru_864_870.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

ru_864_870.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
ru_864_870.sx130x.ifConfs[0].enable = true;
ru_864_870.sx130x.ifConfs[0].rf_chain = 0;
ru_864_870.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
ru_864_870.sx130x.ifConfs[1].enable = true;
ru_864_870.sx130x.ifConfs[1].rf_chain = 0;
ru_864_870.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
ru_864_870.sx130x.ifConfs[2].enable = true;
ru_864_870.sx130x.ifConfs[2].rf_chain = 0;
ru_864_870.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
ru_864_870.sx130x.ifConfs[3].enable = true;
ru_864_870.sx130x.ifConfs[3].rf_chain = 0;
ru_864_870.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
ru_864_870.sx130x.ifConfs[4].enable = true;
ru_864_870.sx130x.ifConfs[4].rf_chain = 0;
ru_864_870.sx130x.ifConfs[4].freq_hz = 400000;
// chan_multiSF_5
ru_864_870.sx130x.ifConfs[5].enable = true;
ru_864_870.sx130x.ifConfs[5].rf_chain = 1;
ru_864_870.sx130x.ifConfs[5].freq_hz = -100000;
// chan_multiSF_6
ru_864_870.sx130x.ifConfs[6].enable = true;
ru_864_870.sx130x.ifConfs[6].rf_chain = 1;
ru_864_870.sx130x.ifConfs[6].freq_hz = 100000;
// chan_multiSF_7
ru_864_870.sx130x.ifConfs[7].enable = false;
ru_864_870.sx130x.ifConfs[7].rf_chain = 1;
ru_864_870.sx130x.ifConfs[7].freq_hz = 300000;
// Lora std 
ru_864_870.sx130x.ifStdConf.enable = false;
ru_864_870.sx130x.ifStdConf.rf_chain = 0;
ru_864_870.sx130x.ifStdConf.freq_hz = 300000;
ru_864_870.sx130x.ifStdConf.bandwidth = 6;
ru_864_870.sx130x.ifStdConf.datarate = 8;
ru_864_870.sx130x.ifStdConf.implicit_hdr = false;
ru_864_870.sx130x.ifStdConf.implicit_payload_length = 17;
ru_864_870.sx130x.ifStdConf.implicit_crc_en = false;
ru_864_870.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
ru_864_870.sx130x.ifFSKConf.enable = false;
ru_864_870.sx130x.ifFSKConf.rf_chain = 1;
ru_864_870.sx130x.ifFSKConf.freq_hz = 300000;
ru_864_870.sx130x.ifFSKConf.bandwidth = 4;
ru_864_870.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

ru_864_870.gateway.gatewayId = 0xaa555a0000000000;
ru_864_870.gateway.serverPortUp = 1700;
ru_864_870.gateway.serverPortDown = 1700;
ru_864_870.gateway.keepaliveInterval = 10;
ru_864_870.gateway.statInterval = 30;
ru_864_870.gateway.pushTimeoutMs.tv_sec = 0;
ru_864_870.gateway.pushTimeoutMs.tv_usec = 50000;
ru_864_870.gateway.forwardCRCValid = true;
ru_864_870.gateway.forwardCRCError = false;
ru_864_870.gateway.forwardCRCDisabled = false;
ru_864_870.gateway.refGeoCoordinates.lat = 0;
ru_864_870.gateway.refGeoCoordinates.lon = 0;
ru_864_870.gateway.refGeoCoordinates.alt = 0;
ru_864_870.gateway.fakeGPS = false;
ru_864_870.gateway.beaconPeriod = 0;
ru_864_870.gateway.beaconFreqHz = 869100000;
ru_864_870.gateway.beaconFreqNb = 1;
ru_864_870.gateway.beaconFreqStep = 0;
ru_864_870.gateway.beaconDataRate = 9;
ru_864_870.gateway.beaconBandwidthHz = 125000;
ru_864_870.gateway.beaconInfoDesc = 0;
ru_864_870.gateway.autoQuitThreshold = 0;
ru_864_870.serverAddr = "eu1.cloud.thethings.network";
ru_864_870.gpsTtyPath = "/dev/ttyAMA0";
ru_864_870.name = "ru 864 870";

// Debug nb_ref_payload, count: 2

ru_864_870.debug.nb_ref_payload = 2;
ru_864_870.debug.ref_payload[0].id = 0xcafe1234;
ru_864_870.debug.ref_payload[1].id = 0xcafe2345;
strcpy(ru_864_870.debug.log_file_name, "loragw_hal.log");

};   // ru_864_870

void setup_us_902_928(MemGatewaySettingsStorage &us_902_928) {

// SX1261 

strcpy(us_902_928.sx1261.sx1261.spi_path, "");
us_902_928.sx1261.sx1261.rssi_offset = 0;
us_902_928.sx1261.spectralScan.enable = false;
us_902_928.sx1261.spectralScan.freq_hz_start = 0;
us_902_928.sx1261.spectralScan.nb_chan = 0;
us_902_928.sx1261.spectralScan.nb_scan = (int) 0;
us_902_928.sx1261.spectralScan.pace_s = 0;
us_902_928.sx1261.lbt.enable = false;
us_902_928.sx1261.lbt.nb_channel = 0;

// SX130x 

us_902_928.sx130x.boardConf.com_type = (lgw_com_type_t) 1;
strcpy(us_902_928.sx130x.boardConf.com_path, "/dev/ttyACM0");
us_902_928.sx130x.boardConf.lorawan_public = true;
us_902_928.sx130x.boardConf.clksrc = 0;
us_902_928.sx130x.antennaGain = 0;
us_902_928.sx130x.boardConf.full_duplex = false;
us_902_928.sx130x.tsConf.enable = false;
us_902_928.sx130x.tsConf.mode = (lgw_ftime_mode_t) 0;

// Radio 0

us_902_928.sx130x.rfConfs[0].enable = true;
us_902_928.sx130x.rfConfs[0].type = (lgw_radio_type_t) 5;
us_902_928.sx130x.rfConfs[0].freq_hz = 904300000;
us_902_928.sx130x.rfConfs[0].rssi_offset = -215.4;
us_902_928.sx130x.rfConfs[0].rssi_tcomp.coeff_a = 0;
us_902_928.sx130x.rfConfs[0].rssi_tcomp.coeff_b = 0;
us_902_928.sx130x.rfConfs[0].rssi_tcomp.coeff_c = 20.41;
us_902_928.sx130x.rfConfs[0].rssi_tcomp.coeff_d = 2162.56;
us_902_928.sx130x.rfConfs[0].rssi_tcomp.coeff_e = 0;
us_902_928.sx130x.rfConfs[0].tx_enable = true;
us_902_928.sx130x.rfConfs[0].single_input_mode = false;
us_902_928.sx130x.tx_freq_min[0] = 923000000;
us_902_928.sx130x.tx_freq_max[0] = 928000000;
us_902_928.sx130x.txLut[0].size = 16;

// TX LUT radio 0, count: 16

us_902_928.sx130x.txLut[0].lut[0].rf_power = 12;
us_902_928.sx130x.txLut[0].lut[0].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[0].pwr_idx = 6;
us_902_928.sx130x.txLut[0].lut[0].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[0].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[0].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[1].rf_power = 13;
us_902_928.sx130x.txLut[0].lut[1].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[1].pwr_idx = 7;
us_902_928.sx130x.txLut[0].lut[1].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[1].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[1].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[2].rf_power = 14;
us_902_928.sx130x.txLut[0].lut[2].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[2].pwr_idx = 8;
us_902_928.sx130x.txLut[0].lut[2].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[2].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[2].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[3].rf_power = 15;
us_902_928.sx130x.txLut[0].lut[3].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[3].pwr_idx = 9;
us_902_928.sx130x.txLut[0].lut[3].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[3].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[3].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[4].rf_power = 16;
us_902_928.sx130x.txLut[0].lut[4].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[4].pwr_idx = 10;
us_902_928.sx130x.txLut[0].lut[4].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[4].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[4].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[5].rf_power = 17;
us_902_928.sx130x.txLut[0].lut[5].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[5].pwr_idx = 11;
us_902_928.sx130x.txLut[0].lut[5].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[5].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[5].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[6].rf_power = 18;
us_902_928.sx130x.txLut[0].lut[6].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[6].pwr_idx = 12;
us_902_928.sx130x.txLut[0].lut[6].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[6].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[6].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[7].rf_power = 19;
us_902_928.sx130x.txLut[0].lut[7].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[7].pwr_idx = 13;
us_902_928.sx130x.txLut[0].lut[7].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[7].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[7].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[8].rf_power = 20;
us_902_928.sx130x.txLut[0].lut[8].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[8].pwr_idx = 14;
us_902_928.sx130x.txLut[0].lut[8].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[8].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[8].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[9].rf_power = 21;
us_902_928.sx130x.txLut[0].lut[9].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[9].pwr_idx = 15;
us_902_928.sx130x.txLut[0].lut[9].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[9].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[9].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[10].rf_power = 22;
us_902_928.sx130x.txLut[0].lut[10].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[10].pwr_idx = 16;
us_902_928.sx130x.txLut[0].lut[10].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[10].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[10].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[11].rf_power = 23;
us_902_928.sx130x.txLut[0].lut[11].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[11].pwr_idx = 17;
us_902_928.sx130x.txLut[0].lut[11].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[11].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[11].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[12].rf_power = 24;
us_902_928.sx130x.txLut[0].lut[12].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[12].pwr_idx = 18;
us_902_928.sx130x.txLut[0].lut[12].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[12].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[12].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[13].rf_power = 25;
us_902_928.sx130x.txLut[0].lut[13].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[13].pwr_idx = 19;
us_902_928.sx130x.txLut[0].lut[13].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[13].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[13].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[14].rf_power = 26;
us_902_928.sx130x.txLut[0].lut[14].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[14].pwr_idx = 21;
us_902_928.sx130x.txLut[0].lut[14].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[14].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[14].mix_gain = 5;
us_902_928.sx130x.txLut[0].lut[15].rf_power = 27;
us_902_928.sx130x.txLut[0].lut[15].pa_gain = 1;
us_902_928.sx130x.txLut[0].lut[15].pwr_idx = 22;
us_902_928.sx130x.txLut[0].lut[15].dig_gain = 0;
us_902_928.sx130x.txLut[0].lut[15].dac_gain = 0;
us_902_928.sx130x.txLut[0].lut[15].mix_gain = 5;

// Radio 1

us_902_928.sx130x.rfConfs[1].enable = true;
us_902_928.sx130x.rfConfs[1].type = (lgw_radio_type_t) 5;
us_902_928.sx130x.rfConfs[1].freq_hz = 905000000;
us_902_928.sx130x.rfConfs[1].rssi_offset = -215.4;
us_902_928.sx130x.rfConfs[1].rssi_tcomp.coeff_a = 0;
us_902_928.sx130x.rfConfs[1].rssi_tcomp.coeff_b = 0;
us_902_928.sx130x.rfConfs[1].rssi_tcomp.coeff_c = 20.41;
us_902_928.sx130x.rfConfs[1].rssi_tcomp.coeff_d = 2162.56;
us_902_928.sx130x.rfConfs[1].rssi_tcomp.coeff_e = 0;
us_902_928.sx130x.rfConfs[1].tx_enable = false;
us_902_928.sx130x.rfConfs[1].single_input_mode = false;
us_902_928.sx130x.tx_freq_min[1] = 0;
us_902_928.sx130x.tx_freq_max[1] = 0;
us_902_928.sx130x.txLut[1].size = 0;

// TX LUT radio 1, count: 0


// chan_multiSF_All spreading_factor_enable bit field

us_902_928.sx130x.demodConf.multisf_datarate = 0xff;	 // 255
// chan_multiSF_0
us_902_928.sx130x.ifConfs[0].enable = true;
us_902_928.sx130x.ifConfs[0].rf_chain = 0;
us_902_928.sx130x.ifConfs[0].freq_hz = -400000;
// chan_multiSF_1
us_902_928.sx130x.ifConfs[1].enable = true;
us_902_928.sx130x.ifConfs[1].rf_chain = 0;
us_902_928.sx130x.ifConfs[1].freq_hz = -200000;
// chan_multiSF_2
us_902_928.sx130x.ifConfs[2].enable = true;
us_902_928.sx130x.ifConfs[2].rf_chain = 0;
us_902_928.sx130x.ifConfs[2].freq_hz = 0;
// chan_multiSF_3
us_902_928.sx130x.ifConfs[3].enable = true;
us_902_928.sx130x.ifConfs[3].rf_chain = 0;
us_902_928.sx130x.ifConfs[3].freq_hz = 200000;
// chan_multiSF_4
us_902_928.sx130x.ifConfs[4].enable = true;
us_902_928.sx130x.ifConfs[4].rf_chain = 1;
us_902_928.sx130x.ifConfs[4].freq_hz = -300000;
// chan_multiSF_5
us_902_928.sx130x.ifConfs[5].enable = true;
us_902_928.sx130x.ifConfs[5].rf_chain = 1;
us_902_928.sx130x.ifConfs[5].freq_hz = -100000;
// chan_multiSF_6
us_902_928.sx130x.ifConfs[6].enable = true;
us_902_928.sx130x.ifConfs[6].rf_chain = 1;
us_902_928.sx130x.ifConfs[6].freq_hz = 100000;
// chan_multiSF_7
us_902_928.sx130x.ifConfs[7].enable = true;
us_902_928.sx130x.ifConfs[7].rf_chain = 1;
us_902_928.sx130x.ifConfs[7].freq_hz = 300000;
// Lora std 
us_902_928.sx130x.ifStdConf.enable = true;
us_902_928.sx130x.ifStdConf.rf_chain = 0;
us_902_928.sx130x.ifStdConf.freq_hz = 300000;
us_902_928.sx130x.ifStdConf.bandwidth = 6;
us_902_928.sx130x.ifStdConf.datarate = 8;
us_902_928.sx130x.ifStdConf.implicit_hdr = false;
us_902_928.sx130x.ifStdConf.implicit_payload_length = 17;
us_902_928.sx130x.ifStdConf.implicit_crc_en = false;
us_902_928.sx130x.ifStdConf.implicit_coderate = 1;
// FSK 
us_902_928.sx130x.ifFSKConf.enable = false;
us_902_928.sx130x.ifFSKConf.rf_chain = 1;
us_902_928.sx130x.ifFSKConf.freq_hz = 300000;
us_902_928.sx130x.ifFSKConf.bandwidth = 4;
us_902_928.sx130x.ifFSKConf.datarate = 50000;

// Gateway 

us_902_928.gateway.gatewayId = 0xaa555a0000000000;
us_902_928.gateway.serverPortUp = 1700;
us_902_928.gateway.serverPortDown = 1700;
us_902_928.gateway.keepaliveInterval = 10;
us_902_928.gateway.statInterval = 30;
us_902_928.gateway.pushTimeoutMs.tv_sec = 0;
us_902_928.gateway.pushTimeoutMs.tv_usec = 50000;
us_902_928.gateway.forwardCRCValid = true;
us_902_928.gateway.forwardCRCError = false;
us_902_928.gateway.forwardCRCDisabled = false;
us_902_928.gateway.refGeoCoordinates.lat = 0;
us_902_928.gateway.refGeoCoordinates.lon = 0;
us_902_928.gateway.refGeoCoordinates.alt = 0;
us_902_928.gateway.fakeGPS = false;
us_902_928.gateway.beaconPeriod = 0;
us_902_928.gateway.beaconFreqHz = 923300000;
us_902_928.gateway.beaconFreqNb = 8;
us_902_928.gateway.beaconFreqStep = 600000;
us_902_928.gateway.beaconDataRate = 12;
us_902_928.gateway.beaconBandwidthHz = 500000;
us_902_928.gateway.beaconInfoDesc = 0;
us_902_928.gateway.autoQuitThreshold = 0;
us_902_928.serverAddr = "nam1.cloud.thethings.network";
us_902_928.gpsTtyPath = "/dev/ttyAMA0";
us_902_928.name = "us 902 928";

// Debug nb_ref_payload, count: 2

us_902_928.debug.nb_ref_payload = 2;
us_902_928.debug.ref_payload[0].id = 0xcafe1234;
us_902_928.debug.ref_payload[1].id = 0xcafe2345;
strcpy(us_902_928.debug.log_file_name, "loragw_hal.log");

};   // us_902_928

const setupMemGatewaySettingsStorage memSetupMemGatewaySettingsStorage[] = {
	{"as 915 921", &setup_as_915_921},
	{"as 915 928", &setup_as_915_928},
	{"as 917 920", &setup_as_917_920},
	{"as 920 923", &setup_as_920_923},
	{"au 915 928", &setup_au_915_928},
	{"cn 470 510", &setup_cn_470_510},
	{"eu 433", &setup_eu_433},
	{"eu 863 870", &setup_eu_863_870},
	{"in 865 867", &setup_in_865_867},
	{"kr 920 923", &setup_kr_920_923},
	{"ru 864 870", &setup_ru_864_870},
	{"us 902 928", &setup_us_902_928}
};
