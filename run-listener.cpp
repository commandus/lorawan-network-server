#include <iostream>
#include <iomanip>
#include "run-listener.h"
#include "utildate.h"
#include "errlist.h"
#include "identity-service-dir-txt.h"
#include "gateway-stat-service-file.h"
#include "gateway-stat-service-post.h"
#include "device-stat-service-file.h"
#include "device-stat-service-post.h"
#include "device-history-service-json.h"
#include "regional-parameter-channel-plan-file-json.h"
#include "device-channel-plan-file-json.h"
#include "receiver-queue-service-dir-txt.h"
#include "receiver-queue-service-file-json.h"

#ifdef ENABLE_LOGGER_HUFFMAN
#include "logger-huffman/logger-parse.h"
#include "logger-loader.h"
#include "database-config-json.h"

#endif

static const char *TAB = "\t";
#define MAX_DEVICE_LIST_COUNT   20

RunListener::RunListener()
    : config(nullptr),
    gatewayList(nullptr), gatewayStatService(nullptr),
    deviceStatService(nullptr), listener(nullptr),
    identityService(nullptr), receiverQueueProcessor(nullptr),
    processor(nullptr), dbByConfig(nullptr),
    deviceHistoryService(nullptr), regionalParameterChannelPlans(nullptr),
    deviceChannelPlan(nullptr), configDatabases(nullptr),
#ifdef ENABLE_PKT2
    parserEnv(nullptr),
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
    loggerParserEnv(nullptr),
#endif
    receiverQueueService(nullptr)
{
}

RunListener::RunListener(Configuration *aConfig, int *lastSysSignal)
    : gatewayList(nullptr), gatewayStatService(nullptr),
    deviceStatService(nullptr), listener(nullptr),
    identityService(nullptr), receiverQueueProcessor(nullptr),
    processor(nullptr), dbByConfig(nullptr),
    deviceHistoryService(nullptr), regionalParameterChannelPlans(nullptr),
    deviceChannelPlan(nullptr), configDatabases(nullptr),
#ifdef ENABLE_PKT2
    parserEnv(nullptr),
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
    loggerParserEnv(nullptr),
#endif
    receiverQueueService(nullptr)
{
    init(aConfig, lastSysSignal);
}

void RunListener::flushFiles()
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
    // if (configDatabases)
    //    configDatabases->flush();
    if (deviceStatService)
        deviceStatService->flush();
}

