/**
 * Simple LoRaWAN network server.
 * Copyright (c) 2021 {@link mailto:andrey.ivanov@ikfia.ysn.ru} Yu.G. Shafer Institute of Cosmophysical Research
 * and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license {@link file://LICENSE}
 */
#include <iostream>
#include <iomanip>
#include <cstring>

#define ENABLE_LOGGER_HUFFMAN   1

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

#ifdef WIN32
#else
#include <execinfo.h>
#endif

#include "argtable3/argtable3.h"
#include "platform.h"
#include "utilstring.h"
#include "daemonize.h"

#include "errlist.h"
#include "utillora.h"
#include "utilstring.h"
#include "utildate.h"

#include "udp-listener.h"
#include "config-json.h"
#include "lora-packet-handler-impl.h"
#include "identity-service-file-json.h"
#include "identity-service-dir-txt.h"
#include "receiver-queue-service-file-json.h"
#include "receiver-queue-service-dir-txt.h"
#include "regional-parameter-channel-plan-file-json.h"

#ifdef ENABLE_LMDB
#include "identity-service-lmdb.h"
#include "receiver-queue-service-lmdb.h"
#endif

#ifdef ENABLE_LOGGER_HUFFMAN
#include "logger-huffman/logger-parse.h"
#include "logger-loader.h"
#endif

#include "receiver-queue-processor.h"

#include "gateway-list.h"
#include "config-filename.h"

#include "db-any.h"

#include "lorawan-ws/lorawan-ws.h"

#include "device-history-service-json.h"

#include "gateway-stat-service-file.h"
#include "gateway-stat-service-post.h"
#include "device-stat-service-file.h"
#include "device-stat-service-post.h"
#include "device-channel-plan-file-json.h"

const std::string programName = "lorawan-network-server";
#define DEF_CONFIG_FILE_NAME ".lorawan-network-server"
#define DEF_IDENTITY_STORAGE_NAME "identity.json"
#define DEF_QUEUE_STORAGE_NAME "queue.json"
#define DEF_GATEWAYS_STORAGE_NAME "gateway.json"
#define DEF_DEVICE_HISTORY_STORAGE_NAME "device-history.json"
#define DEF_DATABASE_CONFIG_FILE_NAME "dbs.js"
#define DEF_PROTO_PATH "proto"

static int lastSysSignal = 0;
#define MAX_DEVICE_LIST_COUNT   20

// sev service config
WSConfig wsConfig;
static Configuration *config = nullptr;
static GatewayList *gatewayList = nullptr;
static GatewayStatService *gatewayStatService = nullptr;
static DeviceStatService *deviceStatService = nullptr;

// Listen UDP port(s) for packets sent by Semtech's gateway
static UDPListener *listener = nullptr;
// Device identity service
static IdentityService *identityService = nullptr;
// ReceiverQueueProcessor get payload from the queue, parseRX and put parsed data
static ReceiverQueueProcessor *receiverQueueProcessor = nullptr;
// LoraPacketProcessor handles uplink messages
static LoraPacketProcessor *processor = nullptr;
// Database list
static DatabaseByConfig *dbByConfig = nullptr;
// Device counters and last received
static DeviceHistoryService *deviceHistoryService = nullptr;
// Regional settings
static RegionalParameterChannelPlans *regionalParameterChannelPlans = nullptr;
static DeviceChannelPlan *deviceChannelPlan = nullptr;

static const char *TAB = "\t";

// pkt2 environment
#ifdef ENABLE_PKT2
static void* parserEnv = nullptr;
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
static void* loggerParserEnv = nullptr;
#endif

ReceiverQueueService *receiverQueueService = nullptr;

#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#define ENABLE_TERM_COLOR	1
#endif

static void flushFiles()
{
    // save
    // identityService->flush();
    if (receiverQueueService)
		receiverQueueService->flush();
	if (gatewayList)
		gatewayList->save();
	if (deviceHistoryService)
		deviceHistoryService->flush();
    // if (deviceChannelPlan)
    //    deviceChannelPlan->flush();
    // if (deviceChannelPlan)
    //    deviceChannelPlan->flush();
}

