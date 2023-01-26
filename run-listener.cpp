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

// JSON
#include "database-config-json.h"
// Javascript, disabled
// #include "pkt2/database-config.h"

#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#include <unistd.h>
#define ENABLE_TERM_COLOR	1
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
    receiverQueueService(nullptr)
{
}

RunListener::RunListener(Configuration *aConfig, int *lastSysSignal)
    : RunListener()
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

void RunListener::onInfo(
    void *env,
    int level,
    int moduleCode,
    int errorCode,
    const std::string &message
)
{
    if (config && config->serverConfig.verbosity < 0) {
        // set level to LOG_ERR if you want see all messages in the vat/log/syslog
        // level = LOG_ERR;
        SYSLOG(level, message.c_str());
        return;
    }
    if (config && config->serverConfig.verbosity < level)
        return;
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

void RunListener::onConnected(bool connected)
{

}

void RunListener::onDisconnected()
{

}

void RunListener::onStarted(uint64_t gatewayId, const std::string regionName, size_t regionIndex)
{

}

void RunListener::onFinished(const std::string &message)
{

}

void RunListener::onValue(Payload &value)
{

}

void RunListener::done()
{
    mDone.lock();

    if (config && config->serverConfig.verbosity > 5)
        onInfo(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, MSG_GRACEFULLY_STOPPED);
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
    plugins.done();

    mDone.unlock();

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
#ifdef ENABLE_LISTENER_UDP
    listener = new UDPListener();
#endif
#ifdef ENABLE_LISTENER_USB
    listener = new USBListener();
#endif
    if (!listener)
        return;
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_INIT_UDP_LISTENER);
    // check signal number when select() has been interrupted
    listener->setSysSignalPtr(lastSysSignal);
    listener->setLogger(config->serverConfig.verbosity, this);
    listener->setGatewayStatDumper(config, [this](void *env, GatewayStat *stat) { return onGatewayStatDump(env, stat); } );

    if (config->serverConfig.verbosity > 5) {
        std::stringstream ss;
        ss << MSG_LISTEN_IP_ADDRESSES << std::endl << config->toDescriptionTableString();
        onInfo(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }

    gatewayList = new GatewayList(config->gatewaysFileName);

    if (config->serverConfig.verbosity > 5) {
        std::stringstream ss;
        ss << MSG_GATEWAY_LIST << std::endl << gatewayList->toDescriptionTableString();
        onInfo(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }

    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_IDENTITY_START);
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
    ss << MSG_IDENTITY_INIT  << identityService->getNetworkId()->toString()
        << ": " << config->serverConfig.identityStorageName
        << "..";
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss.str());
    int rs = identityService->init(config->serverConfig.identityStorageName, nullptr);
    if (rs) {
        std::stringstream ss;
        ss << ERR_INIT_IDENTITY << rs << ": " << strerror_lorawan_ns(rs)
           << " " << config->serverConfig.identityStorageName << std::endl;
        onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
        exit(ERR_CODE_INIT_IDENTITY);
    }

    // Start gateway statistics service
    switch (config->serverConfig.gwStatStorageType) {
        case GW_STAT_FILE_JSON:
            onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_FILE_START);
            gatewayStatService = new GatewayStatServiceFile();
            break;
        case GW_STAT_POST:
            onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_POST_START);
            gatewayStatService = new GatewayStatServicePost();
            break;
        default:
            gatewayStatService = nullptr;
    }

    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_GW_STAT_INIT);
    if (gatewayStatService) {
        rs = gatewayStatService->init(config->serverConfig.logGWStatisticsFileName, nullptr);
        if (rs) {
            std::stringstream ss;
            ss << ERR_INIT_GW_STAT << rs << ": " << strerror_lorawan_ns(rs)
               << " " << config->serverConfig.logGWStatisticsFileName << std::endl;
            onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
            exit(ERR_CODE_INIT_GW_STAT);
        }
    }

    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_STAT_START);
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

    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_STAT_INIT);
    if (deviceStatService) {
        rs = deviceStatService->init(config->serverConfig.logDeviceStatisticsFileName, nullptr);
        if (rs) {
            std::stringstream ss;
            ss << ERR_INIT_DEVICE_STAT << rs << ": " << strerror_lorawan_ns(rs)
               << " " << config->serverConfig.logDeviceStatisticsFileName << std::endl;
            onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
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
        onInfo(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }

    switch (config->serverConfig.deviceStatStorageType) {
        case DEVICE_STAT_FILE_JSON:
        case DEVICE_STAT_FILE_CSV:
        case DEVICE_STAT_POST:
            listener->setDeviceStatDumper(config,
                [this](void *env, const SemtechUDPPacket &packet) {
                    return onDeviceStatDump(env, packet);
                }
            );
            break;
        default:
            break;
    }

    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_HISTORY_START);
    deviceHistoryService = new JsonFileDeviceHistoryService();
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_DEV_HISTORY_INIT);
    rs = deviceHistoryService->init(config->serverConfig.deviceHistoryStorageName, nullptr);
    if (rs) {
        std::stringstream ss;
        ss << ERR_INIT_DEVICE_STAT << rs << ": " << strerror_lorawan_ns(rs)
           << " " << config->serverConfig.deviceHistoryStorageName << std::endl;
        onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());

        // That's ok, no problem at all
        // exit(ERR_CODE_INIT_DEVICE_STAT);
    }

    // load regional settings
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_REGIONAL_SET_START);
    regionalParameterChannelPlans = new RegionalParameterChannelPlanFileJson();
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_REGIONAL_SET_INIT);

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
        onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
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
        onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss.str());
        exit(ERR_CODE_REGION_BAND_NO_DEFAULT);
    }

    if (config->serverConfig.verbosity > 5) {
        std::stringstream ss;
        ss << MSG_REGIONAL_SETTINGS << regionalParameterChannelPlan->toDescriptionTableString();
        onInfo(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
    }

    // Start received message queue service
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_RECEIVER_QUEUE_START);
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
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_RECEIVER_QUEUE_INIT);
    rs = receiverQueueService->init(config->serverConfig.queueStorageName, options);
    if (rs) {
        std::stringstream ss;
        ss << ERR_INIT_QUEUE << rs << ": " << strerror_lorawan_ns(rs)
           << " " << config->serverConfig.queueStorageName << std::endl;
        onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ss.str());
        // that's ok
        // exit(ERR_CODE_INIT_QUEUE);
    }

    // Try load payload parser plugins
    if (config->pluginsPath.empty()) {
        onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_PLUGIN_PATH_NOT_FOUND);
    } else {
        std::stringstream ss1;
        ss1 << MSG_LOAD_PLUGINS << "\"" << config->pluginsPath << "\"";
        onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss1.str());
        plugins.unload();
        int r = plugins.load(config->pluginsPath);
        if (r <= 0)
            onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_NO_PLUGINS_LOADED);
        else {
            std::stringstream ss;
            ss << MSG_LOADED_PLUGINS_COUNT << r;
            onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, ss.str());
        }
    }

    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_START_OUTPUT_DB_SVC);
    // Javascript, disabled
    // configDatabases = new ConfigDatabases(config->databaseConfigFileName);
    // JSON
    configDatabases = new ConfigDatabasesJson(config->databaseConfigFileName);
    if (configDatabases->dbs.empty()) {
        onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, LORA_OK, ERR_LOAD_DATABASE_CONFIG);
        // exit(ERR_CODE_LOAD_DATABASE_CONFIG);
    }

    if (config->serverConfig.verbosity > 5)
        onInfo(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, MSG_DATABASE_LIST);

    // helper class to find out database by name or sequence number (id)
    dbByConfig = new DatabaseByConfig(configDatabases, &plugins);
    // check out database connectivity
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_CHECKING_DB_AVAILABILITY);
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
            onInfo(listener, LOG_INFO, LOG_MAIN_FUNC, LORA_OK, ss.str());
        }
        if (hasConn)
            dc->close();
        else
            dbOk = false;
    }
    // exit, if it can not connect to the database
    if (!dbOk) {
        onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_LOAD_DATABASE_CONFIG, ERR_LOAD_DATABASE_CONFIG);
        exit(ERR_CODE_LOAD_DATABASE_CONFIG);
    }

    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_INIT_PLUGINS);
    // TODO pass
    int pec = plugins.init(config->pluginsParams, this, 0);
	if (pec) {
        onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_INIT_PLUGINS_FAILED, ERR_INIT_PLUGINS_FAILED);
		exit(ERR_CODE_INIT_PLUGINS_FAILED);
	}

    // Set up processor
    onInfo(listener, LOG_DEBUG, LOG_MAIN_FUNC, LORA_OK, MSG_START_PACKET_PROCESSOR);
    processor = new LoraPacketProcessor();
    processor->setLogger(this);
    processor->setIdentityService(identityService);
    processor->setGatewayList(gatewayList);
    processor->setReceiverQueueService(receiverQueueService);
    processor->setDeviceHistoryService(deviceHistoryService);
    // FPort number reserved for messages controls network service. 0- no remote control allowed
    processor->reserveFPort(config->serverConfig.controlFPort);
    processor->setDeviceChannelPlan(deviceChannelPlan);

    receiverQueueProcessor = new ReceiverQueueProcessor();
    receiverQueueProcessor->setLogger(this);

    // Set databases
    receiverQueueProcessor->setDatabaseByConfig(dbByConfig);

    // Set up listener
    listener->setHandler(processor);
    listener->setGatewayList(gatewayList);
    listener->setIdentityService(identityService);
    listener->setDeviceHistoryService(deviceHistoryService);

    if (config->serverConfig.listenAddressIPv4.empty() && config->serverConfig.listenAddressIPv6.empty()) {
        std::stringstream ss;
        ss << ERR_MESSAGE << ERR_CODE_PARAM_NO_INTERFACE << ": " <<  ERR_PARAM_NO_INTERFACE << std::endl;
        onInfo(listener, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_PARAM_NO_INTERFACE, ss.str());
        exit(ERR_CODE_PARAM_NO_INTERFACE);
    }
}

void RunListener::start() {
    // start processing queue
    if (processor && receiverQueueProcessor)
        processor->setReceiverQueueProcessor(receiverQueueProcessor);
}
