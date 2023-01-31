#ifndef LORA_GATEWAY_LISTENER_H_
#define LORA_GATEWAY_LISTENER_H_ 1

#include <vector>
#include <mutex>
#include <functional>

#include "utillora.h"
#include "gateway-settings.h"
#include "packet-listener.h"
#include "log-intf.h"

#define MEASUREMENT_COUNT_SIZE 23

// listen() flags parameter values
// 1- Do not send messages
#define FLAG_GATEWAY_LISTENER_NO_SEND   1
// 2- Do not send beacons
#define FLAG_GATEWAY_LISTENER_NO_BEACON 2

typedef enum {
    meas_nb_rx_rcv,             ///< count packets received
    meas_nb_rx_ok,              ///< count packets received with PAYLOAD CRC OK
    meas_nb_rx_bad,             ///< count packets received with PAYLOAD CRC ERROR
    meas_nb_rx_nocrc,           ///< count packets received with NO PAYLOAD CRC
    meas_up_pkt_fwd,            ///< number of radio packet forwarded to the server

    meas_up_network_byte,       ///< sum of UDP bytes sent for upstream traffic
    meas_up_payload_byte,       ///< sum of radio payload bytes sent for upstream traffic
    meas_up_dgram_sent,         ///< number of datagrams sent for upstream traffic
    meas_up_ack_rcv,            ///< number of datagrams acknowledged for upstream traffic
    meas_dw_pull_sent,          ///< number of PULL requests sent for downstream traffic

    meas_dw_dgram_rcv,          ///< count PULL response packets received for downstream traffic
    meas_dw_network_byte,       ///< sum of UDP bytes sent for upstream traffic
    meas_dw_payload_byte,       ///< sum of radio payload bytes sent for upstream traffic
    meas_nb_tx_ok,              ///< count packets emitted successfully
    meas_nb_tx_fail,            ///< count packets were TX failed for other reasons

    meas_nb_tx_requested,       ///< count TX request from server (downlinks)
    meas_nb_tx_rejected_collision_packet,  ///< count packets were TX request were rejected due to collision with another packet already programmed
    meas_nb_tx_rejected_collision_beacon,  ///< count packets were TX request were rejected due to collision with a beacon already programmed
    meas_nb_tx_rejected_too_late,          ///< count packets were TX request were rejected because it is too late to program it
    meas_nb_tx_rejected_too_early,         ///< count packets were TX request were rejected because timestamp is too much in advance

    meas_nb_beacon_queued,                 ///< count beacon inserted in jit queue
    meas_nb_beacon_sent,                   ///< count beacon actually sent to concentrator
    meas_nb_beacon_rejected                ///< count beacon rejected for queuing
} MEASUREMENT_ENUM;

const char *getMeasurementName(int index);

// measurements to establish statistics
class GatewayMeasurements {
private:
    uint32_t value[MEASUREMENT_COUNT_SIZE];
    mutable std::mutex mAccess;
public:
    void reset();
    GatewayMeasurements();
    const uint32_t get(MEASUREMENT_ENUM index) const;
    void set(MEASUREMENT_ENUM index, uint32_t v);
    // increment
    void inc(MEASUREMENT_ENUM index);
    void inc(MEASUREMENT_ENUM index, uint32_t v);
    void get(uint32_t retval[MEASUREMENT_COUNT_SIZE]) const;
    std::string toString() const;
};

class LGWStatus {
public:
    uint32_t inst_tstamp;     ///< SX1302 counter (INST)
    uint32_t trig_tstamp;     ///< SX1302 counter (PPS)
    float temperature;        ///< Concentrator temperature
};

class TxPacket {
public:
    struct lgw_pkt_tx_s pkt;
    TxPacket();
};

class LoraGatewayListener {
private:
    int logVerbosity;

    std::function<void(
        const LoraGatewayListener *listener,
        const SEMTECH_PROTOCOL_METADATA *metadata,
        const std::string &payload
    )> onUpstream;

