/*
 * C++ wrapper of lora_pkt_fwd.c (C)2019 Semtech License: Revised BSD License, see LICENSE.TXT file include in the project
 */
#include <iomanip>
#include <thread>
#include <cmath>

#include "lora-gateway-listener.h"
#include "utilstring.h"
#include "utilthread.h"
#include "errlist.h"

// XTAL correction constants
#define GPS_REF_MAX_AGE     30          // maximum admitted delay in seconds of GPS loss before considering latest GPS sync unusable
#define XERR_INIT_AVG       16          // number of measurements the XTAL correction is averaged on as initial value
#define XERR_FILT_COEF      256         // coefficient for low-pass XTAL error tracking

const char *MEASUREMENT_NAME[MEASUREMENT_COUNT_SIZE] = {
    "Received",
    "CRC OK",
    "CRC ERROR",
    "No CRC",
    "Forwarded",
    "Bytes down",
    "Radio payload down",
    "Datagrams",
    "ACKed",
    "PULL requests",
    "PULL responses",
    "Bytes up",
    "Radio payload up",
    "TX emitted",
    "TX failed",
    "TX requests",
    "TX packet collision",
    "TX beacon collision",
    "TX too late",
    "TX too early",
    "Beacon queued",
    "Beacon sent",
    "Beacon rejected"
};

const char *MEASUREMENT_SHORT_NAME[MEASUREMENT_COUNT_SIZE] = {
    "recv",
    "crcOk",
    "crcErr",
    "noCRC",
    "fwd",
    "down",
    "radPd",
    "dgrams",
    "ack",
    "pulreq",
    "pulres",
    "byteUp",
    "radPUp",
    "txEmit",
    "txFail",
    "txReq",
    "txpCol",
    "txbCol",
    "tx2lat",
    "tx2ear",
    "becQue",
    "becSen",
    "becRej"
};

GatewayMeasurements::GatewayMeasurements()
{
    reset();
}

// measurements to establish statistics
const uint32_t GatewayMeasurements::get(MEASUREMENT_ENUM index) const
{
    mAccess.lock();
    uint32_t r = value[index];
    mAccess.unlock();
    return r;
}

void GatewayMeasurements::reset()
{
    mAccess.lock();
    memset(value, 0, sizeof(value));
    mAccess.unlock();
}

void GatewayMeasurements::set(
    MEASUREMENT_ENUM index,
    uint32_t v
)
{
    mAccess.lock();
    value[index] = v;
    mAccess.unlock();
}

void GatewayMeasurements::inc(
    MEASUREMENT_ENUM index
)
{
    mAccess.lock();
    value[index]++;
    mAccess.unlock();
}

void GatewayMeasurements::inc(
    MEASUREMENT_ENUM index,
    uint32_t v
)
{
    mAccess.lock();
    value[index] += v;
    mAccess.unlock();
}

void GatewayMeasurements::get(
    uint32_t retval[MEASUREMENT_COUNT_SIZE]
) const
{
    mAccess.lock();
    memcpy(retval, value, sizeof(value));
    mAccess.unlock();
}

std::string GatewayMeasurements::toString() const
{
    uint32_t m[MEASUREMENT_COUNT_SIZE];
    get(m);
    std::stringstream ss;
    ss << "{";
    bool isFirst = true;
    for (int i = 0; i < MEASUREMENT_COUNT_SIZE; i++) {
        if (isFirst)
            isFirst = false;
        else
            ss << ", ";
        ss << "\"" << getMeasurementName(i) << "\": " << m[i];
    }
    ss << "}";
    return ss.str();
}

TxPacket::TxPacket()
{
    memset(&pkt, 0, sizeof(struct lgw_pkt_tx_s));
}

static const char *DEF_GPS_FAMILY = "ubx7";

int LoraGatewayListener::setSystemTime(
    const uint32_t ppmCountUS
)
{
    struct timespec value;
    lgw_cnt2utc(gpsTimeReference, ppmCountUS, &value);
    value.tv_nsec = 0;
    int r = clock_settime(CLOCK_REALTIME, &value);
    if (r)
        return ERR_CODE_LORA_GATEWAY_GPS_SYNC_TIME;
    return 0;
}

/**
 * Parse GPS timestamp message with PPM pulse
 * Set time
 **/
int LoraGatewayListener::syncGPSTime()
{
    if (!gpsEnabled) {
        gpsTimeLastSynced = 0;
        return ERR_CODE_LORA_GATEWAY_GPS_DISABLED;
    }
    struct timespec gpsTime;
    struct timespec utc;
    // get GPS time for synchronization. Ignore coordinates.
    int r = lgw_gps_get(&utc, &gpsTime, nullptr, nullptr);
    if (r) {
        gpsTimeLastSynced = 0;
        return ERR_CODE_LORA_GATEWAY_GPS_GET_TIME;
    }

    // get timestamp captured on PPM pulse
    mLGW.lock();
    uint32_t ppmCountUs; // concentrator timestamp associated with PPM pulse
    r = lgw_get_trigcnt(&ppmCountUs);
    mLGW.unlock();

    if (r) {
        gpsTimeLastSynced = 0;
        return ERR_CODE_LORA_GATEWAY_GPS_GET_TIME;
    }

    // Update time reference with the new GPS time & timestamp
    mutexGPSTimeReference.lock();
    r = lgw_gps_sync(&gpsTimeReference, ppmCountUs, utc, gpsTime);
    if (!r) {
        if (gpsTimeLastSynced == 0)
            r = setSystemTime(ppmCountUs);
        gpsTimeLastSynced = utc.tv_sec;
    }
    mutexGPSTimeReference.unlock();

    if (r) {
        gpsTimeLastSynced = 0;
        return ERR_CODE_LORA_GATEWAY_GPS_SYNC_TIME;
    }
    return 0;
}

/**
 * Parse GPS coordinates
 * Set coordinates
 **/
int LoraGatewayListener::syncGPSLocation()
{
    if (!gpsEnabled) {
        gpsCoordsLastSynced = 0;
        return ERR_CODE_LORA_GATEWAY_GPS_DISABLED;
    }
    struct coord_s gpserr;  // ignore deviation
    // get GPS time and coordinates for synchronization. Ignore coordinates deviation.
    int r = lgw_gps_get(nullptr, nullptr, &gpsLastCoord, &gpserr);
    if (r) {
        gpsCoordsLastSynced = 0;
        return ERR_CODE_LORA_GATEWAY_GPS_GET_COORDS;
    }
    return 0;
}

