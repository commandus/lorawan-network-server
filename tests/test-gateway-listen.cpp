#include <string>
#include <iostream>
#include <iomanip>
#include <signal.h>

#include "lora-gateway-listener.h"
#include "gateway-file-json.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"

#ifdef WIN32
#else
#include <execinfo.h>
#endif

#define TRACE_BUFFER_SIZE   256

#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#define ENABLE_TERM_COLOR	1
#endif

static LoraGatewayListener listener;

static void printTrace() {
    void *t[TRACE_BUFFER_SIZE];
#ifndef WIN32
    size_t size = backtrace(t, TRACE_BUFFER_SIZE);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
#endif
}

void signalHandler(int signal)
{
    switch (signal)
    {
        case SIGINT:
            std::cerr << "Interrupt" << std::endl;
            {
                int r = listener.stop(60);
                if (r)
                    std::cerr << ERR_LORA_GATEWAY_SHUTDOWN_TIMEOUT << std::endl;
                exit(r);
            }
        case SIGSEGV:
            std::cerr << "Segmentation fault" << std::endl;
            printTrace();
            exit(signal);
        default:
            break;
    }
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = &signalHandler;
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGSEGV, &action, nullptr);
}
#endif

void onLog(
    void *listener,
    int level,
    int moduleCode,
    int errorCode,
    const std::string &message
) {
    struct timeval t;
    gettimeofday(&t, nullptr);
    std::cerr << timeval2string(t) << " ";
#ifdef ENABLE_TERM_COLOR
    if (isatty(2))  // if stderr is piped to the file, do not put ANSI color to the file
        std::cerr << "\033[" << logLevelColor(level)  << "m";
#endif
    std::cerr << std::setfill(' ') << std::setw(LOG_LEVEL_FIELD_WIDTH) << std::left << logLevelString(level);
#ifdef ENABLE_TERM_COLOR
    if (isatty(2))
        std::cerr << "\033[0m";
#endif
    std::cerr << message << std::endl;
}

void onUpstream(
    const LoraGatewayListener *listener,
    const SEMTECH_PROTOCOL_METADATA *metadata,
    const std::string &payload
)
{
    struct timeval t;
    gettimeofday(&t, nullptr);
    std::cerr << timeval2string(t) << " ";
#ifdef ENABLE_TERM_COLOR
    if (isatty(2))  // if stderr is piped to the file, do not put ANSI color to the file
        std::cerr << "\033[0;33m";
#endif

#ifdef ENABLE_TERM_COLOR
    if (isatty(2))  // if stderr is piped to the file, do not put ANSI color to the file
        std::cerr << "\033[" << logLevelColor(LOG_INFO)  << "m";
#endif
    std::cerr << std::setw(LOG_LEVEL_FIELD_WIDTH) << std::setfill(' ') << std::left << logLevelString(LOG_INFO);
#ifdef ENABLE_TERM_COLOR
    if (isatty(2))
        std::cerr << "\033[0m";
#endif

    if (metadata) {
        std::cerr << "gatewayId: " << std::hex << metadata->gatewayId
                  << std::dec << " frequency: " << metadata->freq
                  << " CRC status: " << (int) metadata->stat
                  << " modulation: " << (int) metadata->modu
                  << " bandwidth: " << (int) metadata->bandwith
                  << " SF" << (int) metadata->spreadingFactor
                  << " coding rate: " << (int) metadata->codingRate
                  << " bps: " << (int) metadata->bps
                  << " RSSI: " << (int) metadata->rssi
                  << " lsnr: " << metadata->lsnr;

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
            std::cerr
                    << " addr: " << std::hex << std::right << std::setw(8) << std::setfill('0') << addr
                    << " FCnt: " << std::dec << fcnt
                    << " payload: " << hexString(payload) << std::endl;
        }
    }
}

