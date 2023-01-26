/**
 * Simple LoRaWAN network server.
 * Copyright (c) 2021 {@link mailto:andrey.ivanov@ikfia.ysn.ru} Yu.G. Shafer Institute
 * of Cosmophysical Research * and Aeronomy of Siberian Branch of the Russian Academy
 * of Sciences
 * MIT license {@link file://LICENSE}
 */
#include <iostream>
#include <iomanip>
#include <cstring>
#include <csignal>
#include <climits>

#ifdef _MSC_VER
#else
#include <execinfo.h>
#endif

#include "argtable3/argtable3.h"

#include "run-listener.h"
#include "platform.h"
#include "utilstring.h"
#include "utildate.h"
#include "daemonize.h"
#include "errlist.h"
#include "config-json.h"
#include "config-filename.h"
#include "utilfile.h"
#include "db-any.h"
#include "lorawan-ws/lorawan-ws.h"
#include "auth-file.h"
#include "ws-handler.h"

#ifdef ENABLE_LMDB
#include "identity-service-lmdb.h"
#include "receiver-queue-service-lmdb.h"
#endif

#ifdef ENABLE_LOGGER_HUFFMAN
#include "logger-huffman/logger-parse.h"
#include "logger-loader.h"
#endif


#ifdef ENABLE_LISTENER_USB

#include "libloragw-helper.h"
#include <fcntl.h>

class PosixLibLoragwOpenClose : public LibLoragwOpenClose {
public:
    int openDevice(const char *fileName, int mode) override
    {
        return open(fileName, mode);
    };

    int closeDevice(int fd) override
    {
        return close(fd);
    };
};

#endif

const std::string programName = "lorawan-network-server";
#define DEF_CONFIG_FILE_NAME ".lorawan-network-server"
#define DEF_IDENTITY_STORAGE_NAME "identity.json"
#define DEF_QUEUE_STORAGE_NAME "queue.json"
#define DEF_GATEWAYS_STORAGE_NAME "gateway.json"
#define DEF_DEVICE_HISTORY_STORAGE_NAME "device-history.json"
#define DEF_DATABASE_CONFIG_FILE_NAME "dbs.json"
#define DEF_REGIONAL_PARAMATERS_CONFIG_FILE_NAME "regional-parameters.json"
#define DEF_WS_USER_LIST_FILE_NAME "passwd.json"
#define DEF_WS_HTML_FOLDER "html"

static RunListener *runListener = nullptr;
static int lastSysSignal = 0;

// web service config
static WSConfig wsConfig;
// web service special path
#ifdef ENABLE_WS
static WsSpecialPathHandler *wsSpecialPathHandler = nullptr;
#endif
static AuthUserService *authUserService = nullptr;

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

void stop()
{
    if (runListener)
        runListener->stop();
}