void LoraGatewayListener::spectralScanRunner()
{
    if (!config)
        return;
    spectralScanThreadRunning = true;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_SPECTRAL_SCAN_STARTED);

    uint32_t freqHz = config->sx1261()->spectralScan.freq_hz_start;
    uint32_t freqHzStop = config->sx1261()->spectralScan.freq_hz_start + config->sx1261()->spectralScan.nb_chan * 200E3;
    int16_t levels[LGW_SPECTRAL_SCAN_RESULT_SIZE];
    uint16_t results[LGW_SPECTRAL_SCAN_RESULT_SIZE];
    struct timeval startTime;
    lgw_spectral_scan_status_t status;
    uint8_t tx_status = TX_FREE;
    bool spectralScanStarted;

    while (!stopRequest) {
        // Pace the scan thread (1 sec min), and avoid waiting several seconds when exit
        for (int i = 0; i < (int)(config->sx1261()->spectralScan.pace_s ? config->sx1261()->spectralScan.pace_s : 1); i++) {
            if (stopRequest)
                break;
            wait_ms(1000);
        }
        spectralScanStarted = false;

        // Start spectral scan (if no downlink programmed)
        mLGW.lock();
        // Check if there is a downlink programmed
        for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
            if (config->sx130x()->rfConfs[i].tx_enable) {
                int x = lgw_status((uint8_t)i, TX_STATUS, &tx_status);
                if (x != LGW_HAL_SUCCESS) {
                    log(LOG_ERR, ERR_CODE_LORA_GATEWAY_GET_TX_STATUS, ERR_LORA_GATEWAY_GET_TX_STATUS);
                } else {
                    if (tx_status == TX_SCHEDULED || tx_status == TX_EMITTING) {
                        log(LOG_ERR, ERR_CODE_LORA_GATEWAY_SKIP_SPECTRAL_SCAN, ERR_LORA_GATEWAY_SKIP_SPECTRAL_SCAN);
                        break;
                    }
                }
            }
        }
        if (tx_status != TX_SCHEDULED && tx_status != TX_EMITTING) {
            int x = lgw_spectral_scan_start(freqHz, config->sx1261()->spectralScan.nb_scan);
            if (x != 0) {
                mLGW.unlock();
                // write to log
                log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_START_FAILED, ERR_LORA_GATEWAY_SPECTRAL_SCAN_START_FAILED);
                continue;
            }
            spectralScanStarted = true;
        }
        mLGW.unlock();
        if (spectralScanStarted) {
            // Wait for scan to be completed
            status = LGW_SPECTRAL_SCAN_STATUS_UNKNOWN;
            timeout_start(&startTime);
            do {
                // handle timeout
                if (timeout_check(startTime, 2000) != 0) {
                    // write to log
                    log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_TIMEOUT, ERR_LORA_GATEWAY_SPECTRAL_SCAN_TIMEOUT);
                    break;
                }

                // get spectral scan status
                mLGW.lock();
                int x = lgw_spectral_scan_get_status(&status);
                mLGW.unlock();
                if (x != 0) {
                    // write to log
                    log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_FAILED, ERR_LORA_GATEWAY_SPECTRAL_SCAN_FAILED);
                    break;
                }

                // wait a bit before checking status again
                wait_ms(10);
            } while (status != LGW_SPECTRAL_SCAN_STATUS_COMPLETED && status != LGW_SPECTRAL_SCAN_STATUS_ABORTED);
            if (status == LGW_SPECTRAL_SCAN_STATUS_COMPLETED) {
                // Get spectral scan results
                memset(levels, 0, sizeof levels);
                memset(results, 0, sizeof results);
                mLGW.lock();
                int x = lgw_spectral_scan_get_results(levels, results);
                mLGW.unlock();
                if (x != 0) {
                    log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_RESULT, ERR_LORA_GATEWAY_SPECTRAL_SCAN_RESULT);
                    continue; // main while loop
                }
                // print results
                // prevent change onSpectralScan
                mReportSpectralScan.lock();
                if (onSpectralScan) {
                    onSpectralScan(this, freqHz, results);
                }
                mReportSpectralScan.unlock();
                // Next frequency to scan
                freqHz += 200000; // 200kHz channels
                if (freqHz >= freqHzStop) {
                    freqHz = config->sx1261()->spectralScan.freq_hz_start;
                }
            } else if (status == LGW_SPECTRAL_SCAN_STATUS_ABORTED) {
                // write to log
                log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_ABORTED, ERR_LORA_GATEWAY_SPECTRAL_SCAN_ABORTED);
            } else {
                // write to log
                log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_UNEXPECTED_STATUS, ERR_LORA_GATEWAY_SPECTRAL_SCAN_UNEXPECTED_STATUS);
            }
        }
    }
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_SPECTRAL_SCAN_FINISHED);
    spectralScanThreadRunning = false;
}

void LoraGatewayListener::gpsRunner()
{
    gpsThreadRunning = true;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_GPS_STARTED);
    char serial_buff[128]; // buffer to receive GPS data
    memset(serial_buff, 0, sizeof serial_buff);

    size_t wr_idx = 0;     // pointer to end of chars in buffer

    // track of latest NMEA message parsed
    enum gps_msg latest_msg;

    while (!stopRequest) {
        size_t rd_idx = 0;
        size_t frame_end_idx = 0;

        // blocking non-canonical read on serial port
        ssize_t nb_char = read(fdGpsTty, serial_buff + wr_idx, LGW_GPS_MIN_MSG_SIZE);
        if (nb_char <= 0)
            continue;
        wr_idx += (size_t) nb_char;

        // Scan buffer for UBX/NMEA sync chars and attempt to decode frame if one is found
        while (rd_idx < wr_idx) {
            size_t frame_size = 0;

            // Scan buffer for UBX sync char
            if (serial_buff[rd_idx] == (char)LGW_GPS_UBX_SYNC_CHAR) {
                // Found UBX sync char
                latest_msg = lgw_parse_ubx(&serial_buff[rd_idx], (wr_idx - rd_idx), &frame_size);

                if (frame_size > 0) {
                    if (latest_msg == INCOMPLETE) {
                        // UBX header found but frame appears to be missing bytes
                        frame_size = 0;
                    } else if (latest_msg == INVALID) {
                        // message header received but message appears to be corrupted
                        frame_size = 0;
                    } else if (latest_msg == UBX_NAV_TIMEGPS) {
                        syncGPSTime();
                    }
                }
            } else if (serial_buff[rd_idx] == (char)LGW_GPS_NMEA_SYNC_CHAR) {
                // Found NMEA sync char
                // scan for NMEA end marker LF
                char* nmea_end_ptr = (char*) memchr(&serial_buff[rd_idx],(int)0x0a, (wr_idx - rd_idx));
                if (nmea_end_ptr) {
                    // found end marker
                    frame_size = nmea_end_ptr - &serial_buff[rd_idx] + 1;
                    latest_msg = lgw_parse_nmea(&serial_buff[rd_idx], frame_size);

                    if (latest_msg == INVALID || latest_msg == UNKNOWN) {
                        // checksum failed
                        frame_size = 0;
                    } else if (latest_msg == NMEA_RMC) { // Get location from RMC frames
                        syncGPSLocation();
                    }
                }
            }

            if (frame_size > 0) {
                // At this point message is a checksum verified frame we're processed or ignored. Remove frame from buffer
                rd_idx += frame_size;
                frame_end_idx = rd_idx;
            } else {
                rd_idx++;
            }
        }

        if (frame_end_idx) {
            // Frames have been processed. Remove bytes to end of last processed frame
            memcpy(serial_buff, &serial_buff[frame_end_idx], wr_idx - frame_end_idx);
            wr_idx -= frame_end_idx;
        }

        // Prevent buffer overflow
        if ((sizeof(serial_buff) - wr_idx) < LGW_GPS_MIN_MSG_SIZE) {
            memcpy(serial_buff, &serial_buff[LGW_GPS_MIN_MSG_SIZE], wr_idx - LGW_GPS_MIN_MSG_SIZE);
            wr_idx -= LGW_GPS_MIN_MSG_SIZE;
        }
    }
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_GPS_FINISHED);
    gpsThreadRunning = false;
}

/**
 * Check time reference and calculate XTAL correction
 */
