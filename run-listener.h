#ifndef RUN_LISTENER_H_
#define RUN_LISTENER_H_	1

#include "config-json.h"
#include "auth-user.h"
#include "gateway-list.h"
#include "packet-listener.h"
#ifdef ENABLE_LISTENER_UDP
#include "udp-listener.h"
#endif
#ifdef ENABLE_LISTENER_USB
#include "embedded-listener.h"
#endif
#ifdef ENABLE_LISTENER_USB
#include "usb-listener.h"
#endif

#include "receiver-queue-processor.h"
#include "lora-packet-handler-impl.h"
#include "log-intf.h"

class RunListener : public LogIntf {
private:
    PayloadInsertPlugins plugins;
    std::mutex mDone;
public:
    Configuration *config;
    GatewayList *gatewayList;
    GatewayStatService *gatewayStatService;
    DeviceStatService *deviceStatService;

    // Listen UDP port(s) for packets sent by Semtech's gateway
    PacketListener *listener;
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

    RunListener();
    RunListener(Configuration *config, int *lastSysSignal);

    void flushFiles();
    void onInfo(
        void *env,
        int level,
        int moduleCode,
        int errorCode,
        const std::string &message
    ) override;
    void onConnected(bool connected) override;
    void onDisconnected() override;
    void onStarted(uint64_t gatewayId, const std::string regionName, size_t regionIndex) override;
    void onFinished(const std::string &message) override;
    void onReceive(Payload &value) override;
    void onValue(Payload &value) override;

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

