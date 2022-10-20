#include <iostream>
#include <iomanip>

#include "lora-gateway-listener.h"
#include "utilstring.h"
#include "errlist.h"

#define ERR_CODE_LORA_GATEWAY_BASE -800
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED       ERR_CODE_LORA_GATEWAY_BASE - 1
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_TIME_STAMP         ERR_CODE_LORA_GATEWAY_BASE - 2
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_SX1261_RADIO       ERR_CODE_LORA_GATEWAY_BASE - 3
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT        ERR_CODE_LORA_GATEWAY_BASE - 4
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_INVALID_RADIO      ERR_CODE_LORA_GATEWAY_BASE - 5
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_DEMODULATION       ERR_CODE_LORA_GATEWAY_BASE - 6
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL   ERR_CODE_LORA_GATEWAY_BASE - 7
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_STD_CHANNEL        ERR_CODE_LORA_GATEWAY_BASE - 9
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL        ERR_CODE_LORA_GATEWAY_BASE - 10

static const std::string ERR_MSG_LORA_GATEWAY[] = {
    "Failed to configure board",
    "Failed to configure fine timestamp",
    "Failed to configure the SX1261 radio",
    "Failed to configure concentrator TX Gain LUT",
    "Invalid configuration for radio",
    "Invalid configuration for demodulation parameters",
    "Invalid configuration for Lora multi-SF channel"
};

LoraGatewayListener::LoraGatewayListener()
    : last_lgw_retcode(0), lastConfig(nullptr)
{
}

LoraGatewayListener::~LoraGatewayListener()
{
}

int LoraGatewayListener::setup(GatewayConfigFileJson *config)
{
    last_lgw_retcode = 0;
    lastConfig = config;
    if (!config)
        return ERR_CODE_INSUFFICIENT_PARAMS;
    last_lgw_retcode = lgw_board_setconf(&config->sx130xConf.boardConf);
    if (last_lgw_retcode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED;
    if (config->sx130xConf.tsConf.enable) {
        last_lgw_retcode = lgw_ftime_setconf(&config->sx130xConf.tsConf);
        if (last_lgw_retcode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_TIME_STAMP;
    }
    last_lgw_retcode = lgw_sx1261_setconf(&config->sx130xConf.sx1261Config.value);
    if (last_lgw_retcode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_SX1261_RADIO;

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        last_lgw_retcode = lgw_txgain_setconf(i, &config->sx130xConf.txLut[i]);
        if (last_lgw_retcode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT;

    }

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        last_lgw_retcode = lgw_rxrf_setconf(i, &config->sx130xConf.rfConfs[i]);
        if (last_lgw_retcode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_INVALID_RADIO;
    }
    last_lgw_retcode = lgw_demod_setconf(&config->sx130xConf.demodConf);
    if (last_lgw_retcode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEMODULATION;

    for (int i = 0; i < LGW_MULTI_NB; i++) {
        last_lgw_retcode = lgw_rxif_setconf(i, &config->sx130xConf.ifConfs[i]);
        if (last_lgw_retcode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL;
    }
    if (config->sx130xConf.ifStdConf.enable) {
        last_lgw_retcode = lgw_rxif_setconf(8, &config->sx130xConf.ifStdConf);
        if (last_lgw_retcode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_STD_CHANNEL;
    } else; // TODO
    if (config->sx130xConf.ifStdConf.enable) {
        last_lgw_retcode = lgw_rxif_setconf(9, &config->sx130xConf.ifFSKConf);
        if (last_lgw_retcode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL;
    } else; // TODO
}

std::string LoraGatewayListener::version()
{
    return std::string(lgw_version_info());
}

/*
    lgw_gps_enable(gps_tty_path, "ubx7", 0, &gps_tty_fd);
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

}

int LoraGatewayListener::stop()
{

}