void LoraGatewayListener::gpsCheckTimeRunner() {
    gpsCheckTimeThreadRunning = true;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_CHECK_TIME_STARTED);

    // GPS reference validation variables
    long gps_ref_age = 0;
    bool ref_valid_local = false;
    double xtal_err_cpy;

    // variables for XTAL correction averaging
    unsigned init_cpt = 0;
    double init_acc = 0.0;
    double x;

    while (!stopRequest) {
        wait_ms(1000);
        // calculate when the time reference was last updated
        mutexGPSTimeReference.lock();
        gps_ref_age = (long)difftime(time(nullptr), gpsTimeReference.systime);
        if ((gps_ref_age >= 0) && (gps_ref_age <= GPS_REF_MAX_AGE)) {
            /* time ref is ok, validate and  */
            gps_ref_valid = true;
            ref_valid_local = true;
            xtal_err_cpy = gpsTimeReference.xtal_err;
        } else {
            // time ref is too old, invalidate
            gps_ref_valid = false;
            ref_valid_local = false;
        }
        mutexGPSTimeReference.unlock();

        // manage XTAL correction
        if (!ref_valid_local) {
            // couldn't sync, or sync too old -> invalidate XTAL correction
            mXTALcorrection.lock();
            xtal_correct_ok = false;
            xtal_correct = 1.0;
            mXTALcorrection.unlock();
            init_cpt = 0;
            init_acc = 0.0;
        } else {
            if (init_cpt < XERR_INIT_AVG) {
                /* initial accumulation */
                init_acc += xtal_err_cpy;
                ++init_cpt;
            } else if (init_cpt == XERR_INIT_AVG) {
                /* initial average calculation */
                mXTALcorrection.lock();
                xtal_correct = (double)(XERR_INIT_AVG) / init_acc;
                xtal_correct_ok = true;
                mXTALcorrection.unlock();
                ++init_cpt;
            } else {
                // tracking with low-pass filter
                x = 1 / xtal_err_cpy;
                mXTALcorrection.lock();
                xtal_correct = xtal_correct - xtal_correct/XERR_FILT_COEF + x/XERR_FILT_COEF;
                mXTALcorrection.unlock();
            }
        }
    }
    gpsCheckTimeThreadRunning = false;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_CHECK_TIME_FINISHED);
}

#define STATUS_SIZE        200
// max number of packets per fetch/send cycle
#define NB_PKT_MAX         255
#define TX_BUFF_SIZE       ((540 * NB_PKT_MAX) + 30 + STATUS_SIZE)
// ms waited when a fetch return no packets
#define FETCH_SLEEP_MS     10
#define GATEWAY_PROTOCOL    2
#define UNIX_GPS_EPOCH_OFFSET 315964800 // Number of seconds ellapsed between 01.Jan.1970 00:00:00 and 06.Jan.1980 00:00:00

/**
 * Receive Lora packets from end-device(s)
 */
void LoraGatewayListener::upstreamRunner()
{
    SEMTECH_PROTOCOL_METADATA metadata;
    std::string payload;

    metadata.gatewayId = config->gateway()->gatewayId;

    upstreamThreadRunning = true;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_UPSTREAM_STARTED);

    // allocate memory for metadata fetching and processing
    struct lgw_pkt_rx_s rxpkt[NB_PKT_MAX]; // array containing inbound packets + metadata
    struct lgw_pkt_rx_s *p; // pointer on a RX metadata

    // local copy of GPS time reference
    bool ref_ok = false; // determine if GPS time reference must be used or not
    struct tref local_ref; // time reference used for UTC <-> timestamp conversion

    struct timespec recv_time;

    while (!stopRequest) {
        // fetch packets
        mLGW.lock();
        int nb_pkt = lgw_receive(NB_PKT_MAX, rxpkt);
        mLGW.unlock();
        if (nb_pkt == LGW_HAL_ERROR) {
            log(LOG_ERR, ERR_CODE_LORA_GATEWAY_FETCH, ERR_LORA_GATEWAY_FETCH);
            // fatal error, exit
            stop(0);
            return;
        }

        // wait a short time if no packets, nor status report
        if (nb_pkt == 0) {
            wait_ms(FETCH_SLEEP_MS);
            continue;
        }

        // get a copy of GPS time reference (avoid 1 mutex per metadata)
        if (config->gateway()->gpsEnabled) {
            mutexGPSTimeReference.lock();
            ref_ok = gps_ref_valid;
            local_ref = gpsTimeReference;
            mutexGPSTimeReference.unlock();
        } else {
            ref_ok = false;
        }

        // serialize Lora packets metadata and payload
        int pkt_in_dgram = 0;
        for (int i = 0; i < nb_pkt; ++i) {
            p = &rxpkt[i];
            // basic metadata filtering
            measurements.inc(meas_nb_rx_rcv);
            switch(p->status) {
                case STAT_CRC_OK:
                    measurements.inc(meas_nb_rx_ok);
                    if (!config->gateway()->forwardCRCValid) {
                        continue; // skip that metadata
                    }
                    break;
                case STAT_CRC_BAD:
                    measurements.inc(meas_nb_rx_bad);
                    if (!config->gateway()->forwardCRCError) {
                        continue; // skip that metadata
                    }
                    break;
                case STAT_NO_CRC:
                    measurements.inc(meas_nb_rx_nocrc);
                    if (!config->gateway()->forwardCRCDisabled) {
                        continue; // skip that metadata
                    }
                    break;
                default:
                    log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_UNKNOWN_STATUS, ERR_LORA_GATEWAY_UNKNOWN_STATUS);
                    continue; // skip that metadata
                    // exit(EXIT_FAILURE);
            }
            measurements.inc(meas_up_pkt_fwd);
            measurements.inc(meas_up_payload_byte, p->size);

            log(LOG_INFO, ERR_CODE_LORA_GATEWAY_RECEIVED, ERR_LORA_GATEWAY_RECEIVED);

            // time
            metadata.tmst = p->count_us;

            if (ref_ok) {
                // convert metadata timestamp to UTC absolute time
                struct timespec pkt_utc_time;
                int r = lgw_cnt2utc(local_ref, p->count_us, &pkt_utc_time);
                if (r == LGW_GPS_SUCCESS)
                    metadata.t = pkt_utc_time.tv_sec;
                else
                    metadata.t = 0;
                // convert metadata timestamp to GPS absolute time
                struct timespec pkt_gps_time;
                r = lgw_cnt2gps(local_ref, p->count_us, &pkt_gps_time);
                if (r == LGW_GPS_SUCCESS) {
                    // uint64_t pkt_gps_time_ms = pkt_gps_time.tv_sec * 1E3 + pkt_gps_time.tv_nsec / 1E6;
                    metadata.tmst = pkt_gps_time.tv_sec;
                }
            } else {
                metadata.t = time(nullptr);
            }

            // Packet concentrator channel, RF chain & RX frequency, 34-36 useful chars
            metadata.rfch = p->rf_chain;
            metadata.chan = p->if_chain;
            metadata.freq = p->freq_hz;

            // Validate metadata status
            switch (p->status) {
                case STAT_CRC_OK:
                    metadata.stat = 1;
                    break;
                case STAT_CRC_BAD:
                    metadata.stat = -1;
                    break;
                case STAT_NO_CRC:
                    metadata.stat = 0;
                    break;
                default:
                    log(LOG_ERR, ERR_CODE_LORA_GATEWAY_UNKNOWN_STATUS, ERR_LORA_GATEWAY_UNKNOWN_STATUS);
                    metadata.stat = -2;
                    continue;
                    return;
            }

            // Packet modulation, 13-14 useful chars
            if (p->modulation == MOD_LORA) {
                metadata.modu = LORA;

                // Lora datarate & bandwidth, 16-19 useful chars
                switch (p->datarate) {
                    case DR_LORA_SF5:
                        metadata.spreadingFactor = DRLORA_SF5;
                        break;
                    case DR_LORA_SF6:
                        metadata.spreadingFactor = DRLORA_SF6;
                        break;
                    case DR_LORA_SF7:
                        metadata.spreadingFactor = DRLORA_SF7;
                        break;
                    case DR_LORA_SF8:
                        metadata.spreadingFactor = DRLORA_SF8;
                        break;
                    case DR_LORA_SF9:
                        metadata.spreadingFactor = DRLORA_SF9;
                        break;
                    case DR_LORA_SF10:
                        metadata.spreadingFactor = DRLORA_SF10;
                        break;
                    case DR_LORA_SF11:
                        metadata.spreadingFactor = DRLORA_SF11;
                        break;
                    case DR_LORA_SF12:
                        metadata.spreadingFactor = DRLORA_SF12;
                        break;
                    default:
                        metadata.spreadingFactor = DRLORA_SF5;
                        log(LOG_ERR, ERR_CODE_LORA_GATEWAY_UNKNOWN_DATARATE, ERR_LORA_GATEWAY_UNKNOWN_DATARATE);
                        continue;
                }
                switch (p->bandwidth) {
                    case BW_125KHZ:
                        metadata.bandwith = BANDWIDTH_INDEX_125KHZ;
                        break;
                    case BW_250KHZ:
                        metadata.bandwith = BANDWIDTH_INDEX_250KHZ;
                        break;
                    case BW_500KHZ:
                        metadata.bandwith = BANDWIDTH_INDEX_500KHZ;
                        break;
                    default:
                        metadata.bandwith = BANDWIDTH_INDEX_125KHZ;
                        log(LOG_ERR, ERR_CODE_LORA_GATEWAY_UNKNOWN_BANDWIDTH, ERR_LORA_GATEWAY_UNKNOWN_BANDWIDTH);
                        continue;
                }

                // Packet ECC coding rate, 11-13 useful chars
                switch (p->coderate) {
                    case CR_LORA_4_5:
                        metadata.codingRate = CRLORA_4_5;
                        break;
                    case CR_LORA_4_6:
                        metadata.codingRate = CRLORA_4_6;
                        break;
                    case CR_LORA_4_7:
                        metadata.codingRate = CRLORA_4_7;
                        break;
                    case CR_LORA_4_8:
                        metadata.codingRate = CRLORA_4_8;
                        break;
                    case 0: // treat the CR0 case (mostly false sync)
                        metadata.codingRate = CRLORA_0FF;
                        break;
                    default:
                        log(LOG_ERR, ERR_CODE_LORA_GATEWAY_UNKNOWN_CODERATE, ERR_LORA_GATEWAY_UNKNOWN_CODERATE);
                        continue;
                }

                // Signal RSSI, payload size
                metadata.rssi = (int16_t) p->rssis;

                // Lora SNR
                metadata.lsnr = p->snr;
            } else if (p->modulation == MOD_FSK) {
                metadata.modu = FSK;
                metadata.bps = p->datarate;
            } else {
                log(LOG_ERR, ERR_CODE_LORA_GATEWAY_UNKNOWN_MODULATION, ERR_LORA_GATEWAY_UNKNOWN_MODULATION);
                continue;
            }

            payload = std::string((char *)p->payload, p->size);
            pkt_in_dgram++;
        }

        // restart fetch sequence without empty call if all packets have been filtered out
        if (pkt_in_dgram == 0)
            continue;

        // send to the network server
        if (onUpstream)
            onUpstream(this, &metadata, payload);

        measurements.inc(meas_up_dgram_sent);
        measurements.inc(meas_up_network_byte, p->size);    // no network traffic, return size of payload
        // do not wait for ACK, let say it received
        measurements.inc(meas_up_ack_rcv);
    }
    upstreamThreadRunning = false;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_UPSTREAM_FINISHED);

}