void done()
{
    if (runListener)
        runListener->done();
#ifdef ENABLE_WS
    if (wsSpecialPathHandler) {
        delete wsSpecialPathHandler;
        wsSpecialPathHandler = nullptr;
    }
    if (authUserService) {
        delete authUserService;
        authUserService = nullptr;
    }
#endif
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
#ifndef _MSC_VER
	case SIGHUP:
		std::cerr << ERR_HANGUP_DETECTED << std::endl;
		break;
	case SIGUSR2:	// 12
		std::cerr << MSG_SIG_FLUSH_FILES << std::endl;
		// flushFiles();
		break;
#endif
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
		config->configFileName = getDefaultConfigFileName(argv[0], *a_config->sval);
	else
		config->configFileName = getDefaultConfigFileName(argv[0], DEF_CONFIG_FILE_NAME);

	config->serverConfig.daemonize = (a_daemonize->count > 0);
	config->serverConfig.verbosity = a_verbosity->count;

	if (!nErrors)
	{
		for (int i = 0; i < a_address4->count; i++) {
			config->serverConfig.listenAddressIPv4.push_back(a_address4->sval[i]);
		}
		for (int i = 0; i < a_address6->count; i++) {
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
		std::cerr << MSG_PROG_NAME_NETWORK << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_PARAM_INVALID;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return LORA_OK;
}

class StdErrLog: public LogIntf {
public:
    void onInfo(
        void *env,
        int level,
        int moduleCode,
        int errorCode,
        const std::string &message
    ) override {
        if (runListener && runListener->config && runListener->config->serverConfig.daemonize) {
            SYSLOG(level, message.c_str());
            return;
        }
        if (env) {
            if (((WSConfig *) env)->verbosity < level)
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
    void onConnected(bool connected) override
    {

    }

    void onDisconnected() override
    {

    }

    void onStarted(uint64_t gatewayId, const std::string regionName, size_t regionIndex) override
    {

    }

    void onFinished(const std::string &message) override
    {

    }

    void onValue(Payload &value) override
    {

    }
};

static StdErrLog stdErrLog;

#ifdef ENABLE_WS
#ifdef ENABLE_LOGGER_HUFFMAN
    LoggerHuffmanEnv loggerHuffmanEnv;
#endif
#endif

static void wsRun(char *programPath, Configuration* config) {
#ifdef ENABLE_WS
    wsSpecialPathHandler = new WsSpecialPathHandler();
    wsSpecialPathHandler->configDatabases = runListener->configDatabases;
    wsSpecialPathHandler->regionalParameterChannelPlans = runListener->regionalParameterChannelPlans;
    wsSpecialPathHandler->identityService = runListener->identityService;
    wsSpecialPathHandler->gatewayList = runListener->gatewayList;
    wsSpecialPathHandler->config = config;
    wsSpecialPathHandler->gatewayStatService = runListener->gatewayStatService;
    wsSpecialPathHandler->deviceStatService = runListener->deviceStatService;
    std::string userListFileName = getDefaultConfigFileName(programPath, config->wsConfig.userPasswordListFileName);
    if (!util::fileExists(userListFileName)) {
        runListener->onInfo(runListener->listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_LOAD_WS_PASSWD_NOT_FOUND,
                                ERR_LOAD_WS_PASSWD_NOT_FOUND);
        exit(ERR_CODE_LOAD_WS_PASSWD_NOT_FOUND);
    }
    authUserService = new AuthUserFile(config->wsConfig.jwtIssuer,
        config->wsConfig.jwtSecret, file2string(userListFileName.c_str()));
    wsSpecialPathHandler->authUserService = authUserService;

    wsConfig.onSpecialPathHandler = wsSpecialPathHandler;
#endif

    // databases
    // default database
    bool defDbExists = false;
    if (runListener->dbByConfig && !config->wsConfig.defaultDatabase.empty()) {
        DatabaseNConfig *dc = runListener->dbByConfig->find(config->wsConfig.defaultDatabase);
        bool hasConn = dc != nullptr;
        if (hasConn) {
            int r = dc->open();
            if (!r) {
                wsConfig.databases[""] = dc->db;
                defDbExists = true;
            }
        }
    }

    if (!defDbExists) {
        std::stringstream ss;
        ss << ERR_NO_DEFAULT_WS_DATABASE << config->wsConfig.defaultDatabase        // just warning
            << std::endl;

        runListener->onInfo(runListener->listener, LOG_ERR, LOG_MAIN_FUNC,
            ERR_CODE_NO_DEFAULT_WS_DATABASE, ss.str());
    }

    // named databases
    for (std::vector<std::string>::const_iterator it(config->wsConfig.databases.begin()); it != config->wsConfig.databases.end(); it++) {
        DatabaseNConfig *dc = runListener->dbByConfig->find(*it);
        bool hasConn = dc != nullptr;
        if (hasConn) {
            int r = dc->open();
            if (!r) {
                wsConfig.databases[*it] = dc->db;
            }
        }
    }
#ifdef ENABLE_WS
#ifdef ENABLE_LOGGER_HUFFMAN
    if (wsSpecialPathHandler) {
        bool hasLoggerKosaPacketsLoader = false;
        // set database to load from
        std::string loggerDatabaseName;
        std::map<std::string, std::vector <std::string> >::const_iterator pDb = config->pluginsParams.find("logger-huffman-database-name");
        if (pDb != config->pluginsParams.end()) {
            if (!pDb->second.empty())
                loggerDatabaseName = pDb->second[0];
        }

        if (!loggerDatabaseName.empty()) {
            DatabaseNConfig *kldb = nullptr;
            if (runListener->dbByConfig)
                kldb = runListener->dbByConfig->find(loggerDatabaseName);
            if (kldb) {
                loggerHuffmanEnv.loader.setDatabase(kldb->db);
                int r = kldb->open();
                if (r == ERR_CODE_NO_DATABASE) {
                    hasLoggerKosaPacketsLoader = false;
                } else {
                    hasLoggerKosaPacketsLoader = true;
                }
            }
        }
        if (hasLoggerKosaPacketsLoader) {
            std::stringstream sskldb;
            sskldb << MSG_INIT_LOGGER_HUFFMAN << loggerDatabaseName;
            runListener->onInfo(runListener->listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, sskldb.str());
        } else {
            runListener->onInfo(runListener->listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_INIT_LOGGER_HUFFMAN_DB, ERR_INIT_LOGGER_HUFFMAN_DB);
        }
        void *env = initLoggerParser(config->pluginsParams["logger-huffman-passport"],
           [](void *lenv, int level, int moduleCode, int errorCode, const std::string &message) {
               stdErrLog.onInfo(runListener->listener, level, moduleCode, errorCode, message);
            },
            &loggerHuffmanEnv.loader);
        if (!env) {
            runListener->onInfo(runListener->listener, LOG_ERR, LOG_MAIN_FUNC,
                ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER, ERR_INIT_LOGGER_HUFFMAN_PARSER);
            exit(ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER);
        }
        wsSpecialPathHandler->loggerParser = env;
    }
#endif
#endif

    if (config->serverConfig.verbosity > 5) {
        std::stringstream ss;
        ss << MSG_WS_START
           << "threads: " << config->wsConfig.threadCount
           << ", connections limit: " <<  config->wsConfig.connectionLimit
           << ", flags: " << config->wsConfig.flags
           << ", port: " << wsConfig.port
           << ", html root: " << wsConfig.dirRoot
           << ", log verbosity: " << wsConfig.verbosity
           << ", JWT issuer: " << wsConfig.issuer
           << std::endl;
        runListener->onInfo(runListener->listener, LOG_INFO, LOG_MAIN_FUNC, 0, ss.str());

        std::stringstream ss2;
        ss2 << MSG_DATABASE_LIST << std::endl;
        for (std::map<std::string, DatabaseIntf *>::const_iterator it(wsConfig.databases.begin()); it != wsConfig.databases.end(); it++) {
            std::string n = it->first;
            if (n.empty())
                n = MSG_DEFAULT_DATABASE;
            ss2 << "\t" << n << std::endl;
        }
        ss2 << std::endl;
        runListener->onInfo(runListener->listener, LOG_INFO, LOG_MAIN_FUNC, 0, ss2.str());
    }
#ifdef ENABLE_WS
    if (startWS(wsConfig)) {
        if (config->serverConfig.verbosity > 5)
            runListener->onInfo(runListener->listener, LOG_INFO, LOG_MAIN_FUNC, 0, MSG_WS_START_SUCCESS);
    } else {
        runListener->onInfo(runListener->listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_WS_START_FAILED,
            ERR_WS_START_FAILED);
        exit(ERR_CODE_WS_START_FAILED);
    }
#else
    runListener->onInfo(runListener->listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_WS_START_FAILED,
        ERR_WS_START_FAILED);
#endif
}

static void run()
{
    if (runListener->config->wsConfig.enabled)
        wsRun(nullptr, runListener->config);
    if (runListener && runListener->listener) {
        void *config;
#ifdef ENABLE_LISTENER_USB
        LibLoragwHelper libLoragwHelper;
        libLoragwHelper.bind(&stdErrLog, new PosixLibLoragwOpenClose());
        if (!libLoragwHelper.onOpenClose)
            return;
        // config must of GatewaySettings* type
        config = &runListener->config->gatewayConfig;
#endif
        runListener->start();
#ifdef ENABLE_LISTENER_UDP
        config = nullptr;
        runListener->listener->add(runListener->config->serverConfig.listenAddressIPv4, MODE_FAMILY_HINT_IPV4);
        runListener->listener->add(runListener->config->serverConfig.listenAddressIPv6, MODE_FAMILY_HINT_IPV6);
#endif
        int r = runListener->listener->listen(config, 0);
        if (r) {
            std::stringstream ss;
            ss << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r) << std::endl;
            runListener->onInfo(runListener->listener, LOG_ERR, LOG_MAIN_FUNC, r, ss.str());
        }
#ifdef ENABLE_LISTENER_USB
        delete libLoragwHelper.onOpenClose;
        libLoragwHelper.onOpenClose = nullptr;
#endif
    }
}

static void invalidatePaths(char *programPath, Configuration *config)
{
    // validate paths
    if (!util::fileExists(config->serverConfig.identityStorageName)) {
        if (config->serverConfig.identityStorageName.empty())
            config->serverConfig.identityStorageName = DEF_IDENTITY_STORAGE_NAME;
        config->serverConfig.identityStorageName = getDefaultConfigFileName(programPath, config->serverConfig.identityStorageName);
    }
    if (!util::fileExists(config->serverConfig.queueStorageName)) {
        if (config->serverConfig.queueStorageName.empty())
            config->serverConfig.queueStorageName = DEF_QUEUE_STORAGE_NAME;
        config->serverConfig.queueStorageName = getDefaultConfigFileName(programPath, config->serverConfig.queueStorageName);
    }
    if (!util::fileExists(config->gatewaysFileName)) {
        if (config->gatewaysFileName.empty())
            config->gatewaysFileName = DEF_GATEWAYS_STORAGE_NAME;
        config->gatewaysFileName = getDefaultConfigFileName(programPath, config->gatewaysFileName);
    }
    if (!util::fileExists(config->serverConfig.deviceHistoryStorageName)) {
        if (config->serverConfig.deviceHistoryStorageName.empty())
            config->serverConfig.deviceHistoryStorageName = DEF_DEVICE_HISTORY_STORAGE_NAME;
        config->serverConfig.deviceHistoryStorageName = getDefaultConfigFileName(programPath, config->serverConfig.deviceHistoryStorageName);
    }
    if (!util::fileExists(config->databaseConfigFileName)) {
        if (config->databaseConfigFileName.empty())
            config->databaseConfigFileName = DEF_DATABASE_CONFIG_FILE_NAME;
        config->databaseConfigFileName = getDefaultConfigFileName(programPath, config->databaseConfigFileName);
    }
    if (!config->pluginsPath.empty()) {
        if (!util::fileExists(config->pluginsPath)) {
            config->pluginsPath = getDefaultConfigFileName(programPath, config->pluginsPath);;
        }
    }
    if (!util::fileExists(config->serverConfig.regionalSettingsStorageName)) {
        // if regional parameters file name is not specified, try default regional-parameters.json file
        if (config->serverConfig.regionalSettingsStorageName.empty())
            config->serverConfig.regionalSettingsStorageName = DEF_REGIONAL_PARAMATERS_CONFIG_FILE_NAME;
        config->serverConfig.regionalSettingsStorageName = getDefaultConfigFileName(programPath, config->serverConfig.regionalSettingsStorageName);;
    }
    if (!util::fileExists(config->wsConfig.userPasswordListFileName)) {
        if (config->wsConfig.userPasswordListFileName.empty())
            config->wsConfig.userPasswordListFileName = DEF_WS_USER_LIST_FILE_NAME;
        config->wsConfig.userPasswordListFileName = getDefaultConfigFileName(programPath, config->wsConfig.userPasswordListFileName);;
    }
    if (!util::fileExists(config->wsConfig.html)) {
        if (config->wsConfig.html.empty())
            config->wsConfig.html = DEF_WS_HTML_FOLDER;
        config->wsConfig.html = getDefaultConfigFileName(programPath, config->wsConfig.html);;
    }
}

Configuration config;

int main(
	int argc,
	char *argv[])
{

	if (parseCmd(&config, argc, argv) != 0)
		exit(ERR_CODE_COMMAND_LINE);

	// reload config if required
	bool hasConfig = false;
	if (!config.configFileName.empty()) {
		std::string js = file2string(config.configFileName.c_str());
		if (!js.empty()) {
			config.parse(js.c_str());
			hasConfig = true;
		}
	}
	if (!hasConfig) {
		std::cerr << ERR_NO_CONFIG << std::endl;
		exit(ERR_CODE_NO_CONFIG);
	}
    invalidatePaths(argv[0], &config);

    runListener = new RunListener(&config, &lastSysSignal);

	// web service
	if (config.wsConfig.enabled) {
		wsConfig.threadCount = config.wsConfig.threadCount;
		wsConfig.connectionLimit = config.wsConfig.connectionLimit;
		wsConfig.flags = config.wsConfig.flags;

		// listener port
		wsConfig.port = config.wsConfig.port;
		// html root
		wsConfig.dirRoot = config.wsConfig.html.c_str();
		// log verbosity
		wsConfig.verbosity = config.serverConfig.verbosity;
		// web service log
		wsConfig.onLog = &stdErrLog;
        // JWT
        wsConfig.issuer = config.wsConfig.jwtIssuer;
        wsConfig.secret = config.wsConfig.jwtSecret;
	}

	if (config.serverConfig.daemonize)	{
        runListener->onInfo(runListener->listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_LISTENER_DAEMON_RUN);
        std::string progpath = getCurrentDir();
		if (config.serverConfig.verbosity > 1) {
            std::cerr << MSG_DAEMON_STARTED << progpath << "/" << programName << MSG_DAEMON_STARTED_1 << std::endl;
        }

		OPEN_SYSLOG(programName.c_str())
        // negative verbosity forces syslog
        runListener->config->serverConfig.verbosity = - config.serverConfig.verbosity;

		Daemonize daemonize(programName, progpath, run, stop, done);
        std::cerr << MSG_DAEMON_STOPPED << std::endl;
		CLOSE_SYSLOG()
	} else {
#ifdef _MSC_VER
#else
		setSignalHandler();
#endif
        runListener->onInfo(runListener->listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_LISTENER_RUN);
		run();
		done();
	}
	return 0;
}
