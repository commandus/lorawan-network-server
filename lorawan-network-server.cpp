/**
 * Simple LoRaWAN network server.
 * Copyright (c) 2021 andrey.ivanov@ikfia.ysn.ru Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <iostream>
#include <cstring>
#include <fstream>

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>

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

const std::string progname = "lorawan-network-server";
#define DEF_CONFIG_FILE_NAME ".lorawan-network-server"
#define DEF_IDENTITY_STORAGE_NAME "identity.json"
#define DEF_QUEUE_STORAGE_NAME "queue.js"
#define DEF_GATEWAYS_STORAGE_NAME "gateway.json"
#define DEF_DATABASE_CONFIG_FILE_NAME "dbs.js"
#define DEF_PROTO_PATH "proto"

#define DEF_TIME_FORMAT "%FT%T"

#define DEF_BUFFER_SIZE 4096
#define DEF_BUFFER_SIZE_S "4096"

static Configuration *config = NULL;
static GatewayList *gatewayList = NULL;

static UDPListener *listener = NULL;
static IdentityService *identityService = NULL;
static RecieverQueueProcessor *recieverQueueProcessor = NULL;
static LoraPacketProcessor *processor = NULL;
static DatabaseByConfig *dbByConfig = NULL;

// pkt2 environment
static void* pkt2env = NULL;

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
	if (recieverQueueProcessor) {
		delete recieverQueueProcessor;
		recieverQueueProcessor = NULL;
	}
	if (identityService) {
		delete identityService;
		identityService = NULL;
	}
	if (dbByConfig) {
		delete dbByConfig;
		dbByConfig = NULL;
	}
	if (gatewayList) {
		gatewayList->save();
		delete gatewayList;
		gatewayList = NULL;
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
	switch (signal)
	{
	case SIGINT:
		std::cerr << MSG_INTERRUPTED << std::endl;
		stop();
		done();
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
	 "configuration file. Default ~/" DEF_CONFIG_FILE_NAME ", identity storage ~/" DEF_IDENTITY_STORAGE_NAME ", queue storage ~/" DEF_QUEUE_STORAGE_NAME ", gateways ~/" DEF_GATEWAYS_STORAGE_NAME );
	struct arg_str *a_logfilename = arg_str0("l", "logfile", "<file>", "log file");
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 7, "Set verbosity level 1- alert, 2-critical error, 3- error, 4- warning, 5- siginicant info, 6- info, 7- debug");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void *argtable[] = {
		a_config,
		a_address4, a_address6,
		a_logfilename, a_daemonize, a_verbosity, a_help, a_end};

	int nerrors;

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}
	// Parse the command line as defined by argtable[]
	nerrors = arg_parse(argc, argv, argtable);

	if (a_config->count)
		config->configFileName = *a_config->sval;
	else
		config->configFileName = getDefaultConfigFileName(DEF_CONFIG_FILE_NAME);

	config->serverConfig.daemonize = (a_daemonize->count > 0);
	config->serverConfig.verbosity = a_verbosity->count;

	if (!nerrors)
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
	if ((a_help->count) || nerrors)
	{
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
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
	std::cerr << time2string(time(NULL)) << " " << message << std::endl;
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

	listener->setLogger(config->serverConfig.verbosity, onLog);

	if (config->serverConfig.identityStorageName.empty()) {
		config->serverConfig.identityStorageName = getDefaultConfigFileName(DEF_IDENTITY_STORAGE_NAME);
	}
	if (config->serverConfig.queueStorageName.empty()) {
		config->serverConfig.queueStorageName = getDefaultConfigFileName(DEF_QUEUE_STORAGE_NAME);
	}
	if (config->gatewaysFileName.empty()) {
		config->gatewaysFileName = getDefaultConfigFileName(DEF_GATEWAYS_STORAGE_NAME);
	}
	std::cerr << config->toString() << std::endl;

	gatewayList = new GatewayList(config->gatewaysFileName);
	
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
	identityService->init(config->serverConfig.identityStorageName, NULL);

	// Start recived message queue service
	ReceiverQueueService *receiverQueueService;

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
	receiverQueueService->init(config->serverConfig.queueStorageName, options);
	
	if (config->databaseConfigFileName.empty()) {
		config->databaseConfigFileName = DEF_DATABASE_CONFIG_FILE_NAME;
	}

	ConfigDatabases configDatabases(config->databaseConfigFileName);
	if (configDatabases.dbs.size() == 0) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
		exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	if (config->serverConfig.verbosity > 2)
		std::cerr << MSG_DATABASE_LIST << std::endl;

	// heloer class to find out database by name or sequnce number (id)
	dbByConfig = new DatabaseByConfig(&configDatabases);
	// check out database connectivity
	bool dbOk = true;
	for (std::vector<ConfigDatabase>::const_iterator it(configDatabases.dbs.begin()); it != configDatabases.dbs.end(); it++) {
		DatabaseNConfig *dc = dbByConfig->find(it->name);
		bool hasConn;
		if (!dc) {
			dbOk = false;
			hasConn = false;
		} else {
			hasConn = true;
		}
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
		if (!hasConn) {
			dbOk = false;
		}
	}
	// exit, if can not connected to the database
	if (!dbOk) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
		exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}

	if (config->protoPath.empty()) {
		// if proto path is not specidfied, try use default ./proto/ path
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
	processor->setReceiverQueueService(receiverQueueService);

	// Set pkt2 environment
	recieverQueueProcessor = new RecieverQueueProcessor();
	recieverQueueProcessor->setPkt2Env(pkt2env);
	recieverQueueProcessor->setLogger(onLog);
	// Set databases
	recieverQueueProcessor->setDatabaseByConfig(dbByConfig);
	// start processing queue
	processor->setRecieverQueueProcessor(recieverQueueProcessor);
	
	// Set up listener
	listener->setGatewayList(gatewayList);
	listener->setHandler(processor);
	listener->setIdentityService(identityService);

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
			std::cerr << MSG_DAEMON_STARTED << progpath << "/" << progname << MSG_DAEMON_STARTED_1 << std::endl;
		OPENSYSLOG()
		Daemonize daemonize(progname, progpath, run, stop, done);
		// CLOSESYSLOG()
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