#define PROTOCOL_VERSION            2           // v1.6
#define MIN_LORA_PREAMBLE_LEN       6           // minimum Lora preamble length
#define MIN_FSK_PREAMBBLE_LEN       3           // minimum FSK preamble length

static uint16_t crc16(
    const uint8_t *data,
    size_t size
) {
    if (!data)
        return 0;
    const uint16_t crc_poly = 0x1021;
    const uint16_t init_val = 0x0000;
    uint16_t r = init_val;

    for (unsigned int i = 0; i < size; ++i) {
        r ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; ++j) {
            r = (r & 0x8000) ? (r << 1) ^ crc_poly : (r << 1);
        }
    }
    return r;
}

static double difftimespec(
    struct timespec end,
    struct timespec beginning
)
{
    double r;
    r = 1E-9 * (double)(end.tv_nsec - beginning.tv_nsec);
    r += (double)(end.tv_sec - beginning.tv_sec);
    return r;
}

bool LoraGatewayListener::getTxGainLutIndex(
    uint8_t rf_chain,
    int8_t rf_power,
    uint8_t *lut_index
)
{
    uint8_t pow_index;
    int current_best_index = -1;
    uint8_t current_best_match = 0xff;
    int diff;

    if (!lut_index)
        return false;

    // Search requested power in TX gain LUT
    for (pow_index = 0; pow_index < config->sx130x()->txLut[rf_chain].size; pow_index++) {
        diff = rf_power - config->sx130x()->txLut[rf_chain].lut[pow_index].rf_power;
        if (diff < 0) {
            // The selected power must be lower or equal to requested one
            continue;
        } else {
            // Record the index corresponding to the closest rf_power available in LUT
            if ((current_best_index == -1) || (diff < current_best_match)) {
                current_best_match = diff;
                current_best_index = pow_index;
            }
        }
    }

    // Return corresponding index
    if (current_best_index > -1) {
        *lut_index = (uint8_t)current_best_index;
    } else {
        *lut_index = 0;
        return false;
    }
    return true;
}

/**
 * Validate transmission packet in tx param, if packet is valid, enqueueTxPacket packet to be sent or send immediately
 * @param tx
 * @return
 */