static void done()
{
	// destroy and free all
	delete listener;
	listener = nullptr;

	if (config) {
		if(config->serverConfig.verbosity > 1)
			std::cerr << MSG_GRACEFULLY_STOPPED << std::endl;
		delete config;
		config = nullptr;
	}
	if (processor) {
		delete processor;
		processor = nullptr;
	}
	if (receiverQueueProcessor) {
		delete receiverQueueProcessor;
        receiverQueueProcessor = nullptr;
	}

    // save changes
    flushFiles();

    if (receiverQueueService) {
		delete receiverQueueService;
		receiverQueueService = nullptr;
	}
	if (identityService) {
		delete identityService;
		identityService = nullptr;
	}
    if (gatewayStatService) {
        delete gatewayStatService;
        gatewayStatService = nullptr;
    }
    if (deviceStatService) {
        delete deviceStatService;
        deviceStatService = nullptr;
    }
    if (dbByConfig) {
		delete dbByConfig;
		dbByConfig = nullptr;
	}
	if (gatewayList) {
		delete gatewayList;
		gatewayList = nullptr;
	}
	if (deviceHistoryService) {
		delete deviceHistoryService;
        deviceHistoryService = nullptr;
	}
    if (regionalParameterChannelPlans) {
        delete regionalParameterChannelPlans;
        regionalParameterChannelPlans = nullptr;
    }
    if (deviceChannelPlan) {
        delete deviceChannelPlan;
        deviceChannelPlan = nullptr;
    }

#ifdef ENABLE_PKT2
	if (parserEnv)
		donePkt2(parserEnv);
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
    if (loggerParserEnv)
		doneLoggerParser(loggerParserEnv);
#endif
	exit(0);
}

static void stop()
{
	if (listener)
		listener->stopped = true;
}

#define TRACE_BUFFER_SIZE   256

static void printTrace() {
    void *t[TRACE_BUFFER_SIZE];
    size_t size = backtrace(t, TRACE_BUFFER_SIZE);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
}

void signalHandler(int signal)
{
	lastSysSignal = signal;
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
	case SIGHUP:
		std::cerr << ERR_HANGUP_DETECTED << std::endl;
		break;
	case SIGUSR2:	// 12
		std::cerr << MSG_SIG_FLUSH_FILES << std::endl;
		// flushFiles();
		break;
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
	sigaction(SIGHUP, &action, nullptr);
	sigaction(SIGSEGV, &action, nullptr);
    sigaction(SIGABRT, &action, nullptr);
	sigaction(SIGUSR2, &action, nullptr);
}
#endif

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
	Configuration *config,
	int argc,
	char *argv[])
{
	// device path
	struct arg_str *a_address4 = arg_strn(nullptr, nullptr, "<IPv4 address:port>", 0, 8, "listener IPv4 interface e.g. *:8003");
	struct arg_str *a_address6 = arg_strn("6", "ipv6", "<IPv6 address:port>", 0, 8, "listener IPv6 interface e.g. :1700");
	struct arg_str *a_config = arg_str0("c", "config", "<file>",
                                        "configuration file. Default ~/" DEF_CONFIG_FILE_NAME ", identity storage ~/" DEF_IDENTITY_STORAGE_NAME
	", queue storage ~/" DEF_QUEUE_STORAGE_NAME ", gateways ~/" DEF_GATEWAYS_STORAGE_NAME 
	", device history ~/" DEF_DEVICE_HISTORY_STORAGE_NAME
	);
	struct arg_str *a_logfilename = arg_str0("l", "logfile", "<file>", "log file");
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 7, "Set verbosity level 1- alert, 2-critical error, 3- error, 4- warning, 5- siginicant info, 6- info, 7- debug");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void *argtable[] = {
		a_config,
		a_address4, a_address6,
		a_logfilename, a_daemonize, a_verbosity, a_help, a_end};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_PARAM_INVALID;
	}
	// Parse the command line as defined by argtable[]
	int nErrors = arg_parse(argc, argv, argtable);

	if (a_config->count)
		config->configFileName = *a_config->sval;
	else
		config->configFileName = getDefaultConfigFileName(DEF_CONFIG_FILE_NAME);

	config->serverConfig.daemonize = (a_daemonize->count > 0);
	config->serverConfig.verbosity = a_verbosity->count;

	if (!nErrors)
	{
		for (int i = 0; i < a_address4->count; i++)
		{
			config->serverConfig.listenAddressIPv4.push_back(a_address4->sval[i]);
		}
		for (int i = 0; i < a_address6->count; i++)
		{
			config->serverConfig.listenAddressIPv6.push_back(a_address6->sval[i]);
		}
	}

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nErrors)
	{
		if (nErrors)
			arg_print_errors(stderr, a_end, programName.c_str());
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_PARAM_INVALID;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return LORA_OK;
}