void RunListener::listenerOnLog(
    void *env,
    int level,
    int moduleCode,
    int errorCode,
    const std::string &message
)
{
    if (env) {
        UDPListener *l = (UDPListener *) env;
        if (l) {
            if (l->verbosity < 0) {
                SYSLOG(level, message.c_str());
                return;
            }
            if (l->verbosity < level)
                return;
        }
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

void RunListener::done()
{
    if (config && config->serverConfig.verbosity > 5)
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, MSG_GRACEFULLY_STOPPED);
    // destroy and free all
    delete listener;
    listener = nullptr;
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
    if (configDatabases) {
        delete configDatabases;
        configDatabases = nullptr;
    }
#ifdef ENABLE_PKT2
    if (parserEnv)
		donePkt2(parserEnv);
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
    if (loggerParserEnv) {
        doneLoggerParser(loggerParserEnv);
        loggerParserEnv = nullptr;
    }
#endif
    if (config) {
        delete config;
        config = nullptr;
    }
    exit(0);
}

void RunListener::stop()
{
    if (listener)
        listener->stopped = true;
}

void RunListener::onGatewayStatDump(
    void *env,
    GatewayStat *stat
) {
    if (gatewayStatService)
        gatewayStatService->put(stat);
}

void RunListener::onDeviceStatDump(
    void *env,
    const SemtechUDPPacket &value
) {
    if (deviceStatService)
        deviceStatService->put(&value);
}

void RunListener::init(
    Configuration *aConfig,
    int *lastSysSignal
) {
    config = aConfig;
    listener = new UDPListener();
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_INIT_UDP_LISTENER);
    // check signal number when select() has been interrupted
    listener->setSysSignalPtr(lastSysSignal);
    listener->setLogger(config->serverConfig.verbosity, listenerOnLog);
    listener->setGatewayStatDumper(config, [this](void *env, GatewayStat *stat) { return onGatewayStatDump(env, stat); } );

    if (config->serverConfig.verbosity > 5) {
        std::stringstream ss;
        ss << MSG_LISTEN_IP_ADDRESSES << std::endl << config->toDescriptionTableString();
        listenerOnLog(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }

    gatewayList = new GatewayList(config->gatewaysFileName);

    if (config->serverConfig.verbosity > 5) {
        std::stringstream ss;
        ss << MSG_GATEWAY_LIST << std::endl << gatewayList->toDescriptionTableString();
        listenerOnLog(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }

    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_IDENTITY_START);
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
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss.str());
    int rs = identityService->init(config->serverConfig.identityStorageName, nullptr);
    if (rs) {
        std::stringstream ss;
        ss << ERR_INIT_IDENTITY << rs << ": " << strerror_lorawan_ns(rs)
           << " " << config->serverConfig.identityStorageName << std::endl;
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
        exit(ERR_CODE_INIT_IDENTITY);
    }

    // Start gateway statistics service
    switch (config->serverConfig.gwStatStorageType) {
        case GW_STAT_FILE_JSON:
            listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_FILE_START);
            gatewayStatService = new GatewayStatServiceFile();
            break;
        case GW_STAT_POST:
            listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_POST_START);
            gatewayStatService = new GatewayStatServicePost();
            break;
        default:
            gatewayStatService = nullptr;
    }

    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_INIT);
    if (gatewayStatService) {
        rs = gatewayStatService->init(config->serverConfig.logGWStatisticsFileName, nullptr);
        if (rs) {
            std::stringstream ss;
            ss << ERR_INIT_GW_STAT << rs << ": " << strerror_lorawan_ns(rs)
               << " " << config->serverConfig.logGWStatisticsFileName << std::endl;
            listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
            exit(ERR_CODE_INIT_GW_STAT);
        }
    }

    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_STAT_START);
    // Start device statistics service
    switch (config->serverConfig.deviceStatStorageType) {
        case DEVICE_STAT_FILE_CSV:
            deviceStatService = new DeviceStatServiceFileCsv();
            break;
        case DEVICE_STAT_FILE_JSON:
            deviceStatService = new DeviceStatServiceFileJson();
            break;
        case DEVICE_STAT_POST:
            deviceStatService = new DeviceStatServicePost();
            break;
        default:
            deviceStatService = nullptr;
    }

    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_STAT_INIT);
    if (deviceStatService) {
        rs = deviceStatService->init(config->serverConfig.logDeviceStatisticsFileName, nullptr);
        if (rs) {
            std::stringstream ss;
            ss << ERR_INIT_DEVICE_STAT << rs << ": " << strerror_lorawan_ns(rs)
               << " " << config->serverConfig.logDeviceStatisticsFileName << std::endl;
            listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
            exit(ERR_CODE_INIT_DEVICE_STAT);
        }
    }

    if (config->serverConfig.verbosity > 5) {
        std::vector<NetworkIdentity> identities;
        std::stringstream ss;
        ss << MSG_DEVICES << std::endl;
        identityService->list(identities, 0, MAX_DEVICE_LIST_COUNT + 1);
        size_t c = 0;
        for (std::vector<NetworkIdentity>::const_iterator it(identities.begin()); it != identities.end(); it++) {
            ss
                    << TAB << activation2string(it->activation)
                    << TAB << DEVADDR2string(it->devaddr)
                    << TAB << DEVEUI2string(it->devEUI)
                    << TAB << DEVICENAME2string(it->name);
            if (identityService->canControlService(it->devaddr))
                ss << TAB << "master";
            ss << std::endl;
            if (c > MAX_DEVICE_LIST_COUNT) {
                ss << TAB << ".." << std::endl;
                break;
            }
            c++;
        }
        ss << identityService->size() << " " << MSG_DEVICE_COUNT << std::endl;
        listenerOnLog(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }

    switch (config->serverConfig.deviceStatStorageType) {
        case DEVICE_STAT_FILE_JSON:
        case DEVICE_STAT_FILE_CSV:
        case DEVICE_STAT_POST:
            listener->setDeviceStatDumper(config,
                [this](void *env, const SemtechUDPPacket &packet)
                                          { return onDeviceStatDump(env, packet); }
            );
            break;
        default:
            break;
    }

    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_HISTORY_START);
    deviceHistoryService = new JsonFileDeviceHistoryService();
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_HISTORY_INIT);
    rs = deviceHistoryService->init(config->serverConfig.deviceHistoryStorageName, nullptr);
    if (rs) {
        std::stringstream ss;
        ss << ERR_INIT_DEVICE_STAT << rs << ": " << strerror_lorawan_ns(rs)
           << " " << config->serverConfig.deviceHistoryStorageName << std::endl;
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());

        // That's ok, no problem at all
        // exit(ERR_CODE_INIT_DEVICE_STAT);
    }

    // load regional settings
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_REGIONAL_SET_START);
    regionalParameterChannelPlans = new RegionalParameterChannelPlanFileJson();
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_REGIONAL_SET_INIT);

    // initialize regional settings
    rs = regionalParameterChannelPlans->init(config->serverConfig.regionalSettingsStorageName, nullptr);
    if (rs) {
        int parseCode;
        std::string parseDescription = regionalParameterChannelPlans->getErrorDescription(parseCode);

        std::stringstream ss;
        ss << ERR_MESSAGE << ERR_CODE_INIT_REGION_BANDS << ": " << ERR_INIT_REGION_BANDS
           << ", code " << rs << ": " << strerror_lorawan_ns(rs)
           << ", file: " << config->serverConfig.regionalSettingsStorageName
           << ", parseRX error " << parseCode << ": " << parseDescription
           << std::endl;
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
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
        std::stringstream ss;
        ss << ERR_MESSAGE << ERR_CODE_REGION_BAND_NO_DEFAULT << ": " << ERR_REGION_BAND_NO_DEFAULT << std::endl;
        listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss.str());
        exit(ERR_CODE_REGION_BAND_NO_DEFAULT);
    }

    if (config->serverConfig.verbosity > 5) {
        std::stringstream ss;
        ss << MSG_REGIONAL_SETTINGS << regionalParameterChannelPlan->toDescriptionTableString();
        listenerOnLog(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }

    // Start received message queue service
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_RECEIVER_QUEUE_START);
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
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_RECEIVER_QUEUE_INIT);
    rs = receiverQueueService->init(config->serverConfig.queueStorageName, options);
    if (rs) {
        std::stringstream ss;
        ss << ERR_INIT_QUEUE << rs << ": " << strerror_lorawan_ns(rs)
           << " " << config->serverConfig.queueStorageName << std::endl;
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
        // that's ok
        // exit(ERR_CODE_INIT_QUEUE);
    }

    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_START_OUTPUT_DB_SVC);