int LoraGatewayListener::enqueueTxPacket(
    TxPacket &tx
)
{
    // determine packet type (class A, B or C)
    enum jit_pkt_type_e downlinkClass;
    // and calculate appropriate time to send
    switch(tx.pkt.tx_mode) {
        case IMMEDIATE:
            // TX procedure: send immediately
            downlinkClass = JIT_PKT_TYPE_DOWNLINK_CLASS_C;
            break;
        case TIMESTAMPED:
            // tx.pkt.count_us is time stamp
            // Concentrator timestamp is given, we consider it is a Class A downlink
            downlinkClass = JIT_PKT_TYPE_DOWNLINK_CLASS_A;
            break;
        case ON_GPS:
        {
            // otherwise send on GPS time (converted to timestamp packet)
            // tx.pkt.count_us is time stamp
            struct tref local_ref; // time reference used for UTC <-> timestamp conversion
            if (config->gateway()->gpsEnabled) {
                mutexGPSTimeReference.lock();
                if (gps_ref_valid) {
                    local_ref = gpsTimeReference;
                    mutexGPSTimeReference.unlock();
                } else {
                    mutexGPSTimeReference.unlock();
                    log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME, ERR_LORA_GATEWAY_SEND_AT_GPS_TIME);
                    // TODO inform network server
                    return ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME;
                }
            } else {
                log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME_DISABLED, ERR_LORA_GATEWAY_SEND_AT_GPS_TIME_DISABLED);
                // TODO inform network server
                return ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME_DISABLED;
            }

            // Convert GPS time from milliseconds to timespec
            double x4;
            double x3 = modf((double) tx.pkt.count_us / 1E3, &x4);
            struct timespec gps_tx; // GPS time that needs to be converted to timestamp
            gps_tx.tv_sec = (time_t) x4; // get seconds from integer part
            gps_tx.tv_nsec = (long) (x3 * 1E9); // get nanoseconds from fractional part

            // transform GPS time to timestamp
            int r = lgw_gps2cnt(local_ref, gps_tx, &(tx.pkt.count_us));
            if (r != LGW_GPS_SUCCESS) {
                log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME_INVALID, ERR_LORA_GATEWAY_SEND_AT_GPS_TIME_INVALID);
                return ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME_INVALID;
            } else {
                log(LOG_INFO, 0, MSG_LORA_GATEWAY_SEND_AT_GPS_TIME);
            }
            // GPS timestamp is given, we consider it is a Class B downlink
            downlinkClass = JIT_PKT_TYPE_DOWNLINK_CLASS_B;
        }
        default:
            log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_UNKNOWN_TX_MODE, ERR_LORA_GATEWAY_UNKNOWN_TX_MODE);
            return ERR_CODE_LORA_GATEWAY_UNKNOWN_TX_MODE;
    }

    // Validate is channel allowed
    if (!config->sx130x()->rfConfs[tx.pkt.rf_chain].tx_enable) {
        log(LOG_ERR, ERR_CODE_LORA_GATEWAY_TX_CHAIN_DISABLED, ERR_LORA_GATEWAY_TX_CHAIN_DISABLED);
        return ERR_CODE_LORA_GATEWAY_TX_CHAIN_DISABLED;
    }

    // Correct radio transmission power
    tx.pkt.rf_power -= config->sx130x()->antennaGain;

    // Validate preamble length
    switch (tx.pkt.modulation) {
        case MOD_LORA:
            // Check minimum Lora preamble length
            if (tx.pkt.preamble < MIN_LORA_PREAMBLE_LEN)
                tx.pkt.preamble = MIN_LORA_PREAMBLE_LEN;
            break;
        case MOD_FSK:
            // Check minimum FSK preamble length
            if (tx.pkt.preamble < MIN_FSK_PREAMBBLE_LEN)
                tx.pkt.preamble = MIN_FSK_PREAMBBLE_LEN;
            break;
        default:
            log(LOG_ERR, ERR_CODE_LORA_GATEWAY_UNKNOWN_MODULATION, ERR_LORA_GATEWAY_UNKNOWN_MODULATION);
            return ERR_CODE_LORA_GATEWAY_UNKNOWN_MODULATION;
            break;
    }

    // record measurement data
    measurements.inc(meas_dw_dgram_rcv);    // count only datagrams with no JSON errors
    measurements.inc(meas_dw_dgram_rcv);
    measurements.inc(meas_dw_network_byte); // meas_dw_network_byte
    measurements.inc(meas_dw_payload_byte, tx.pkt.size);

    // reset error/warning results
    int jit_result = JIT_ERROR_OK;
    int warning_result = JIT_ERROR_OK;

    // check TX frequency before trying to queue packet
    if ((tx.pkt.freq_hz < config->sx130x()->tx_freq_min[tx.pkt.rf_chain]) || (tx.pkt.freq_hz > config->sx130x()->tx_freq_max[tx.pkt.rf_chain])) {
        jit_result = JIT_ERROR_TX_FREQ;
        log(LOG_ERR, ERR_CODE_LORA_GATEWAY_TX_UNSUPPORTED_FREQUENCY, ERR_LORA_GATEWAY_TX_UNSUPPORTED_FREQUENCY);
        return ERR_CODE_LORA_GATEWAY_TX_UNSUPPORTED_FREQUENCY;
    }

    // check TX power before trying to queue packet, send a warning if not supported
    if (jit_result == JIT_ERROR_OK) {
        uint8_t tx_lut_idx;
        int r = getTxGainLutIndex(tx.pkt.rf_chain, tx.pkt.rf_power, &tx_lut_idx);
        if ((r < 0) || (config->sx130x()->txLut[tx.pkt.rf_chain].lut[tx_lut_idx].rf_power != tx.pkt.rf_power)) {
            // this RF power is not supported, throw a warning, and use the closest lower power supported
            warning_result = JIT_ERROR_TX_POWER;
            log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_TX_UNSUPPORTED_POWER, ERR_LORA_GATEWAY_TX_UNSUPPORTED_POWER);
            tx.pkt.rf_power = config->sx130x()->txLut[tx.pkt.rf_chain].lut[tx_lut_idx].rf_power;
        }
    }

    // insert packet to be sent into JIT queue
    if (jit_result == JIT_ERROR_OK) {
        mLGW.lock();
        uint32_t current_concentrator_time;
        lgw_get_instcnt(&current_concentrator_time);
        mLGW.unlock();
        jit_result = jit_enqueue(&jit_queue[tx.pkt.rf_chain], current_concentrator_time, &tx.pkt, downlinkClass);
        if (jit_result) {
            switch (jit_result) {
                case JIT_ERROR_TOO_EARLY:
                    measurements.inc(meas_nb_tx_rejected_too_early);
                    break;
                case JIT_ERROR_TOO_LATE:
                    measurements.inc(meas_nb_tx_rejected_too_late);
                    break;
                case JIT_ERROR_COLLISION_PACKET:
                    measurements.inc(meas_nb_tx_rejected_collision_packet);
                    break;
                default:
                    break;
            }

            log(LOG_ERR, ERR_CODE_LORA_GATEWAY_JIT_ENQUEUE_FAILED, ERR_LORA_GATEWAY_JIT_ENQUEUE_FAILED);
            return ERR_CODE_LORA_GATEWAY_TX_UNSUPPORTED_FREQUENCY;
        } else {
            // In case of a warning having been raised before, we notify it
            jit_result = warning_result;
        }
        measurements.inc(meas_nb_tx_requested);
    }
    return LORA_OK;
}

/**
 * Transmit beacons
 */
