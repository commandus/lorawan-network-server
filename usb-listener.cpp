#include <iostream>
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
    if (metadata) {
        ss << "gatewayId: " << std::hex << metadata->gatewayId
           << std::dec << " frequency: " << metadata->freq
           << " CRC status: " << (int) metadata->stat
           << " modulation: " << (int) metadata->modu
           << " bandwidth: " << (int) metadata->bandwith
           << " SF" << (int) metadata->spreadingFactor
           << " coding rate: " << (int) metadata->codingRate
           << " bps: " << (int) metadata->bps
           << " RSSI: " << (int) metadata->rssi
           << " lsnr: " << metadata->lsnr;
    }
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
        ss
            << " addr: " << std::hex << std::right << std::setw(8) << std::setfill('0') << addr
            << " FCnt: " << std::dec << fcnt
            << " payload: " << hexString(payload) << std::endl;
    }
    listener->log(LOG_INFO, LOG_EMBEDDED_GATEWAY, ss.str());
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
	clear();
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
}

int USBListener::listen(void *config)
{
    if (!config)
        return ERR_CODE_LORA_GATEWAY_START_FAILED;
    listener.config = (GatewaySettings *) config;
    listener.setLogVerbosity(verbosity);
    listener.setOnLog(onLog);
    listener.setOnSpectralScan(onSpectralScan);
    listener.setOnUpstream(onUpstream);

    return listener.start();
}