void onLog(
	void *env,
	int level,
	int moduleCode,
	int errorCode,
	const std::string &message
)
{
	if (env) {
		if (((UDPListener *) listener)->verbosity < level)
			return;
	}
	struct timeval t;
	gettimeofday(&t, nullptr);
    std::cerr << timeval2string(t) << " ";
#ifdef ENABLE_TERM_COLOR
    if (isatty(2))  // if stderr is piped to the file, do not put ANSI color to the file
        std::cerr << "\033[" << logLevelColor(level)  << "m";
#endif
    std::cerr<< std::setw(LOG_LEVEL_FIELD_WIDTH) << std::left << logLevelString(level);
#ifdef ENABLE_TERM_COLOR
    if (isatty(2))
        std::cerr << "\033[0m";
#endif
    std::cerr << message << std::endl;
}

void onGatewayStatDump(
	void *env,
	GatewayStat *stat
) {
    if (gatewayStatService)
        gatewayStatService->put(stat);
}

void onDeviceStatDump(
	void *env,
	const SemtechUDPPacket &value
) {
    if (deviceStatService)
        deviceStatService->put(&value);
}

static void run()
{
	listener->listen();
}

int main(
	int argc,
	char *argv[])
{
	config = new Configuration("");
	if (parseCmd(config, argc, argv) != 0)
		exit(ERR_CODE_COMMAND_LINE);
	// reload config if required
	bool hasConfig = false;
	if (!config->configFileName.empty()) {
		std::string js = file2string(config->configFileName.c_str());
		if (!js.empty()) {
			config->parse(js.c_str());
			hasConfig = true;
		}
	}
	if (!hasConfig) {
		std::cerr << ERR_NO_CONFIG << std::endl;
		exit(ERR_CODE_NO_CONFIG);
	}

    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_INIT_UDP_LISTENER);
    listener = new UDPListener();
	// check signal number when select() has been interrupted
	listener->setSysSignalPtr(&lastSysSignal);
	listener->setLogger(config->serverConfig.verbosity, onLog);
	listener->setGatewayStatDumper(config, onGatewayStatDump);

	if (config->serverConfig.identityStorageName.empty()) {
		config->serverConfig.identityStorageName = getDefaultConfigFileName(DEF_IDENTITY_STORAGE_NAME);
	}
	if (config->serverConfig.queueStorageName.empty()) {
		config->serverConfig.queueStorageName = getDefaultConfigFileName(DEF_QUEUE_STORAGE_NAME);
	}
	if (config->gatewaysFileName.empty()) {
		config->gatewaysFileName = getDefaultConfigFileName(DEF_GATEWAYS_STORAGE_NAME);
	}
	if (config->serverConfig.deviceHistoryStorageName.empty()) {
		config->serverConfig.deviceHistoryStorageName = getDefaultConfigFileName(DEF_DEVICE_HISTORY_STORAGE_NAME);
	}
    if (config->serverConfig.verbosity > 2) {
        std::cerr << MSG_LISTEN_IP_ADDRESSES << std::endl;
        std::cerr << config->toDescriptionTableString() << std::endl;
    }

	gatewayList = new GatewayList(config->gatewaysFileName);
	
	if (config->serverConfig.verbosity > 2)
		std::cerr << MSG_GATEWAY_LIST << gatewayList->toDescriptionTableString() << std::endl;

    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_IDENTITY_START);
	// Start identity service
	switch (config->serverConfig.storageType) {
		case IDENTITY_STORAGE_LMDB:
#ifdef ENABLE_LMDB
			identityService = new LmdbIdentityService();
#endif
			break;
		case IDENTITY_STORAGE_DIR_TEXT:
			identityService = new DirTxtIdentityService();
			break;
		default:
			identityService = new JsonFileIdentityService();
	}
    // set network identifier
    identityService->setNetworkId(config->serverConfig.netid);

    std::stringstream ss;
    ss << MSG_IDENTITY_INIT  << identityService->getNetworkId()->toString() << "..";
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss.str());
    int rs = identityService->init(config->serverConfig.identityStorageName, nullptr);
    if (rs) {
        std::cerr << ERR_INIT_IDENTITY << rs << ": " << strerror_lorawan_ns(rs)
                  << " " << config->serverConfig.identityStorageName << std::endl;
        exit(ERR_CODE_INIT_IDENTITY);
    }

    // Start gateway statistics service
    switch (config->serverConfig.gwStatStorageType) {
        case GW_STAT_FILE_JSON:
            onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_FILE_START);
            gatewayStatService = new GatewayStatServiceFile();
            break;
        case GW_STAT_POST:
            onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_POST_START);
            gatewayStatService = new GatewayStatServicePost();
            break;
        default:
            gatewayStatService = nullptr;
    }

    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_INIT);
    if (gatewayStatService) {
        rs = gatewayStatService->init(config->serverConfig.logGWStatisticsFileName, nullptr);
        if (rs) {
            std::cerr << ERR_INIT_GW_STAT << rs << ": " << strerror_lorawan_ns(rs)
                      << " " << config->serverConfig.logGWStatisticsFileName << std::endl;
            exit(ERR_CODE_INIT_GW_STAT);
        }
    }

    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_STAT_START);
    // Start device statistics service
    switch (config->serverConfig.deviceStatStorageType) {
        case DEVICE_STAT_FILE_JSON:
            deviceStatService = new DeviceStatServiceFile();
            break;
        case DEVICE_STAT_POST:
            deviceStatService = new DeviceStatServicePost();
            break;
        default:
            deviceStatService = nullptr;
    }

    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_STAT_INIT);
    if (deviceStatService) {
        rs = deviceStatService->init(config->serverConfig.logDeviceStatisticsFileName, nullptr);
        if (rs) {
            std::cerr << ERR_INIT_DEVICE_STAT << rs << ": " << strerror_lorawan_ns(rs)
                      << " " << config->serverConfig.logDeviceStatisticsFileName << std::endl;
            exit(ERR_CODE_INIT_DEVICE_STAT);
        }
    }

    if (config->serverConfig.verbosity > 3) {
		std::vector<NetworkIdentity> identities;
		std::cerr << MSG_DEVICES << std::endl;
		identityService->list(identities, 0, MAX_DEVICE_LIST_COUNT + 1);
        size_t c = 0;
		for (std::vector<NetworkIdentity>::const_iterator it(identities.begin()); it != identities.end(); it++) {
			std::cerr
                << TAB << activation2string(it->activation)
                << TAB << DEVADDR2string(it->devaddr)
                << TAB << DEVEUI2string(it->devEUI)
                << TAB << DEVICENAME2string(it->name);
            if (identityService->canControlService(it->devaddr))
				std::cerr << TAB << "master";
			std::cerr << std::endl;
            if (c > MAX_DEVICE_LIST_COUNT) {
                std::cerr << TAB << ".." << std::endl;
                break;
            }
            c++;
		}
        std::cerr << TAB << identityService->size() << " " << MSG_DEVICE_COUNT << std::endl;
	}

    switch (config->serverConfig.deviceStatStorageType) {
        case DEVICE_STAT_FILE_JSON:
        case DEVICE_STAT_POST:
            listener->setDeviceStatDumper(config, onDeviceStatDump);
            break;
        default:
            break;
    }

    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_HISTORY_START);
    // std::cerr << "Device history name: " << config->serverConfig.deviceHistoryStorageName << std::endl;
    deviceHistoryService = new JsonFileDeviceHistoryService();
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_HISTORY_INIT);
    rs = deviceHistoryService->init(config->serverConfig.deviceHistoryStorageName, nullptr);
    if (rs) {
        std::cerr << ERR_INIT_DEVICE_STAT << rs << ": " << strerror_lorawan_ns(rs)
                  << " " << config->serverConfig.deviceHistoryStorageName << std::endl;
        // That's ok, no problem at all
        // exit(ERR_CODE_INIT_DEVICE_STAT);
    }

    // load regional settings
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_REGIONAL_SET_START);
    regionalParameterChannelPlans = new RegionalParameterChannelPlanFileJson();
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_REGIONAL_SET_INIT
        + config->serverConfig.regionalSettingsStorageName + "..");

    // initialize regional settings
    rs = regionalParameterChannelPlans->init(config->serverConfig.regionalSettingsStorageName, nullptr);
    if (rs) {
        int parseCode;
        std::string parseDescription = regionalParameterChannelPlans->getErrorDescription(parseCode);
        std::cerr << ERR_MESSAGE << ERR_CODE_INIT_REGION_BANDS << ": " << ERR_INIT_REGION_BANDS
            << ", code " << rs << ": " << strerror_lorawan_ns(rs)
                  << ", file: " << config->serverConfig.regionalSettingsStorageName
                  << ", parseRX error " << parseCode << ": " << parseDescription
                  << std::endl;
        exit(ERR_CODE_INIT_REGION_BANDS);
    }
    // initialize regional settings device mapping
    deviceChannelPlan = new DeviceChannelPlanFileJson(regionalParameterChannelPlans);
    if (!config->serverConfig.regionalSettingsChannelPlanName.empty()) {
        // override default regional settings if region name specified
        deviceChannelPlan->setDefaultPlanName(config->serverConfig.regionalSettingsChannelPlanName);
    }

    const RegionalParameterChannelPlan *regionalParameterChannelPlan = deviceChannelPlan->get();
    if (!regionalParameterChannelPlan) {
        std::cerr << ERR_MESSAGE << ERR_CODE_REGION_BAND_NO_DEFAULT << ": " << ERR_REGION_BAND_NO_DEFAULT << std::endl;
        exit(ERR_CODE_REGION_BAND_NO_DEFAULT);
    }

    if (config->serverConfig.verbosity > 3) {
        std::cerr << MSG_REGIONAL_SETTINGS << regionalParameterChannelPlan->toDescriptionTableString() << std::endl;
    }

	// Start received message queue service
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_RECEIVER_QUEUE_START);
	void *options = nullptr;
	DirTxtReceiverQueueServiceOptions dirOptions;
	switch (config->serverConfig.messageQueueType) {
		case MESSAGE_QUEUE_STORAGE_LMDB:
#ifdef ENABLE_LMDB
			//receiverQueueService = new LmdbReceiverQueueService();
#endif			
			break;
		case MESSAGE_QUEUE_STORAGE_DIR_TEXT:
			receiverQueueService = new DirTxtReceiverQueueService();
			dirOptions.format = (DIRTXT_FORMAT) config->serverConfig.messageQueueDirFormat;
			options = (void *) &dirOptions;
			break;
		default:
			receiverQueueService = new JsonFileReceiverQueueService();
			
		break;
	}
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_RECEIVER_QUEUE_INIT);
	rs = receiverQueueService->init(config->serverConfig.queueStorageName, options);
	if (rs) {
		std::cerr << ERR_INIT_QUEUE << rs << ": " << strerror_lorawan_ns(rs)
			<< " " << config->serverConfig.queueStorageName << std::endl;
		// that's ok
		// exit(ERR_CODE_INIT_QUEUE);
	}

	if (config->databaseConfigFileName.empty()) {
		config->databaseConfigFileName = DEF_DATABASE_CONFIG_FILE_NAME;
	}

    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, "Start output database service ..");
	ConfigDatabases configDatabases(config->databaseConfigFileName);
	if (configDatabases.dbs.empty()) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
		// exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	if (config->serverConfig.verbosity > 2)
		std::cerr << MSG_DATABASE_LIST << std::endl;

	// helper class to find out database by name or sequnce number (id)
	dbByConfig = new DatabaseByConfig(&configDatabases);
	// check out database connectivity
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, "Check database availability ..");
	bool dbOk = true;
	for (std::vector<ConfigDatabase>::const_iterator it(configDatabases.dbs.begin()); it != configDatabases.dbs.end(); it++) {
        if (!it->active)
            continue;
		DatabaseNConfig *dc = dbByConfig->find(it->name);
		bool hasConn = dc != nullptr;
        int r = 0;
		if (hasConn) {
            r = dc->open();
            if (r == ERR_CODE_NO_DATABASE) {
                hasConn = false;
            }
            hasConn = hasConn && (r == 0);
        }
		if (config->serverConfig.verbosity > 2) {
			std::cerr << TAB << it->name  << TAB << MSG_CONNECTION
                << (hasConn ? MSG_CONN_ESTABLISHED : MSG_CONN_FAILED);
            if (r)
                std::cerr <<  ": " << strerror_lorawan_ns(r);
            if (dc->db)
                std::cerr << " " << dc->db->errmsg;
            std::cerr << std::endl;
		}
        if (hasConn)
		    dc->close();
		else
			dbOk = false;
	}
	// exit, if it can not connect to the database
	if (!dbOk) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
		exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	// web service
	if (config->wsConfig.enabled) {
		wsConfig.threadCount = config->wsConfig.threadCount;
		wsConfig.connectionLimit = config->wsConfig.connectionLimit;
		wsConfig.flags = config->wsConfig.flags;

		// listener port
		wsConfig.port = config->wsConfig.port;
		// html root
		wsConfig.dirRoot = config->wsConfig.html.c_str();
		// log verbosity
		wsConfig.verbosity = config->serverConfig.verbosity;
		// log callback
		wsConfig.onLog = onLog;

		// databases
		// default database
		bool defDbExists = false;
		if (!config->wsConfig.defaultDatabase.empty()) {
			DatabaseNConfig *dc = dbByConfig->find(config->wsConfig.defaultDatabase);
			bool hasConn = dc != nullptr;
			if (hasConn) {
				int r = dc->open();
				if (!r) {
					wsConfig.databases[""] = dc->db;
					defDbExists = true;
				}
			}
		}
		
		if (!defDbExists)
			std::cerr << ERR_NO_DEFAULT_WS_DATABASE << config->wsConfig.defaultDatabase << std::endl;

		// named databases
		for (std::vector<std::string>::const_iterator it(config->wsConfig.databases.begin()); it != config->wsConfig.databases.end(); it++) {
			DatabaseNConfig *dc = dbByConfig->find(*it);
			bool hasConn = dc != nullptr;
			if (hasConn) {
				int r = dc->open();
				if (!r) {
					wsConfig.databases[*it] = dc->db;
				}
			}
		}

		if (config->serverConfig.verbosity > 2) {
			std::cerr << MSG_WS_START
				<< " threads: " << config->wsConfig.threadCount
				<< ", connections limit: " <<  config->wsConfig.connectionLimit
				<< ", flags: " << config->wsConfig.flags
				<< ", port: " << wsConfig.port
				<< ", html root: " << wsConfig.dirRoot
				<< ", log verbosity: " << wsConfig.verbosity
				<< std::endl;
			std::cerr << MSG_DATABASE_LIST << std::endl;
			for (std::map<std::string, DatabaseIntf *>::const_iterator it(wsConfig.databases.begin()); it != wsConfig.databases.end(); it++) {
				std::string n = it->first;
				if (n.empty())
					n = MSG_DEFAULT_DATABASE;
				std::cerr << TAB << n << std::endl;
			}
			std::cerr << std::endl;
		}

		if (startWS(wsConfig)) {
			if (config->serverConfig.verbosity > 2)
				std::cerr << MSG_WS_START_SUCCESS << std::endl;

		} else {
			std::cerr << ERR_WS_START_FAILED << std::endl;
			exit(ERR_CODE_WS_START_FAILED);
		}
	}

	if (config->protoPath.empty()) {
		// if proto path is not specified, try use default ./proto/ path
		config->protoPath = DEF_PROTO_PATH;
	}