void LoraGatewayListener::downstreamBeaconRunner() {
    if (!gpsEnabled) {
        // TODO remove this return in production
        log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_BEACON_DOWNSTREAM_NO_GPS);
        return;
    }
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_BEACON_DOWNSTREAM_STARTED);
    downstreamBeaconThreadRunning = true;
    // beacon variables
    struct lgw_pkt_tx_s beacon_pkt;
    uint8_t beacon_chan;
    size_t beacon_RFU1_size = 0;
    size_t beacon_RFU2_size = 0;
    uint8_t beacon_pyld_idx = 0;
    time_t diff_beacon_time;
    struct timespec next_beacon_gps_time; // gps time of next beacon packet
    struct timespec last_beacon_gps_time; // gps time of last enqueued beacon packet

    // beacon data fields, byte 0 is Least Significant Byte
    int32_t field_latitude; // 3 bytes, derived from reference latitude
    int32_t field_longitude; // 3 bytes, derived from reference longitude
    uint16_t field_crc1, field_crc2;

    // Just In Time downlink
    uint32_t current_concentrator_time;
    enum jit_error_e jit_result = JIT_ERROR_OK;
    enum jit_pkt_type_e downlink_type;
    enum jit_error_e warning_result = JIT_ERROR_OK;
    uint8_t tx_lut_idx = 0;

    // beacon variables initialization
    last_beacon_gps_time.tv_sec = 0;
    last_beacon_gps_time.tv_nsec = 0;

    // beacon packet parameters
    beacon_pkt.tx_mode = ON_GPS; // send on PPS pulse
    beacon_pkt.rf_chain = 0; // antenna A
    beacon_pkt.rf_power = config->gateway()->beaconPower;
    beacon_pkt.modulation = MOD_LORA;
    switch (config->gateway()->beaconBandwidthHz) {
        case 125000:
            beacon_pkt.bandwidth = BW_125KHZ;
            break;
        case 500000:
            beacon_pkt.bandwidth = BW_500KHZ;
            break;
        default:
            log(LOG_ERR, ERR_CODE_LORA_GATEWAY_UNKNOWN_BANDWIDTH, ERR_LORA_GATEWAY_UNKNOWN_BANDWIDTH);
            stop(0);
    }
    switch (config->gateway()->beaconDataRate) {
        case 8:
            beacon_pkt.datarate = DR_LORA_SF8;
            beacon_RFU1_size = 1;
            beacon_RFU2_size = 3;
            break;
        case 9:
            beacon_pkt.datarate = DR_LORA_SF9;
            beacon_RFU1_size = 2;
            beacon_RFU2_size = 0;
            break;
        case 10:
            beacon_pkt.datarate = DR_LORA_SF10;
            beacon_RFU1_size = 3;
            beacon_RFU2_size = 1;
            break;
        case 12:
            beacon_pkt.datarate = DR_LORA_SF12;
            beacon_RFU1_size = 5;
            beacon_RFU2_size = 3;
            break;
        default:
            log(LOG_ERR, ERR_CODE_LORA_GATEWAY_UNKNOWN_DATARATE, ERR_LORA_GATEWAY_UNKNOWN_DATARATE);
            stop(0);
    }
    beacon_pkt.size = beacon_RFU1_size + 4 + 2 + 7 + beacon_RFU2_size + 2;
    beacon_pkt.coderate = CR_LORA_4_5;
    beacon_pkt.invert_pol = false;
    beacon_pkt.preamble = 10;
    beacon_pkt.no_crc = true;
    beacon_pkt.no_header = true;

    // network common part beacon fields (little endian)
    for (int i = 0; i < (int)beacon_RFU1_size; i++) {
        beacon_pkt.payload[beacon_pyld_idx++] = 0;
    }

    // network common part beacon fields (little endian)
    beacon_pyld_idx += 4; // time (variable), filled later
    beacon_pyld_idx += 2; // crc1 (variable), filled later

    // calculate the latitude and longitude that must be publicly reported
    field_latitude = (int32_t)((config->gateway()->refGeoCoordinates.lat / 90.0) * (double)(1<<23));
    if (field_latitude > (int32_t) 0x007fffff) {
        field_latitude = (int32_t) 0x007ffffF; // +90 N is represented as 89.99999 N
    } else if (field_latitude < (int32_t) 0xff800000) {
        field_latitude = (int32_t) 0xff800000;
    }
    field_longitude = (int32_t)((config->gateway()->refGeoCoordinates.lon / 180.0) * (double)(1<<23));
    if (field_longitude > (int32_t) 0x007fffff) {
        field_longitude = (int32_t) 0x007fffff; // +180 E is represented as 179.99999 E
    } else if (field_longitude < (int32_t) 0xff800000) {
        field_longitude = (int32_t) 0xff800000;
    }

    // gateway specific beacon fields
    beacon_pkt.payload[beacon_pyld_idx++] = config->gateway()->beaconInfoDesc;
    beacon_pkt.payload[beacon_pyld_idx++] = 0xff &  field_latitude;
    beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (field_latitude >> 8);
    beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (field_latitude >> 16);
    beacon_pkt.payload[beacon_pyld_idx++] = 0xff &  field_longitude;
    beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (field_longitude >> 8);
    beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (field_longitude >> 16);

    // RFU
    for (int i = 0; i < (int)beacon_RFU2_size; i++) {
        beacon_pkt.payload[beacon_pyld_idx++] = 0;
    }

    // CRC of the beacon gateway specific part fields
    field_crc2 = crc16((beacon_pkt.payload + 6 + beacon_RFU1_size), 7 + beacon_RFU2_size);
    beacon_pkt.payload[beacon_pyld_idx++] = 0xff &  field_crc2;
    beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (field_crc2 >> 8);

    while (!stopRequest) {
        // Pre-allocate beacon slots in JiT queue, to check downlink collisions
        int retry = 0;
        int beacon_loop = JIT_NUM_BEACON_IN_QUEUE - jit_queue[0].num_beacon;
        while (beacon_loop > 0 && config->gateway()->beaconPeriod > 0) {
            mutexGPSTimeReference.lock();
            // Wait for GPS to be ready before inserting beacons in JiT queue
            if (gps_ref_valid && xtal_correct_ok) {
                // compute GPS time for next beacon to come
                //   LoRaWAN: T = k*beacon_period + TBeaconDelay
                //            with TBeaconDelay = [1.5ms +/- 1µs]*/
                if (last_beacon_gps_time.tv_sec == 0) {
                    // if no beacon has been queued, get next slot from current GPS time
                    diff_beacon_time = gpsTimeReference.gps.tv_sec % ((time_t) config->gateway()->beaconPeriod);
                    next_beacon_gps_time.tv_sec = gpsTimeReference.gps.tv_sec + ((time_t) config->gateway()->beaconPeriod - diff_beacon_time);
                } else {
                    // if there is already a beacon, take it as reference
                    next_beacon_gps_time.tv_sec = last_beacon_gps_time.tv_sec + config->gateway()->beaconPeriod;
                }
                // now we can add a beacon_period to the reference to get next beacon GPS time
                next_beacon_gps_time.tv_sec += (retry * config->gateway()->beaconPeriod);
                next_beacon_gps_time.tv_nsec = 0;
#if DEBUG_BEACON
                {
                    time_t time_gps = gpsTimeReference.gps.tv_sec + UNIX_GPS_EPOCH_OFFSET;
                    time_t time_last_beacon = last_beacon_gps_time.tv_sec + UNIX_GPS_EPOCH_OFFSET;
                    time_t time_next_beacon = next_beacon_gps_time.tv_sec + UNIX_GPS_EPOCH_OFFSET;
                    std::stringstream ss;
                    ss << MSG_BEACON_TIME << ctime(&time_gps)
                        << MSG_BEACON_TIME_LAST << ctime(&time_last_beacon)
                        << MSG_BEACON_TIME_NEXT << ctime(&time_next_beacon);
                    log(LOG_DEBUG, 0, ss.str());
                }
#endif
                // convert GPS time to concentrator time, and set packet counter for JiT trigger
                lgw_gps2cnt(gpsTimeReference, next_beacon_gps_time, &(beacon_pkt.count_us));
                mutexGPSTimeReference.unlock();

                // apply frequency correction to beacon TX frequency
                if (config->gateway()->beaconFreqNb > 1) {
                    // floor rounding
                    beacon_chan = (next_beacon_gps_time.tv_sec / config->gateway()->beaconPeriod) % config->gateway()->beaconFreqNb;
                } else {
                    beacon_chan = 0;
                }
                // Compute beacon frequency
                beacon_pkt.freq_hz = config->gateway()->beaconFreqHz + (beacon_chan * config->gateway()->beaconFreqStep);

                // load time in beacon payload
                beacon_pyld_idx = beacon_RFU1_size;
                beacon_pkt.payload[beacon_pyld_idx++] = 0xff &  next_beacon_gps_time.tv_sec;
                beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (next_beacon_gps_time.tv_sec >>  8);
                beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (next_beacon_gps_time.tv_sec >> 16);
                beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (next_beacon_gps_time.tv_sec >> 24);

                // calculate CRC
                field_crc1 = crc16(beacon_pkt.payload, 4 + beacon_RFU1_size); // CRC for the network common part
                beacon_pkt.payload[beacon_pyld_idx++] = 0xff & field_crc1;
                beacon_pkt.payload[beacon_pyld_idx++] = 0xff & (field_crc1 >> 8);

                // Insert beacon packet in JiT queue
                mLGW.lock();
                lgw_get_instcnt(&current_concentrator_time);
                mLGW.unlock();
                jit_result = jit_enqueue(&jit_queue[0], current_concentrator_time, &beacon_pkt, JIT_PKT_TYPE_BEACON);
                if (jit_result == JIT_ERROR_OK) {
                    // update stats
                    measurements.inc(meas_nb_beacon_queued);
                    // One more beacon in the queue
                    beacon_loop--;
                    retry = 0;
                    last_beacon_gps_time.tv_sec = next_beacon_gps_time.tv_sec; // keep this beacon time as reference for next one to be programmed

                    // display beacon payload
                    std::stringstream ss;
                    ss << MSG_BEACON_QUEUED << beacon_pkt.count_us
                        << ", " << beacon_pkt.freq_hz
                        << " Hz, " << beacon_pkt.size
                        << " bytes => ";
                    for (int i = 0; i < beacon_pkt.size; ++i) {
                        ss << std::hex << std::setw(2) << std::setfill('0') << beacon_pkt.payload[i];
                    }
                    log(LOG_INFO, 0, ss.str());
                } else {
                    // update stats
                    switch (jit_result) {
                        case JIT_ERROR_COLLISION_BEACON:
                            measurements.inc(meas_nb_tx_rejected_collision_beacon);
                            break;
                        default:
                            measurements.inc(meas_nb_beacon_rejected);
                            break;
                    }
                    log(LOG_INFO, ERR_CODE_LORA_GATEWAY_BEACON_FAILED, ERR_LORA_GATEWAY_BEACON_FAILED);
                    // In case previous enqueueTxPacket failed, we retry one period later until it succeeds
                    // Note: In case the GPS has been unlocked for a while, there can be lots of retries
                    //       to be done from last beacon time to a new valid one
                    retry++;
                }
            } else {
                mutexGPSTimeReference.unlock();
                break;
            }
        }
        sleep(1);
    }
    downstreamBeaconThreadRunning = false;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_BEACON_DOWNSTREAM_FINISHED);
}