    std::function<void(
        const LoraGatewayListener *listener,
        const uint32_t frequency,
        const uint16_t results[LGW_SPECTRAL_SCAN_RESULT_SIZE]
    )> onSpectralScan;

    std::function<void(
        const LoraGatewayListener *listener,
        bool gracefullyStopped
    )> onStop;

    // thread control
    bool stopRequest;               ///< set to true to stop all threads
    bool gps_ref_valid;             ///< is GPS reference acceptable (ie. not too old)
    // control access
    std::mutex mutexGPSTimeReference;        ///< control access to set system time
    std::mutex mLGW;                         ///< control access to the concentrator
    std::mutex mReportSpectralScan;          ///< control access to spectral scan report
    mutable std::mutex mLog;                 ///< control access to log facility
    std::mutex mXTALcorrection;              ///< control access to the XTAL correction

    struct jit_queue_s jit_queue[LGW_RF_CHAIN_NB];  ///< Just In Time TX scheduling for each radio channel
    struct tref gpsTimeReference;    ///< time reference used for GPS <-> timestamp conversion
    bool xtal_correct_ok;            ///< set true when XTAL correction is stable enough
    double xtal_correct;             ///< XTAL frequency correction coefficient. XTAL(crystal) in timing refers to a quartz crystal.

    int setSystemTime(const uint32_t ppmCountUS);
    bool gpsEnabled;
    time_t gpsTimeLastSynced;
    time_t gpsCoordsLastSynced;
    struct coord_s gpsLastCoord;

    int syncGPSTime();
    int syncGPSLocation();

    // threads
    void upstreamRunner();      // receive Lora packets from end-device(s)
    void downstreamBeaconRunner();    // transmit beacons
    void jitRunner();           // transmit from JIT queue
    void spectralScanRunner();
    void gpsRunner();
    void gpsCheckTimeRunner();
    bool getTxGainLutIndex(uint8_t rf_chain, int8_t rf_power, uint8_t * lut_index);
protected:
    // Apply config
    int setup();
public:
    // thread finish indicators
    bool upstreamThreadRunning;
    bool downstreamBeaconThreadRunning;
    bool jitThreadRunning;
    bool gpsThreadRunning;
    bool gpsCheckTimeThreadRunning;
    bool spectralScanThreadRunning;

    PacketListener *packetListener;
    int lastLgwCode;
    GatewaySettings *config;
    int flags;
    GatewayMeasurements measurements;

    int fdGpsTty;        ///< file descriptor of the GPS TTY port
    uint64_t eui;        ///< Gateway EUI

    LoraGatewayListener();
    LoraGatewayListener(GatewaySettings *cfg);
    ~LoraGatewayListener();

    void log(
        int level,
        int errorCode,
        const std::string &message
    ) const;

    /**
        LGW library version.
        Calls lgw_version_info();
    */
    std::string version();
    // SX1302 Status
    bool getStatus(LGWStatus &status);
    int start();
    int stop(int waitSeconds);
    bool isRunning() const;
    bool isStopped() const;

    void setOnSpectralScan(
        std::function<void(
            const LoraGatewayListener *listener,
            const uint32_t frequency,
            const uint16_t results[LGW_SPECTRAL_SCAN_RESULT_SIZE]
        )> value
    );
    void setOnLog(LogIntf *value);
    void setOnUpstream(
        std::function<void(
            const LoraGatewayListener *listener,
            const SEMTECH_PROTOCOL_METADATA *metadata,
            const std::string &payload
        )> value
    );
    void setOnStop(
        std::function<void(
            const LoraGatewayListener *listener,
            bool gracefullyStopped
        )> value
    );
    void setLogVerbosity(int level);
    int enqueueTxPacket(TxPacket &tx);

    std::string toString() const;

    LogIntf *onLog;
};

#endif
