#ifndef RUN_LISTENER_H_
#define RUN_LISTENER_H_	1

#include "config-json.h"
#include "auth-user.h"
#include "gateway-list.h"
#include "udp-listener.h"
#include "receiver-queue-processor.h"
#include "lora-packet-handler-impl.h"

class RunListener {
public:
    Configuration *config;

    GatewayList *gatewayList;
    GatewayStatService *gatewayStatService;
    DeviceStatService *deviceStatService;

    // Listen UDP port(s) for packets sent by Semtech's gateway
    UDPListener *listener;
    // Device identity service
    IdentityService *identityService;
    // ReceiverQueueProcessor get payload from the queue, parseRX and put parsed data
    ReceiverQueueProcessor *receiverQueueProcessor;
    // LoraPacketProcessor handles uplink messages
    LoraPacketProcessor *processor;
    // Database list
    DatabaseByConfig *dbByConfig;
    // Device counters and last received
    DeviceHistoryService *deviceHistoryService;
    // Regional settings
    RegionalParameterChannelPlans *regionalParameterChannelPlans;
    DeviceChannelPlan *deviceChannelPlan;
    // Database collection
    ConfigDatabasesIntf *configDatabases;
    ReceiverQueueService *receiverQueueService;

    // pkt2 environment
#ifdef ENABLE_PKT2
    void* parserEnv;
#endif
#ifdef ENABLE_LOGGER_HUFFMAN
    void* loggerParserEnv;
#endif

    RunListener();
    RunListener(Configuration *config, int *lastSysSignal);

    void flushFiles();
    static void listenerOnLog(
            void *env,
            int level,
            int moduleCode,
            int errorCode,
            const std::string &message
    );
    void init(Configuration *config, int *lastSysSignal);
    void start();
    void stop();
    void done();

    void onGatewayStatDump(
        void *env,
        GatewayStat *stat
    );
    void onDeviceStatDump(
        void *env,
        const SemtechUDPPacket &value
    );
};

#endif