#ifdef ENABLE_PKT2
    configDatabases = new ConfigDatabases(config->databaseConfigFileName);
#else
    configDatabases = new ConfigDatabasesJson(config->databaseConfigFileName);
#endif
    if (configDatabases->dbs.empty()) {
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ERR_LOAD_DATABASE_CONFIG);
        // exit(ERR_CODE_LOAD_DATABASE_CONFIG);
    }

    if (config->serverConfig.verbosity > 5)
        listenerOnLog(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, MSG_DATABASE_LIST);

    // helper class to find out database by name or sequence number (id)
    dbByConfig = new DatabaseByConfig(configDatabases);
    // check out database connectivity
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_CHECKING_DB_AVAILABILITY);
    bool dbOk = true;
    for (std::vector<ConfigDatabase>::const_iterator it(configDatabases->dbs.begin()); it != configDatabases->dbs.end(); it++) {
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
        if (config->serverConfig.verbosity > 5) {
            std::stringstream ss;
            ss << TAB << it->name  << TAB << MSG_CONNECTION
               << (hasConn ? MSG_CONN_ESTABLISHED : MSG_CONN_FAILED);
            if (r)
                ss <<  ": " << strerror_lorawan_ns(r);
            if (dc->db)
                ss << " " << dc->db->errmsg;
            ss << std::endl;
            listenerOnLog(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
        }
        if (hasConn)
            dc->close();
        else
            dbOk = false;
    }
    // exit, if it can not connect to the database
    if (!dbOk) {
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_LOAD_DATABASE_CONFIG, ERR_LOAD_DATABASE_CONFIG);
        exit(ERR_CODE_LOAD_DATABASE_CONFIG);
    }


