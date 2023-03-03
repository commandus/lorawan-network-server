/**
 * LoRaWAN gateway server skeleton
 * Copyright (c) 2021 {@link mailto:andrey.ivanov@ikfia.ysn.ru} Yu.G. Shafer Institute
 * of Cosmophysical Research * and Aeronomy of Siberian Branch of the Russian Academy
 * of Sciences
 * MIT license {@link file://LICENSE}
 */
#include <iomanip>
#include <iostream>
#include <cstring>
#include <csignal>
#include <climits>

#ifdef _MSC_VER
#else
#include <execinfo.h>
#endif

#include <fcntl.h>

#include "argtable3/argtable3.h"

#include "libloragw-helper.h"

#include "platform.h"
#include "usb-listener.h"
#include "identity-service-file-json.h"
#include "utilstring.h"
#include "utildate.h"
#include "daemonize.h"
#include "errlist.h"
#include "config-filename.h"
#include "utilfile.h"
#include "client-id.h"

// generated gateway regional settings source code
#include "gateway_usb_conf.cpp"

class PosixLibLoragwOpenClose : public LibLoragwOpenClose {
private:
        std::string devicePath;
public:
    
    explicit PosixLibLoragwOpenClose(const std::string &aDevicePath) : devicePath(aDevicePath) {};

    int openDevice(const char *fileName, int mode) override
    {
        return open(devicePath.c_str(), mode);
    };

    int closeDevice(int fd) override
    {
        return close(fd);
    };
};

static std::string getRegionNames()
{
    std::stringstream ss;
    for (size_t i = 0; i < sizeof(memSetupMemGatewaySettingsStorage) / sizeof(setupMemGatewaySettingsStorage); i++) {
        ss << "\"" << memSetupMemGatewaySettingsStorage[i].name << "\" ";
    }
    return ss.str();
}

size_t findRegionIndex(
    const std::string &namePrefix
)
{
    for (size_t i = 0; i < sizeof(memSetupMemGatewaySettingsStorage) / sizeof(setupMemGatewaySettingsStorage); i++) {
        if (memSetupMemGatewaySettingsStorage[i].name.find(namePrefix) != std::string::npos) {
            return i;
        }
    }
    return 0;
}

class GatewayConfigMem : public GatewaySettings {
public:
    MemGatewaySettingsStorage storage;
    sx1261_config_t *sx1261() override { return &storage.sx1261; };
    sx130x_config_t *sx130x() override { return &storage.sx130x; };
    gateway_t *gateway() override { return &storage.gateway; };
    struct lgw_conf_debug_s *debug() override { return &storage.debug; };
    std::string *serverAddress() override { return &storage.serverAddr; };
    std::string *gpsTTYPath() override { return &storage.gpsTtyPath; };;
    void set(MemGatewaySettingsStorage &value) { storage = value;};
};

const std::string programName = "lorawan-gateway";
#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#define ENABLE_TERM_COLOR	1
#endif

/**
 * Handle uplink messages interface
 */
class StdoutLoraPacketHandler : public LoraPacketHandler {
public:
    int ack(
        int socket,
        const sockaddr_in* gwAddress,
        const SEMTECH_PREFIX_GW &dataprefix
    ) override
    {
        return 0;
    };

    // Return 0, retval = EUI and keys
    int put(
        const struct timeval &time,
        SemtechUDPPacket &packet
    ) override
    {
        uint32_t f = 0;
        uint32_t sf = 0;
        int16_t rssi = 0;
        float lsnr = 0.0f;
        if (!packet.metadata.empty()) {
            rfmMetaData &m = packet.metadata[0];
            f = m.freq;
            sf = m.spreadingFactor;
            rssi = m.rssi;
            lsnr = m.lsnr;
        }
        std::cout << time2string(time.tv_sec) << "\t"
            << DEVEUI2string(packet.devId.devEUI) << "\t"
            << DEVICENAME2string(packet.devId.name) << "\t"
            << hexString(packet.payload) << "\t"
            << f << "\t"
            << sf << "\t"
            << rssi << "\t"
            << lsnr << "\t"
            << std::endl;
        return 0;
    }

