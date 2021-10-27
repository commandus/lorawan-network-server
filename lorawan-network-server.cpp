/**
 * Simple LoRaWAN network server.
 * Copyright (c) 2021 andrey.ivanov@ikfia.ysn.ru Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <iostream>
#include <iomanip>
#include <cstring>
#include <fstream>

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>

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

#ifdef ENABLE_LMDB
#include "identity-service-lmdb.h"
#include "receiver-queue-service-lmdb.h"
#endif			

#include "receiver-queue-processor.h"

#include "gateway-list.h"
#include "config-filename.h"
#include "pkt2/database-config.h"
#include "pkt2/str-pkt2.h"
#include "db-any.h"

#include "device-history-service-json.h"

#include "gateway-stat-service-file.h"
#include "gateway-stat-service-post.h"
#include "device-stat-service-file.h"
#include "device-stat-service-post.h"

const std::string programName = "lorawan-network-server";
#define DEF_CONFIG_FILE_NAME ".lorawan-network-server"
#define DEF_IDENTITY_STORAGE_NAME "identity.json"
#define DEF_QUEUE_STORAGE_NAME "queue.json"
#define DEF_GATEWAYS_STORAGE_NAME "gateway.json"
#define DEF_DEVICE_HISTORY_STORAGE_NAME "device-history.json"
#define DEF_DATABASE_CONFIG_FILE_NAME "dbs.js"
#define DEF_PROTO_PATH "proto"

static int lastSysSignal = 0;

static Configuration *config = NULL;
static GatewayList *gatewayList = NULL;
static GatewayStatService *gatewayStatService = NULL;
static DeviceStatService *deviceStatService = NULL;

// Listen UDP port(s) for packets sent by Semtech's gateway
static UDPListener *listener = NULL;
// Device identity service
static IdentityService *identityService = NULL;
// ReceiverQueueProcessor get payload from the queue, parse and put parsed data
static ReceiverQueueProcessor *receiverQueueProcessor = NULL;
// LoraPacketProcessor handles uplink messages
static LoraPacketProcessor *processor = NULL;
// Database list
static DatabaseByConfig *dbByConfig = NULL;
// Device counters and last received
static DeviceHistoryService *deviceHistoryService = NULL;

// pkt2 environment
static void* pkt2env = NULL;

ReceiverQueueService *receiverQueueService = NULL;

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
}

static void done()
{
	// destroy and free all
	delete listener;
	listener = NULL;

	if (config) {
		if(config->serverConfig.verbosity > 1)
			std::cerr << MSG_GRACEFULLY_STOPPED << std::endl;
		delete config;
		config = NULL;
	}
	if (processor) {
		delete processor;
		processor = NULL;
	}
	if (receiverQueueProcessor) {
		delete receiverQueueProcessor;
        receiverQueueProcessor = NULL;
	}

    // save changes
    flushFiles();

    if (receiverQueueService) {
		delete receiverQueueService;
		receiverQueueService = NULL;
	}
	if (identityService) {
		delete identityService;
		identityService = NULL;
	}
    if (gatewayStatService) {
        delete gatewayStatService;
        gatewayStatService = NULL;
    }
    if (deviceStatService) {
        delete deviceStatService;
        deviceStatService = NULL;
    }
    if (dbByConfig) {
		delete dbByConfig;
		dbByConfig = NULL;
	}
	if (gatewayList) {
		delete gatewayList;
		gatewayList = NULL;
	}
	if (deviceHistoryService) {
		delete deviceHistoryService;
        deviceHistoryService = NULL;
	}
	if (pkt2env)
		donePkt2(pkt2env);

	exit(0);
}

static void stop()
{
	if (listener)
		listener->stopped = true;
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
    {
		std::cerr << ERR_SEGMENTATION_FAULT << std::endl;
		void *t[256];
		size_t size = backtrace(t, 256);
		backtrace_symbols_fd(t, size, STDERR_FILENO);
		exit(ERR_CODE_SEGMENTATION_FAULT);
    }
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
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGUSR2, &action, NULL);
	
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
	struct arg_str *a_address4 = arg_strn(NULL, NULL, "<IPv4 address:port>", 0, 8, "listener IPv4 interface e.g. *:8003");
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
	int modulecode,
	int errorcode,
	const std::string &message
)
{
	if (env) {
		if (((UDPListener *) listener)->verbosity < level)
			return;
	}
	struct timeval t;
	gettimeofday(&t, NULL);
	// "\033[1;31mbold red text\033[0m\n";
	std::cerr << timeval2string(t) << " " 
#ifdef ENABLE_TERM_COLOR
		<< "\033[" << logLevelColor(level)  << "m"
#endif		
		<< std::setw(9) << std::left
		<< logLevelString(level) 
#ifdef ENABLE_TERM_COLOR
		<< "\033[0m"
#endif		
		<< message << std::endl;
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
	{
		exit(ERR_CODE_COMMAND_LINE);
	}
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
	std::cerr << config->toString() << std::endl;

	gatewayList = new GatewayList(config->gatewaysFileName);
	
	if (config->serverConfig.verbosity > 2)
		std::cerr << gatewayList->toJsonString() << std::endl;

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

    int rs = identityService->init(config->serverConfig.identityStorageName, NULL);
    if (rs) {
        std::cerr << ERR_INIT_IDENTITY << rs << ": " << strerror_lorawan_ns(rs)
                  << " " << config->serverConfig.identityStorageName << std::endl;
        exit(ERR_CODE_INIT_IDENTITY);
    }

    // Start gateway statistics service
    switch (config->serverConfig.gwStatStorageType) {
        case GW_STAT_FILE_JSON:
            gatewayStatService = new GatewayStatServiceFile();
            break;
        case GW_STAT_POST:
            gatewayStatService = new GatewayStatServicePost();
            break;
        default:
            gatewayStatService = NULL;
    }

    if (gatewayStatService) {
        rs = gatewayStatService->init(config->serverConfig.logGWStatisticsFileName, NULL);
        if (rs) {
            std::cerr << ERR_INIT_GW_STAT << rs << ": " << strerror_lorawan_ns(rs)
                      << " " << config->serverConfig.logGWStatisticsFileName << std::endl;
            exit(ERR_CODE_INIT_GW_STAT);
        }
    }

    // Start device statistics service
    switch (config->serverConfig.deviceStatStorageType) {
        case DEVICE_STAT_FILE_JSON:
            deviceStatService = new DeviceStatServiceFile();
            break;
        case DEVICE_STAT_POST:
            deviceStatService = new DeviceStatServicePost();
            break;
        default:
            deviceStatService = NULL;
    }

    if (deviceStatService) {
        rs = deviceStatService->init(config->serverConfig.logDeviceStatisticsFileName, NULL);
        if (rs) {
            std::cerr << ERR_INIT_DEVICE_STAT << rs << ": " << strerror_lorawan_ns(rs)
                      << " " << config->serverConfig.logDeviceStatisticsFileName << std::endl;
            exit(ERR_CODE_INIT_DEVICE_STAT);
        }
    }

    if (config->serverConfig.verbosity > 3) {
		std::vector<NetworkIdentity> identities;
		std::cerr << MSG_DEVICES << std::endl;
		identityService->list(identities, 0, 0);
		for (std::vector<NetworkIdentity>::const_iterator it(identities.begin()); it != identities.end(); it++) {
			std::cerr << "\t" << DEVADDR2string(it->devaddr)
                << "\t" << DEVEUI2string(it->deviceEUI)
                << "\t" << DEVICENAME2string(it->name);
            if (identityService->canControlService(it->devaddr))
				std::cerr << "\tmaster";
			std::cerr << std::endl;
		}
	}

    switch (config->serverConfig.deviceStatStorageType) {
        case DEVICE_STAT_FILE_JSON:
        case DEVICE_STAT_POST:
            listener->setDeviceStatDumper(config, onDeviceStatDump);
            break;
        default:
            break;
    }

    // std::cerr << "Device history name: " << config->serverConfig.deviceHistoryStorageName << std::endl;
    deviceHistoryService = new JsonFileDeviceHistoryService();
    rs = deviceHistoryService->init(config->serverConfig.deviceHistoryStorageName, NULL);
    if (rs) {
        std::cerr << ERR_INIT_DEVICE_STAT << rs << ": " << strerror_lorawan_ns(rs)
                  << " " << config->serverConfig.deviceHistoryStorageName << std::endl;
        // That's ok, no problem at all
        // exit(ERR_CODE_INIT_DEVICE_STAT);
    }

	// Start received message queue service
	void *options = NULL;
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

	ConfigDatabases configDatabases(config->databaseConfigFileName);
	if (configDatabases.dbs.size() == 0) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
		// exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	if (config->serverConfig.verbosity > 2)
		std::cerr << MSG_DATABASE_LIST << std::endl;

	// heloer class to find out database by name or sequnce number (id)
	dbByConfig = new DatabaseByConfig(&configDatabases);
	// check out database connectivity
	bool dbOk = true;
	for (std::vector<ConfigDatabase>::const_iterator it(configDatabases.dbs.begin()); it != configDatabases.dbs.end(); it++) {
        if (!it->active)
            continue;
		DatabaseNConfig *dc = dbByConfig->find(it->name);
		bool hasConn = dc != NULL;
		if (!dc)
			dbOk = false;
		int r = dc->open();
		hasConn = hasConn && (r == 0);
		if (config->serverConfig.verbosity > 2) {
			std::cerr << "\t" << it->name  << "\t " << MSG_CONNECTION << (hasConn?MSG_CONN_ESTABLISHED : MSG_CONN_FAILED) << std::endl;
			if (r) {
				if (dc->db)
					std::cerr << dc->db->errmsg << std::endl;
			}
		}
		dc->close();
		if (!hasConn)
			dbOk = false;
	}
	// exit, if can not connected to the database
	if (!dbOk) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
		exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	if (config->protoPath.empty()) {
		// if proto path is not specified, try use default ./proto/ path
		config->protoPath = DEF_PROTO_PATH;
	}

	pkt2env = initPkt2(config->protoPath, 0);
	if (!pkt2env) {
		std::cerr << ERR_LOAD_PROTO << std::endl;
		exit(ERR_CODE_LOAD_PROTO);
	}

	// Set up processor
	processor = new LoraPacketProcessor();
	processor->setLogger(onLog);
	processor->setIdentityService(identityService);
	processor->setGatewayList(gatewayList);
	processor->setReceiverQueueService(receiverQueueService);
    processor->setDeviceHistoryService(deviceHistoryService);
	// FPort number reserved for messages controls network service. 0- no remote control allowed
	processor->reserveFPort(config->serverConfig.controlFPort);

	// Set pkt2 environment
	receiverQueueProcessor = new ReceiverQueueProcessor();
	receiverQueueProcessor->setPkt2Env(pkt2env);
	receiverQueueProcessor->setLogger(onLog);

	// Set databases
	receiverQueueProcessor->setDatabaseByConfig(dbByConfig);
	// start processing queue
	processor->setRecieverQueueProcessor(receiverQueueProcessor);
	
	// Set up listener
	listener->setHandler(processor);
	listener->setGatewayList(gatewayList);
	listener->setIdentityService(identityService);
    listener->setDeviceHistoryService(deviceHistoryService);

	if (config->serverConfig.listenAddressIPv4.size() == 0 && config->serverConfig.listenAddressIPv6.size() == 0) {
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
		run();
		done();
	}
	return 0;
}
