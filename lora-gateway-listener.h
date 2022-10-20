#ifndef LORA_GATEWAY_LISTENER_H_
#define LORA_GATEWAY_LISTENER_H_ 1

#include <vector>
#include "gateway-file-json.h"

class LoraGatewayListener {
public:
    GatewayConfigFileJson *lastConfig;

    LoraGatewayListener();
    ~LoraGatewayListener();

    /*
        lgw_board_setconf(&boardconf);
        lgw_ftime_setconf(&tsconf);
        lgw_sx1261_setconf(&sx1261conf);
        lgw_txgain_setconf(i, &txlut[i]);
        lgw_rxrf_setconf(i, &rfconf);
        lgw_demod_setconf(&demodconf);
        lgw_rxif_setconf(i, &ifconf);
        lgw_rxif_setconf(8, &ifconf);
        lgw_rxif_setconf(9, &ifconf);
        lgw_debug_setconf(&debugconf);
    */
    int setup(GatewayConfigFileJson *config);
    /*
        lgw_version_info();
    */
    std::string version();
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
    int start();
    int stop();
};

#endif