    int putUnidentified(
            const struct timeval &time,
            SemtechUDPPacket &packet
    ) override
    {
        std::cout << timeval2string(time) << "\t"
            << DEVADDRINT2string(packet.getDeviceAddr()) << "\t"
            << "unknown\t"
            << "undeciphered\t"
            << hexString(packet.payload) << std::endl;
        return 0;
    }

    // Reserve FPort number for network service purposes
    void reserveFPort(
        uint8_t value
    ) override
    {
    }

    int join(
        const struct timeval &time,
        int socket,
        const sockaddr_in *socketAddress,
        SemtechUDPPacket &packet
    ) override
    {
        return 0;
    }
};

class LocalGatewayConfiguration {
public:
    std::string devicePath;
    std::string identityFileName;
    uint64_t gatewayIdentifier;
    size_t regionIdx;
    bool enableSend;
    bool enableBeacon;
    bool daemonize;
    int verbosity;

};

static GatewayConfigMem gwSettings;

GatewaySettings* getGatewayConfig(LocalGatewayConfiguration *config) {
    MemGatewaySettingsStorage settings;
    // set regional settings
    memSetupMemGatewaySettingsStorage[config->regionIdx].setup(settings);
    // set COM port device path, just in case
    strncpy(settings.sx130x.boardConf.com_path, config->devicePath.c_str(), sizeof(settings.sx130x.boardConf.com_path));
    gwSettings.set(settings);
    return &gwSettings;
}

static LocalGatewayConfiguration localConfig;

static PacketListener *listener = nullptr;
static IdentityService *identityService = nullptr;

static void stop()
{
    if (listener)
        listener->clear();
}

static LibLoragwHelper libLoragwHelper;

static void done()
{
    if (libLoragwHelper.onOpenClose) {
        delete libLoragwHelper.onOpenClose;
        libLoragwHelper.onOpenClose = nullptr;
    }
    if (listener) {
        delete listener;
        listener = nullptr;
    }
    if (identityService) {
        delete identityService;
        identityService = nullptr;
    }
}

class StdErrLog: public LogIntf {
public:
    void onConnected(bool connected) override
    {

    }

    void onDisconnected() override
    {

    }

    void onStarted(uint64_t gatewayId, const std::string regionName, size_t regionIndex) override
    {
        std::cout << "gateway " << std::hex << gatewayId
            << " region " << regionName << " (settings #" << std::dec << regionIndex << ")" << std::endl;
    }

    void onFinished(const std::string &message) override
    {

    }

    void onReceive(Payload &value) override
    {
        // std::cerr << hexString(value.payload) << std::endl;
    }

    void onValue(Payload &value) override
    {
        // TODO send to another program or service
        // std::cout << hexString(value.payload) << std::endl;
    }

    void onInfo(
        void *env,
        int level,
        int moduleCode,
        int errorCode,
        const std::string &message
    ) override {
        if (localConfig.daemonize) {
            SYSLOG(level, message.c_str());
            return;
        }
        if (env) {
            if (localConfig.verbosity < level)
                return;
        }
        struct timeval t;
        gettimeofday(&t, nullptr);
        std::cerr << timeval2string(t);
#ifdef ENABLE_TERM_COLOR
        if (isatty(2))  // if stderr is piped to the file, do not put ANSI color to the file
            std::cerr << "\033[" << logLevelColor(level)  << "m";
#endif
        std::cerr << " " << std::setw(LOG_LEVEL_FIELD_WIDTH) << std::left << logLevelString(level);
#ifdef ENABLE_TERM_COLOR
        if (isatty(2))
            std::cerr << "\033[0m";
#endif
        std::cerr << message << std::endl;
        if (level == LOG_ALERT) {
            stop();
        }
    }

    // not used
    int identityGet(DeviceId& deviceid, DEVADDR& addr)
    {
        return 0;
    }

    // not used
    int identityGetNetworkIdentity(NetworkIdentity &retVal, const DEVEUI &eui)
    {
        return 0;
    }

