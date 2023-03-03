#include <iomanip>

#include "usb-listener.h"
#include "utilstring.h"
#include "errlist.h"

void onUpstream(
    const LoraGatewayListener *listener,
    const SEMTECH_PROTOCOL_METADATA *metadata,
    const std::string &payload
)
{
    if (!listener)
        return;
    std::stringstream ss;
    size_t sz = payload.size();
    if (sz >= 8) {
        uint32_t addr;
        memmove(&addr, &payload[1], sizeof(DEVADDR));
#if BYTE_ORDER == BIG_ENDIAN
        addr = be32toh(addr);
#endif
        uint16_t fcnt;
        memmove(&fcnt, &payload[6], sizeof(fcnt));
#if BYTE_ORDER == BIG_ENDIAN
        fcnt = be16toh(fcnt);
#endif
        if (metadata) {
            ss  << std::hex << std::right << std::setw(8) << std::setfill('0') << addr
                << " " << std::dec << metadata->freq
                << "Hz SF" << (int) metadata->spreadingFactor
                << " " << (int) metadata->rssi
                << "dBm " << metadata->lsnr << "dB ";
        }
        ss
            << hexString(payload)
            << " FCnt " << std::dec << fcnt
            << std::endl;

    }
    listener->log(LOG_INFO, LOG_EMBEDDED_GATEWAY, ss.str());
    if (listener->packetListener) {
        if (listener->packetListener->handler) {
            // get time
            struct timeval receivedTime;
            gettimeofday(&receivedTime, nullptr);
            // get gateway id
            SEMTECH_PREFIX_GW prefix;
            prefix.version = 2;
            prefix.token = 0;   // random number 0
            prefix.tag = SEMTECH_GW_PUSH_DATA;
            // set gateway identifier
            int2deveui(prefix.mac, listener->config->gateway()->gatewayId);
            // construct Semtech packet
            SemtechUDPPacket p(prefix, metadata, payload, listener->packetListener->identityService);
            if (listener->onLog) {
                Payload pl;
                pl.eui = p.getDeviceEUI();
                pl.received = receivedTime.tv_sec;
                pl.frequency = metadata->freq;
                pl.rssi = metadata->rssi;
                pl.lsnr = metadata->lsnr;
                pl.payload = p.payload;
            }
            if (p.errcode)
                listener->packetListener->handler->putUnidentified(receivedTime, p);
            else
                listener->packetListener->handler->put(receivedTime, p);
        }
    }
}

void onSpectralScan(
    const LoraGatewayListener *listener,
    const uint32_t frequency,
    const uint16_t results[LGW_SPECTRAL_SCAN_RESULT_SIZE]
){
    std::stringstream ss;
    ss << MSG_SPECTRAL_SCAN_FREQUENCY "Spectral scan, frequency " << frequency << std::endl;
    for (int i = 0; i < LGW_SPECTRAL_SCAN_RESULT_SIZE; i++) {
        ss << std::dec << std::right << std::setw(7) << std::setfill('0') << results[i] << " ";
    }
    listener->log(LOG_INFO, LOG_EMBEDDED_GATEWAY, ss.str());
}

USBListener::USBListener()
    : PacketListener()
{
}

USBListener::~USBListener() {
}

std::string USBListener::toString() const{
	std::stringstream ss;
    ss << "{\"listener\": " << listener.toString()
        << "}";
	return ss.str();
}

void USBListener::clear() {
    listener.stop(0);
}

bool USBListener::add(
    const std::string& value,
    int hint
)
{
    return false;
}

/**
 * @param config GatewaySettings* mandatory can not be NULL
 * @param flags optional flags
 * @return
 */
int USBListener::listen(
    const std::string &regionName,
    int regionIndex,
    void *config,
    int flags,
    ThreadStartFinish *threadStartFinish)
{
    if (!config)
        return ERR_CODE_NO_CONFIG;
    this->regionName = regionName;
    this->regionIndex = regionIndex;
    // set itself
    listener.packetListener = this;
    // set config
    listener.config = (GatewaySettings *) config;
    listener.flags = flags;
    // copy verbosity level
    listener.setLogVerbosity(verbosity);
    // copy log
    listener.setOnLog(onLog);
    listener.setOnSpectralScan(onSpectralScan);
    listener.setOnUpstream(onUpstream);
    listener.setThreadStartFinish(threadStartFinish);

    int r = listener.start();
    if (r)
        return r;
    while (!listener.isStopped()) {
        // spectral scan?
        // wait until all threads are stopped
        sleep(1);
    }
    return 0;
}