/**
 * Transmit packets from JIT queue
 **/
void LoraGatewayListener::jitRunner() {
    jitThreadRunning = true;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_JIT_QUEUE_STARTED);
    int result = LGW_HAL_SUCCESS;
    struct lgw_pkt_tx_s pkt;
    int pkt_index = -1;
    uint32_t current_concentrator_time;
    enum jit_error_e jit_result;
    enum jit_pkt_type_e pkt_type;
    uint8_t tx_status;

    while (!stopRequest) {
        wait_ms(10);
        for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
            // transfer data and metadata to the concentrator, and schedule TX
            mLGW.lock();
            lgw_get_instcnt(&current_concentrator_time);
            mLGW.unlock();
            jit_result = jit_peek(&jit_queue[i], current_concentrator_time, &pkt_index);
            if (jit_result == JIT_ERROR_OK) {
                if (pkt_index > -1) {
                    jit_result = jit_dequeue(&jit_queue[i], pkt_index, &pkt, &pkt_type);
                    if (jit_result == JIT_ERROR_OK) {
                        // update beacon stats
                        if (pkt_type == JIT_PKT_TYPE_BEACON) {
                            // Compensate beacon frequency with xtal error
                            mXTALcorrection.lock();    // prevent xtal_correct
                            pkt.freq_hz = (uint32_t)(xtal_correct * (double) pkt.freq_hz);
                            mXTALcorrection.unlock();

                            // write to log
                            std::stringstream ss;
                            ss << "Beacon pkt " << pkt.freq_hz << " Hz (xtal_correct=" << xtal_correct;
                            log(LOG_INFO, 0, ss.str());

                            // Update statistics
                            measurements.inc(meas_nb_beacon_sent);

                            // write to log
                            std::stringstream ss2;
                            ss2 << MSG_BEACON_DEQUEUED << pkt.count_us;
                            log(LOG_INFO, 0, ss2.str());
                        }

                        // check if concentrator is free for sending new packet
                        mLGW.lock(); // may have to wait for a fetch to finish
                        result = lgw_status(pkt.rf_chain, TX_STATUS, &tx_status);
                        mLGW.unlock(); // free concentrator ASAP
                        if (result == LGW_HAL_ERROR) {
                            // write to log
                            log(LOG_INFO, ERR_CODE_LORA_GATEWAY_STATUS_FAILED, ERR_LORA_GATEWAY_STATUS_FAILED);
                        } else {
                            if (tx_status == TX_EMITTING) {
                                // write to log
                                log(LOG_INFO, ERR_CODE_LORA_GATEWAY_EMIT_ALLREADY, ERR_LORA_GATEWAY_EMIT_ALLREADY);
                                continue;
                            } else if (tx_status == TX_SCHEDULED) {
                                // write to log
                                log(LOG_INFO, ERR_CODE_LORA_GATEWAY_SCHEDULED_ALLREADY, ERR_LORA_GATEWAY_SCHEDULED_ALLREADY);
                            }
                        }

                        // send packet to concentrator
                        mLGW.lock(); // may have to wait for a fetch to finish
                        if (config->sx1261()->spectralScan.enable) {
                            result = lgw_spectral_scan_abort();
                            if (result) {
                                // write to log
                                log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_ABORT_FAILED, ERR_LORA_GATEWAY_SPECTRAL_SCAN_ABORT_FAILED);
                            }
                        }
                        result = lgw_send(&pkt);
                        mLGW.unlock(); // free concentrator ASAP
                        if (result != LGW_HAL_SUCCESS) {
                            measurements.inc(meas_nb_tx_fail);
                            // write to log
                            log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_SEND_FAILED, ERR_LORA_GATEWAY_SEND_FAILED);
                            continue;
                        } else {
                            measurements.inc(meas_nb_tx_ok);
                            // write to log
                            log(LOG_INFO, ERR_CODE_LORA_GATEWAY_SENT, ERR_LORA_GATEWAY_SENT);
                        }
                    } else {
                        // write to log
                        log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_JIT_DEQUEUE_FAILED, ERR_LORA_GATEWAY_JIT_DEQUEUE_FAILED);
                    }
                }
            } else if (jit_result == JIT_ERROR_EMPTY) {
                // Do nothing, it can happen
            } else {
                // write to log
                log(LOG_WARNING, ERR_CODE_LORA_GATEWAY_JIT_PEEK_FAILED, ERR_LORA_GATEWAY_JIT_PEEK_FAILED);
            }
        }
    }
    jitThreadRunning = false;
    log(LOG_DEBUG, LOG_EMBEDDED_GATEWAY, MSG_JIT_QUEUE_FINISHED);
}
LoraGatewayListener::LoraGatewayListener()
    : logVerbosity(0), onUpstream(nullptr), onSpectralScan(nullptr), onLog(nullptr), stopRequest(false),
      upstreamThreadRunning(false), downstreamBeaconThreadRunning(false), jitThreadRunning(false),
      gpsThreadRunning(false), gpsCheckTimeThreadRunning(false), spectralScanThreadRunning(false),
      gps_ref_valid(false),
      lastLgwCode(0), config(nullptr), fdGpsTty(-1), eui(0),
      gpsCoordsLastSynced(0), gpsTimeLastSynced(0), gpsEnabled(false),
      xtal_correct_ok(false), xtal_correct(1.0),
      packetListener(nullptr)
{
    // JIT queue initialization
    jit_queue_init(&jit_queue[0]);
    jit_queue_init(&jit_queue[1]);
}

LoraGatewayListener::LoraGatewayListener(GatewaySettings *cfg)
    : LoraGatewayListener()
{
    config = cfg;
}

LoraGatewayListener::~LoraGatewayListener()
{
}

void LoraGatewayListener::setLogVerbosity(
    int level
)
{
    logVerbosity = level;
}