#ifdef ENABLE_PKT2
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, "Initialize payload parser PKT2 ..");
	parserEnv = initPkt2(config->protoPath, 0);
	if (!parserEnv) {
		std::cerr << ERR_LOAD_PROTO << std::endl;
		exit(ERR_CODE_LOAD_PROTO);
	}
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
    DbLoggerKosaPacketsLoader loggerKosaPacketsLoader;
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, "Initialize payload parser logger-huffman..");
	loggerParserEnv = initLoggerParser(config->databaseExtraConfigFileNames, onLog, &loggerKosaPacketsLoader);
	if (!loggerParserEnv) {
		std::cerr << ERR_INIT_LOGGER_HUFFMAN_PARSER << std::endl;
		exit(ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER);
	}
#endif

	// Set up processor
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_PKT2_START);
	processor = new LoraPacketProcessor();
	processor->setLogger(onLog);
	processor->setIdentityService(identityService);
	processor->setGatewayList(gatewayList);
	processor->setReceiverQueueService(receiverQueueService);
    processor->setDeviceHistoryService(deviceHistoryService);
	// FPort number reserved for messages controls network service. 0- no remote control allowed
	processor->reserveFPort(config->serverConfig.controlFPort);
    processor->setDeviceChannelPlan(deviceChannelPlan);

	// Set pkt2 environment
	receiverQueueProcessor = new ReceiverQueueProcessor();
