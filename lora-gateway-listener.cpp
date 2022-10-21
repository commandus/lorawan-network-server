#include <iostream>
#include <iomanip>

#include "lora-gateway-listener.h"
#include "utilstring.h"
#include "errlist.h"

LoraGatewayListener::LoraGatewayListener()
    : lastLgwCode(0), config(nullptr), fdGpsTty(-1), eui(0)
{
}

LoraGatewayListener::~LoraGatewayListener()
{
}

int LoraGatewayListener::setup(GatewayConfigFileJson *config)
{
    lastLgwCode = 0;
    config = config;
    if (!config)
        return ERR_CODE_INSUFFICIENT_PARAMS;
    lastLgwCode = lgw_board_setconf(&config->sx130xConf.boardConf);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED;
    if (config->sx130xConf.tsConf.enable) {
        lastLgwCode = lgw_ftime_setconf(&config->sx130xConf.tsConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_TIME_STAMP;
    }
    lastLgwCode = lgw_sx1261_setconf(&config->sx130xConf.sx1261Config.value);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_SX1261_RADIO;

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        lastLgwCode = lgw_txgain_setconf(i, &config->sx130xConf.txLut[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT;

    }

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        lastLgwCode = lgw_rxrf_setconf(i, &config->sx130xConf.rfConfs[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_INVALID_RADIO;
    }
    lastLgwCode = lgw_demod_setconf(&config->sx130xConf.demodConf);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEMODULATION;

    for (int i = 0; i < LGW_MULTI_NB; i++) {
        lastLgwCode = lgw_rxif_setconf(i, &config->sx130xConf.ifConfs[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL;
    }
    if (config->sx130xConf.ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(8, &config->sx130xConf.ifStdConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_STD_CHANNEL;
    } else; // TODO
    if (config->sx130xConf.ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(9, &config->sx130xConf.ifFSKConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL;
    } else; // TODO
    lastLgwCode = lgw_debug_setconf(&config->debugConf.value);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEBUG;
}

std::string LoraGatewayListener::version()
{
    return std::string(lgw_version_info());
}

bool LoraGatewayListener::getStatus(LGWStatus &status)
{
    lastLgwCode = lgw_get_instcnt(&status.inst_tstamp);
    lastLgwCode = lgw_get_trigcnt(&status.trig_tstamp);
    lastLgwCode = lgw_get_temperature(&temperature);
    return lastLgwCode == 0;
}

/*
    lgw_start();
    lgw_get_eui(&eui);
    lgw_get_instcnt(&inst_tstamp);
    lgw_get_trigcnt(&trig_tstamp);
    lgw_get_temperature(&temperature);
    lgw_gps_disable(gps_tty_fd);
    lgw_stop();
    nb_pkt = lgw_receive(NB_PKT_MAX, rxpkt);
    lgw_cnt2utc(local_ref, p->count_us, &pkt_utc_time);
    lgw_cnt2gps(local_ref, p->count_us, &pkt_gps_time);
    lgw_gps2cnt(time_reference_gps, next_beacon_gps_time, &(beacon_pkt.count_us));
    lgw_get_instcnt(&current_concentrator_time);
    lgw_gps2cnt(local_ref, gps_tx, &(txpkt.count_us));
    lgw_get_instcnt(&current_concentrator_time);
    lgw_get_instcnt(&current_concentrator_time);
    lgw_status(pkt.rf_chain, TX_STATUS, &tx_status);
    lgw_spectral_scan_abort();
    lgw_send(&pkt);
    lgw_cnt2utc(time_reference_gps, ppm_tstamp, &y);
    lgw_gps_get(&utc, &gps_time, NULL, NULL);
    lgw_get_trigcnt(&trig_tstamp);
    lgw_gps_sync(&time_reference_gps, trig_tstamp, utc, gps_time);
    lgw_gps_get(NULL, NULL, &coord, &gpserr);
    lgw_parse_ubx(&serial_buff[rd_idx], (wr_idx - rd_idx), &frame_size);
    lgw_parse_nmea(&serial_buff[rd_idx], frame_size);
    lgw_status((uint8_t)i, TX_STATUS, &tx_status);
    lgw_spectral_scan_start(freq_hz, spectral_scan_params.nb_scan);
    lgw_spectral_scan_get_status(&status);
    lgw_spectral_scan_get_results(levels, results);
*/
int LoraGatewayListener::start()
{
    if (!config)
        return ERR_CODE_NO_CONFIG;

    lastLgwCode = 0;
    // GPS sync
    if (!config->gatewayConf.gpsTTYPath.empty()) {
        lastLgwCode = lgw_gps_enable((char *) config->gatewayConf.gpsTTYPath.c_str(), "ubx7", 0, &fdGpsTty);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED;
    }
    // starting the concentrator
    lastLgwCode = lgw_start();
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_START_FAILED;
    // get the concentrator EUI
    lastLgwCode = lgw_get_eui(&eui);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_GET_EUI;

}

int LoraGatewayListener::stop()
{

}