void onSpectralScan(
    const LoraGatewayListener *listener,
    const uint32_t frequency,
    const uint16_t results[LGW_SPECTRAL_SCAN_RESULT_SIZE]
){
    struct timeval t;
    gettimeofday(&t, nullptr);
    std::cerr << timeval2string(t) << " ";
#ifdef ENABLE_TERM_COLOR
    if (isatty(2))  // if stderr is piped to the file, do not put ANSI color to the file
        std::cerr << "\033[0;33m";
#endif

#ifdef ENABLE_TERM_COLOR
    if (isatty(2))  // if stderr is piped to the file, do not put ANSI color to the file
        std::cerr << "\033[" << logLevelColor(LOG_INFO)  << "m";
#endif
    std::cerr << std::setw(LOG_LEVEL_FIELD_WIDTH) << std::setfill(' ') << std::left << logLevelString(LOG_INFO);
#ifdef ENABLE_TERM_COLOR
    if (isatty(2))
        std::cerr << "\033[0m";
#endif

    std::cerr << "frequency " << frequency << std::endl;
    for (int i = 0; i < LGW_SPECTRAL_SCAN_RESULT_SIZE; i++) {
        std::cerr
            << std::dec << std::right << std::setw(7) << std::setfill('0') << results[i]
            << " ";
    }
    std::cerr << std::endl;
}

int main(int argc, char **argv)
{
    setSignalHandler();

    std::string tty;
    if (argc > 1) {
        tty = argv[1];
    }
    GatewayConfigFileJson config;
    const std::string fn("/home/andrei/git/rak_common_for_gateway/lora/rak2287/sx1302_hal/packet_forwarder/global_conf.json");
    std::string v = file2string(fn.c_str());

    int r = config.parseString(v);
    if (!tty.empty())
        config.sx130xConf.setUsbPath(tty);

    if (r) {
        std::cerr << "Error " << r << std::endl;
        std::cerr << "Parse error " << config.errorDescription << " at " << config.errorOffset << std::endl;
        exit(r);
    }

    listener.config = &config;
    std::string devUsb = config.sx130xConf.getUsbPath();

    std::cout << "libloragw " << listener.version() << std::endl;
    std::cout << "Gateway id " << std::hex << config.gatewayConf.value.gatewayId << std::endl;
    std::cout << "USB path " << devUsb << std::endl;

    listener.setLogVerbosity(LOG_DEBUG);
    listener.setOnLog(onLog);
    listener.setOnSpectralScan(onSpectralScan);
    listener.setOnUpstream(onUpstream);
    r = listener.start();

    if (r) {
        std::cerr << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r)
            << " lgw error code: " << listener.lastLgwCode
            << std::endl;

        exit(r);
    }

    std::cout << "Press Ctrl+C to exit" << std::endl
        << "Enter 'r' to run" << std::endl
        << "Enter 'o' to stop" << std::endl
        << "Enter 's' to see statistics" << std::endl
        << "Enter 't' to see status" << std::endl;
    char c;
    while(true) {
        std::cin >> c;
        switch (c) {
            case 'R':
            case 'r': {
                r = listener.start();
                if (r) {
                    std::cerr << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r)
                      << " lgw error code: " << listener.lastLgwCode
                      << std::endl;
                    exit(r);
                } else
                    std::cout << "Started successfully" << std::endl;
            }
            break;
            case 'O':
            case 'o': {
                r = listener.stop(60);
                if (r) {
                    std::cerr << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r)
                      << " lgw error code: " << listener.lastLgwCode
                      << std::endl;
                    exit(r);
                } else
                    std::cout << "Stopped successfully" << std::endl;
            }
                break;
            case 'S':
            case 's':
                {
                    uint32_t m[MEASUREMENT_COUNT_SIZE];
                    listener.measurements.get(m);
                    for (int i = 0; i < MEASUREMENT_COUNT_SIZE; i++) {
                        std::cerr << std::right << std::setfill(' ') << std::setw(6) << getMeasurementName(i);
                    }
                    std::cerr << std::endl;

                    for (int i = 0; i < MEASUREMENT_COUNT_SIZE; i++) {
                        std::cerr << std::dec << std::right << std::setw(6) << std::setfill(' ') << m[i];
                    }
                    std::cerr  << std::endl;
                }
                break;
            case 'T':
            case 't':
                {
                    LGWStatus status;
                    if (listener.getStatus(status)) {
                        std::cerr << "T " << status.temperature << "C "
                            << "SX1302 counter (INST): " << status.inst_tstamp
                            << " SX1302 counter (PPS): " << status.trig_tstamp
                            << std::endl;
                    }
                }
                break;
            default:
                break;
        }
    }
}