int LoraGatewayListener::setup()
{
    lastLgwCode = 0;
    if (!config)
        return ERR_CODE_INSUFFICIENT_PARAMS;
    lastLgwCode = lgw_board_setconf(&config->sx130x()->boardConf);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED;
    if (config->sx130x()->tsConf.enable) {
        lastLgwCode = lgw_ftime_setconf(&config->sx130x()->tsConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_TIME_STAMP;
    }
    lastLgwCode = lgw_sx1261_setconf(&config->sx1261()->sx1261);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_SX1261_RADIO;

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        if (config->sx130x()->txLut[i].size) {
            lastLgwCode = lgw_txgain_setconf(i, &config->sx130x()->txLut[i]);
            if (lastLgwCode)
                return ERR_CODE_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT;
        }
    }

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        lastLgwCode = lgw_rxrf_setconf(i, &config->sx130x()->rfConfs[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_INVALID_RADIO;
    }
    lastLgwCode = lgw_demod_setconf(&config->sx130x()->demodConf);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEMODULATION;

    for (int i = 0; i < LGW_MULTI_NB; i++) {
        lastLgwCode = lgw_rxif_setconf(i, &config->sx130x()->ifConfs[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL;
    }
    if (config->sx130x()->ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(8, &config->sx130x()->ifStdConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_STD_CHANNEL;
    } else; // TODO
    if (config->sx130x()->ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(9, &config->sx130x()->ifFSKConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL;
    } else; // TODO
    lastLgwCode = lgw_debug_setconf(config->debug());
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEBUG;
    return LORA_OK;
}

std::string LoraGatewayListener::version()
{
    return std::string(lgw_version_info());
}

bool LoraGatewayListener::getStatus(LGWStatus &status)
{
    mLGW.lock();
    lastLgwCode = lgw_get_temperature(&status.temperature);
    if (!lastLgwCode) {
        lastLgwCode = lgw_get_instcnt(&status.inst_tstamp);
        if (!lastLgwCode)
            lastLgwCode = lgw_get_trigcnt(&status.trig_tstamp);
    }
    mLGW.unlock();
    return lastLgwCode == 0;
}

int LoraGatewayListener::start()
{
    if (!config)
        return ERR_CODE_NO_CONFIG;
    stopRequest = false;
    lastLgwCode = 0;
    // GPS sync
    if (config->gateway()->gpsEnabled) {
        lastLgwCode = lgw_gps_enable((char *) config->gpsTTYPath()->c_str(),
            (char *) DEF_GPS_FAMILY, 0, &fdGpsTty);
        if (lastLgwCode) {
            config->gateway()->gpsEnabled = false;
            // return ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED;
        }
    }
    // load config
    int r = setup();
    if (r)
        return r;
    // starting the concentrator
    lastLgwCode = lgw_start();
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_START_FAILED;
    // get the concentrator EUI
    lastLgwCode = lgw_get_eui(&eui);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_GET_EUI;

    if (!upstreamThreadRunning) {
        std::thread upstreamThread(&LoraGatewayListener::upstreamRunner, this);
        setThreadName(&upstreamThread, MODULE_NAME_GW_UPSTREAM);
        upstreamThread.detach();
    }
    if (!downstreamBeaconThreadRunning) {
        std::thread downstreamBeaconThread(&LoraGatewayListener::downstreamBeaconRunner, this);
        setThreadName(&downstreamBeaconThread, MODULE_NAME_GW_DOWNSTREAM);
        downstreamBeaconThread.detach();
    }
    if (!jitThreadRunning) {
        std::thread jitThread(&LoraGatewayListener::jitRunner, this);
        setThreadName(&jitThread, MODULE_NAME_GW_JIT);
        jitThread.detach();
    }

    if (config->sx1261()->spectralScan.enable) {
        if (!spectralScanThreadRunning) {
            std::thread spectralScanThread(&LoraGatewayListener::spectralScanRunner, this);
            setThreadName(&spectralScanThread, MODULE_NAME_GW_SPECTRAL_SCAN);
            spectralScanThread.detach();
        }
    }

    if (gpsEnabled) {
        if (!gpsThreadRunning) {
            std::thread gpsThread(&LoraGatewayListener::gpsRunner, this);
            setThreadName(&gpsThread, MODULE_NAME_GW_GPS);
            gpsThread.detach();
        }
        if (!gpsCheckTimeThreadRunning) {
            std::thread gpsCheckTimeThread(&LoraGatewayListener::gpsCheckTimeRunner, this);
            setThreadName(&gpsCheckTimeThread, MODULE_NAME_GW_GPS_CHECK_TIME);
            gpsCheckTimeThread.detach();
        }
    }
    return 0;
}

bool LoraGatewayListener::isRunning() const
{
    return upstreamThreadRunning
        && ((!gpsEnabled) || downstreamBeaconThreadRunning)
        && jitThreadRunning
        && ((!gpsEnabled) || (gpsThreadRunning && gpsCheckTimeThreadRunning))
        && ((!config) || (!config->sx1261()->spectralScan.enable) || spectralScanThreadRunning);
}

bool LoraGatewayListener::isStopped() const
{
    return (!upstreamThreadRunning)
       && (!downstreamBeaconThreadRunning)
       && (!jitThreadRunning)
       && (!gpsThreadRunning)
       && (!gpsCheckTimeThreadRunning)
       && (!spectralScanThreadRunning);
}

#define DEF_WAIT_SECONDS    60

int LoraGatewayListener::stop(int waitSeconds)
{
    stopRequest = true;
    if (fdGpsTty >= 0) {
        lastLgwCode = lgw_gps_disable(fdGpsTty);
        fdGpsTty = -1;
    }
    if (waitSeconds <= 0) {
        waitSeconds = DEF_WAIT_SECONDS;
    }

    bool success = false;
    for (int i = 0; i < waitSeconds; i++) {
        if (!isStopped()) {
            sleep(1);
            continue;
        }
        success = true;
    }

    success &= lgw_stop() == 0;

    if (onStop) {
        onStop(this, success);
    }
    return success ? LORA_OK : ERR_CODE_LORA_GATEWAY_SHUTDOWN_TIMEOUT;
}

void LoraGatewayListener::log(
    int level,
    int errorcode,
    const std::string &message
) const
{
    if (!onLog)
        return;
    if (level > logVerbosity)
        return;
    mLog.lock();
    onLog->logMessage((void *) this, LOG_EMBEDDED_GATEWAY, level, errorcode, message);
    mLog.unlock();
}

void LoraGatewayListener::setOnSpectralScan(
    std::function<void(
        const LoraGatewayListener *listener,
        const uint32_t frequency,
        const uint16_t results[LGW_SPECTRAL_SCAN_RESULT_SIZE]
    )> value)
{
    mReportSpectralScan.lock();
    onSpectralScan = value;
    mReportSpectralScan.unlock();
}

void LoraGatewayListener::setOnLog(
    LogIntf *value
)
{
    mLog.lock();
    onLog = value;
    mLog.unlock();
}

void LoraGatewayListener::setOnUpstream(
    std::function<void(
        const LoraGatewayListener *listener,
        const SEMTECH_PROTOCOL_METADATA *metadata,
        const std::string &payload
    )> value
)
{
    // no prevent mutex required
    onUpstream = value;
}

void LoraGatewayListener::setOnStop(
    std::function<void(
        const LoraGatewayListener *listener,
        bool gracefullyStopped
    )> value
)
{
    onStop = value;
}

const char *getMeasurementName(int index)
{
    if (index < 0 || index >= MEASUREMENT_COUNT_SIZE)
        return "";
    return MEASUREMENT_SHORT_NAME[index];
}

std::string LoraGatewayListener::toString() const {
    return "{\"measurements\": " + measurements.toString() + "}";
}