#ifdef ENABLE_PKT2
    onLog(nullptr, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, "Initialize payload parser PKT2 ..");
	parserEnv = initPkt2(config->protoPath, 0);
	if (!parserEnv) {
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_LOAD_PROTO, ERR_LOAD_PROTO);
		exit(ERR_CODE_LOAD_PROTO);
	}
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
    DbLoggerKosaPacketsLoader loggerKosaPacketsLoader;
    bool hasLoggerKosaPacketsLoader = false;
    // set database to load from
    if (!config->loggerDatabaseName.empty()) {
        DatabaseNConfig *kldb = dbByConfig->find(config->loggerDatabaseName);
        if (kldb) {
            loggerKosaPacketsLoader.setDatabase(kldb->db);
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
        sskldb << MSG_INIT_LOGGER_HUFFMAN << config->loggerDatabaseName;
        listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, sskldb.str());
    } else {
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_INIT_LOGGER_HUFFMAN_DB, ERR_INIT_LOGGER_HUFFMAN_DB);
    }
    loggerParserEnv = initLoggerParser(config->databaseExtraConfigFileNames, listenerOnLog, &loggerKosaPacketsLoader);
    if (!loggerParserEnv) {
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER, ERR_INIT_LOGGER_HUFFMAN_PARSER);
        exit(ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER);
    }
#endif

    // Set up processor
    listenerOnLog(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_PKT2_START);
    processor = new LoraPacketProcessor();
    processor->setLogger(listenerOnLog);
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
    receiverQueueProcessor->setLogger(listenerOnLog);

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
        std::stringstream ss;
        ss << ERR_MESSAGE << ERR_CODE_PARAM_NO_INTERFACE << ": " <<  ERR_PARAM_NO_INTERFACE << std::endl;
        listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_PARAM_NO_INTERFACE, ss.str());
        exit(ERR_CODE_PARAM_NO_INTERFACE);
    }
}

void RunListener::start() {
    for (std::vector<std::string>::const_iterator it(config->serverConfig.listenAddressIPv4.begin()); it != config->serverConfig.listenAddressIPv4.end(); it++) {
        if (!listener->add(*it, MODE_FAMILY_HINT_IPV4)) {
            std::stringstream ss;
            ss << ERR_MESSAGE << ERR_CODE_SOCKET_BIND << ": " <<  ERR_SOCKET_BIND << *it << std::endl;
            listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_PARAM_NO_INTERFACE, ss.str());
            exit(ERR_CODE_SOCKET_BIND);
        }
    }
    for (std::vector<std::string>::const_iterator it(config->serverConfig.listenAddressIPv6.begin()); it != config->serverConfig.listenAddressIPv6.end(); it++) {
        if (!listener->add(*it, MODE_FAMILY_HINT_IPV6)) {
            std::stringstream ss;
            ss << ERR_MESSAGE << ERR_CODE_SOCKET_BIND << ": " <<  ERR_SOCKET_BIND << *it << std::endl;
            listenerOnLog(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_PARAM_NO_INTERFACE, ss.str());
            exit(ERR_CODE_SOCKET_BIND);
        }
    }
}