    // not used
    size_t identitySize()
    {
        return 0;
    }
};

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
    LocalGatewayConfiguration *config,
    int argc,
    char *argv[])
{
    // device path
    struct arg_str *a_device_path = arg_str1(nullptr, nullptr, "<device-name>", "USB gateway device e.g. /dev/ttyACM0");
    struct arg_str *a_region_name = arg_str1("c", "region", "<region-name>", "Region name, e.g. \"EU433\" or \"US\"");
    struct arg_str *a_identity_file_name = arg_str0("i", "id", "<id-file-name>", "Device identities JSON file name");
    struct arg_lit *a_enable_send = arg_lit0("s", "allow-send", "Allow send");
    struct arg_lit *a_enable_beacon = arg_lit0("b", "allow-beacon", "Allow send beacon");
    struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "Run as daemon");
    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 7, "Verbosity level 1- alert, 2-critical error, 3- error, 4- warning, 5- siginicant info, 6- info, 7- debug");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_device_path, a_region_name, a_identity_file_name,
        a_enable_send, a_enable_beacon,
        a_daemonize, a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_PARAM_INVALID;
    }
    // Parse the command line as defined by argtable[]
    int nErrors = arg_parse(argc, argv, argtable);

    if (a_device_path->count)
        config->devicePath = std::string(*a_device_path->sval);
    else
        config->devicePath = "";
    config->gatewayIdentifier = 0;
    if (a_identity_file_name->count)
        config->identityFileName = *a_identity_file_name->sval;
    else
        config->identityFileName = "";

    if (a_region_name->count)
        config->regionIdx = findRegionIndex(*a_region_name->sval);
    else
        config->regionIdx = 0;

    config->enableSend = (a_enable_send->count > 0);
    config->enableBeacon = (a_enable_beacon->count > 0);

    config->daemonize = (a_daemonize->count > 0);
    config->verbosity = a_verbosity->count;

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nErrors) {
        if (nErrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << "Usage: " << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr << MSG_PROG_NAME_GATEWAY_USB << std::endl;
        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        std::cerr << "  region name: "
            << getRegionNames() << std::endl;
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_PARAM_INVALID;
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return LORA_OK;
}

#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#define ENABLE_TERM_COLOR	1
#endif

#define TRACE_BUFFER_SIZE   256

static void printTrace() {
#ifdef _MSC_VER
#else
    void *t[TRACE_BUFFER_SIZE];
    size_t size = backtrace(t, TRACE_BUFFER_SIZE);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
#endif
}

static StdErrLog errLog;

static void init();
static void run();

void signalHandler(int signal)
{
    // lastSysSignal = signal;
    switch (signal)
    {
        case SIGINT:
            std::cerr << MSG_INTERRUPTED << std::endl;
            stop();
            done();
            break;
        case SIGSEGV:
            std::cerr << ERR_SEGMENTATION_FAULT << std::endl;
            printTrace();
            exit(ERR_CODE_SEGMENTATION_FAULT);
        case SIGABRT:
            std::cerr << ERR_ABRT << std::endl;
            printTrace();
            exit(ERR_CODE_ABRT);
#ifndef _MSC_VER
        case SIGHUP:
            std::cerr << ERR_HANGUP_DETECTED << std::endl;
            break;
        case SIGUSR2:	// 12
            std::cerr << MSG_SIG_FLUSH_FILES << std::endl;
            // flushFiles();
            break;
#endif
        case 42:	// restart
            std::cerr << MSG_RESTART_REQUEST << std::endl;
            stop();
            done();
            init();
            run();
            break;
        default:
            break;
    }
}

void setSignalHandler()
{
#ifndef _MSC_VER
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = &signalHandler;
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGHUP, &action, nullptr);
    sigaction(SIGSEGV, &action, nullptr);
    sigaction(SIGABRT, &action, nullptr);
    sigaction(SIGUSR2, &action, nullptr);
    sigaction(42, &action, nullptr);
#endif
}

