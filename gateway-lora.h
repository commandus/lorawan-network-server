#ifndef GATEWAY_LORA
#define GATEWAY_LORA 1

/*
    Semtech's Lora concentrator configuration structures
    @see gateway-settings.h
*/
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#ifdef __GNUC__
#define restrict __restrict__ // G++ has restrict
#else
#define restrict // C++ in general doesn't
#endif

#define _XOPEN_SOURCE 700

#endif

#include "packet_forwarder/loragw_hal.h"
#include "packet_forwarder/loragw_aux.h"
#include "packet_forwarder/loragw_reg.h"
#include "packet_forwarder/loragw_gps.h"

#include "packet_forwarder/jitqueue.h"


/**
 * spectral scan
 */
typedef struct spectral_scan_s {
    bool enable;            ///< enable spectral scan thread
    uint32_t freq_hz_start; ///< first channel frequency, in Hz
    uint8_t nb_chan;        ///< number of channels to scan (200kHz between each channel)
    uint16_t nb_scan;       ///< number of scan points for each frequency scan
    uint32_t pace_s;        ///< number of seconds between 2 scans in the thread
} spectral_scan_t;

typedef struct sx1261_config_s {
    struct lgw_conf_sx1261_s sx1261;
    spectral_scan_t spectralScan;
    struct lgw_conf_lbt_s lbt;
} sx1261_config_t;

typedef struct sx130x_config_s {
    struct lgw_conf_board_s boardConf;
    int8_t antennaGain;
    struct lgw_conf_ftime_s tsConf;
    struct lgw_conf_rxrf_s rfConfs[LGW_RF_CHAIN_NB];

    uint32_t tx_freq_min[LGW_RF_CHAIN_NB];
    uint32_t tx_freq_max[LGW_RF_CHAIN_NB];
    struct lgw_tx_gain_lut_s txLut[LGW_RF_CHAIN_NB];
    struct lgw_conf_rxif_s ifConfs[LGW_MULTI_NB];
    struct lgw_conf_rxif_s ifStdConf;
    struct lgw_conf_rxif_s ifFSKConf;
    struct lgw_conf_demod_s demodConf;
} sx130x_config_t;

typedef struct gateway_s {
    uint64_t gatewayId;
    uint16_t serverPortUp;      // not used
    uint16_t serverPortDown;    // not used
    uint32_t keepaliveInterval;
    uint32_t statInterval;
    struct timeval pushTimeoutMs;
    bool forwardCRCValid;
    bool forwardCRCError;
    bool forwardCRCDisabled;
    bool gpsEnabled;
    struct coord_s refGeoCoordinates;
    bool fakeGPS;
    uint32_t beaconPeriod;  ///< send class B beacon period in seconds. 0- do not send beacons, Must be >=6s and <86400
    uint32_t beaconFreqHz;
    uint8_t beaconFreqNb;
    uint32_t beaconFreqStep;
    uint8_t beaconDataRate;
    uint32_t beaconBandwidthHz;
    uint8_t beaconPower;
    uint8_t beaconInfoDesc;
    uint32_t autoQuitThreshold;
} gateway_t;

typedef struct gateway_settings_s {
    sx1261_config_t sx1261;
    sx130x_config_t sx130x;
    gateway_t gateway;
    struct lgw_conf_debug_s debug;
} gateway_settings_t;

typedef struct gateway_settings_ptr_s {
    sx1261_config_t *sx1261;
    sx130x_config_t *sx130x;
    gateway_t *gateway;
    struct lgw_conf_debug_s *debug;
} gateway_settings_ptr_t;

#ifdef __cplusplus
}
#endif

#endif