#ifdef ENABLE_PKT2
    receiverQueueProcessor->setParserEnv(parserEnv);
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
    receiverQueueProcessor->setParserEnv(loggerParserEnv);
#endif
	receiverQueueProcessor->setLogger(onLog);

	// Set databases
	receiverQueueProcessor->setDatabaseByConfig(dbByConfig);

    // start processing queue
    processor->setReceiverQueueProcessor(receiverQueueProcessor);
	
	// Set up listener
	listener->setHandler(processor);
	listener->setGatewayList(gatewayList);
	listener->setIdentityService(identityService);
    listener->setDeviceHistoryService(deviceHistoryService);

	if (config->serverConfig.listenAddressIPv4.empty() && config->serverConfig.listenAddressIPv6.empty()) {
			std::cerr << ERR_MESSAGE << ERR_CODE_PARAM_NO_INTERFACE << ": " <<  ERR_PARAM_NO_INTERFACE << std::endl;
			exit(ERR_CODE_PARAM_NO_INTERFACE);
	}
	for (std::vector<std::string>::const_iterator it(config->serverConfig.listenAddressIPv4.begin()); it != config->serverConfig.listenAddressIPv4.end(); it++) {
		if (!listener->add(*it, MODE_FAMILY_HINT_IPV4)) {
			std::cerr << ERR_MESSAGE << ERR_CODE_SOCKET_BIND << ": " <<  ERR_SOCKET_BIND << *it << std::endl;
			exit(ERR_CODE_SOCKET_BIND);
		}
	}
	for (std::vector<std::string>::const_iterator it(config->serverConfig.listenAddressIPv6.begin()); it != config->serverConfig.listenAddressIPv6.end(); it++) {
		if (!listener->add(*it, MODE_FAMILY_HINT_IPV6)) {
			std::cerr << ERR_MESSAGE << ERR_CODE_SOCKET_BIND << ": " <<  ERR_SOCKET_BIND << *it << std::endl;
			exit(ERR_CODE_SOCKET_BIND);
		}
	}

	if (config->serverConfig.daemonize)
	{
        onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_LISTENER_DAEMON_RUN);
		char wd[PATH_MAX];
		std::string progpath = getcwd(wd, PATH_MAX);
		if (config->serverConfig.verbosity > 1)
			std::cerr << MSG_DAEMON_STARTED << progpath << "/" << programName << MSG_DAEMON_STARTED_1 << std::endl;
		OPEN_SYSLOG()
		Daemonize daemonize(programName, progpath, run, stop, done);
		// CLOSE_SYSLOG()
	}
	else
	{
#ifdef _MSC_VER
#else
		setSignalHandler();
#endif
        onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_LISTENER_RUN);
		run();
		done();
	}
	return 0;
}