static void run()
{
    if (!localConfig.daemonize)
        setSignalHandler();

    if (listener->onLog)
        listener->onLog->onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_LISTENER_RUN);

    libLoragwHelper.bind(&errLog, new PosixLibLoragwOpenClose(localConfig.devicePath));
    if (!libLoragwHelper.onOpenClose)
        return;

    int flags = 0;
    if (!localConfig.enableSend)
        flags |= FLAG_GATEWAY_LISTENER_NO_SEND;
    if (!localConfig.enableBeacon)
        flags |= FLAG_GATEWAY_LISTENER_NO_BEACON;

    int r = listener->listen(memSetupMemGatewaySettingsStorage[localConfig.regionIdx].name,
                             localConfig.regionIdx, getGatewayConfig(&localConfig), flags, nullptr);
    if (r && listener->onLog) {
        std::stringstream ss;
        ss << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r) << std::endl;
        listener->onLog->onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, r, ss.str());
    }
}

static StdoutLoraPacketHandler packetHandler;

static void init()
{
    identityService = new JsonFileIdentityService();
    if (!identityService) {
        std::cerr << ERR_MESSAGE << ERR_CODE_FAIL_IDENTITY_SERVICE << ": " << ERR_FAIL_IDENTITY_SERVICE << std::endl;
        exit(ERR_CODE_INSUFFICIENT_MEMORY);
    }

    if (localConfig.identityFileName.empty()) {
        // std::cerr << ERR_WARNING << ERR_CODE_INIT_IDENTITY << ": " << ERR_INIT_IDENTITY << std::endl;
        if (localConfig.verbosity > 0)
            std::cerr << MSG_NO_IDENTITIES << std::endl;
    } else {
        if (int r = identityService->init(localConfig.identityFileName, nullptr) != 0) {
            std::cerr << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r) << std::endl;
            exit(ERR_CODE_NO_CONFIG);
        }
    }

    libLoragwHelper.bind(&errLog, nullptr);

    listener = new USBListener();
    if (!listener) {
        std::cerr << ERR_MESSAGE << ERR_CODE_FAIL_IDENTITY_SERVICE << ": " << ERR_FAIL_IDENTITY_SERVICE << std::endl;
        exit(ERR_CODE_INSUFFICIENT_MEMORY);
    }

    // signal is not required in USB listener
    // listener->setSysSignalPtr(&lastSysSignal);
    listener->setLogger(localConfig.verbosity, &errLog);
    listener->setHandler(&packetHandler);
    listener->setIdentityService(identityService);
    ((USBListener*) listener)->listener.setOnStop(
        [] (const LoraGatewayListener *lsnr,
            bool gracefullyStopped
        ) {
            if (!gracefullyStopped) {
                // wait until all threads done
                int seconds2wait = 0;
                while(!lsnr->isStopped() && seconds2wait < 60)
                {
                    std::cerr << ".";
                    sleep(1);
                    seconds2wait++;
                }
            }
        }
    );
}

int main(
	int argc,
	char *argv[])
{
    if (parseCmd(&localConfig, argc, argv) != 0) {
        // std::cerr << ERR_MESSAGE << ERR_CODE_COMMAND_LINE << ": " << ERR_COMMAND_LINE << std::endl;
        exit(ERR_CODE_COMMAND_LINE);
    }

    if (!localConfig.gatewayIdentifier) {
        // std::cerr << ERR_WARNING << ERR_CODE_INVALID_GATEWAY_ID << ": " << ERR_INVALID_GATEWAY_ID << std::endl;
        localConfig.gatewayIdentifier = getGatewayId();
        if (localConfig.verbosity > 0)
            std::cerr << "gateway id " << std::hex << localConfig.gatewayIdentifier << std::dec << std::endl;
    }

    init();

    if (localConfig.daemonize)	{
        if (listener->onLog)
            listener->onLog->onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_LISTENER_DAEMON_RUN);
        std::string progpath = getCurrentDir();
        if (localConfig.verbosity > 1) {
            std::cerr << MSG_DAEMON_STARTED << progpath << "/" << programName << MSG_DAEMON_STARTED_1 << std::endl;
        }

        OPEN_SYSLOG(programName.c_str())
        Daemonize daemonize(programName, progpath, run, stop, done);
        std::cerr << MSG_DAEMON_STOPPED << std::endl;
        CLOSE_SYSLOG()
    } else {
        run();
        done();
    }
    return 0;
}
