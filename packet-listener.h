#ifndef PACKET_LISTENER_H
#define PACKET_LISTENER_H 1

#include <string>
#include <vector>
#include <functional>
#include "udp-socket.h"
#include "utillora.h"
#include "lora-packet-handler-abstract.h"
#include "identity-service.h"
#include "device-history-service-abstract.h"
#include "gateway-list.h"
#include "log-intf.h"

typedef enum {
    THREAD_MAIN = 0,
    THREAD_UPSTREAM,
    THREAD_DOWNSTREAM,
    THREAD_JIT,
    THREAD_GPS,
    THREAD_GPSCHECKTIME,
    THREAD_SPECTRALSCAN
} ENUM_GATEWAY_THREAD;

// interface
class ThreadStartFinish {
public:
    virtual void onThreadStart(ENUM_GATEWAY_THREAD thread) = 0;
    virtual void onThreadFinish(ENUM_GATEWAY_THREAD thread) = 0;
};

/**
 * Abstract class listen for packets sent by Semtech's gateway
 * from gateways listed in the GatewayList.
 * Gateway send PULL request with address and port on which gateway now listen, UDPListener 
 * save received address and port in the GatewayList.
 * Devices are identified by IdentityService.
 * Identified packets with application payload with optional MAC commands passed to the LoraPacketHandler.
 * LoraPacketHandler handles MAC commands in another thread, application payload pass further via queue.
 */
class PacketListener
{
protected:
    GatewayList *gatewayList;
    int *sysSignalPtr;
public:
	int verbosity;
	std::string regionName;
	int regionIndex;
	IdentityService *identityService;
	DeviceHistoryService *deviceHistoryService;

	bool stopped;

    LogIntf *onLog;

	std::function<void(
		void *env,
		GatewayStat *value
	)> onGatewayStatDump;

	void *gwStatEnv;

	std::function<void(
        void *env,
        const SemtechUDPPacket &value
	)> onDeviceStatDump;

	void *deviceStatEnv;

	LoraPacketHandler *handler;

	PacketListener();
	virtual ~PacketListener();

    virtual std::string toString() const;

    virtual bool add(const std::string &value, int hint) = 0;
    int add(const std::vector<std::string> &value, int hint);
	virtual void clear();

	virtual int listen(const std::string &regionName, int regionIndex, void *config, int flags,
					   ThreadStartFinish *threadStartFinish) = 0;

	void setLogger(int aVerbosity, LogIntf *value);

	void setHandler(LoraPacketHandler *value);
	void setIdentityService(IdentityService* value);
	void setGatewayList(GatewayList *value);
	void setDeviceHistoryService(DeviceHistoryService *value);
	void setSysSignalPtr(int *value);

	void setGatewayStatDumper(
		void *gwStatEnv,
		std::function<void(
			void *env,
			GatewayStat *value
	)> onGatewayStatDump);

	void setDeviceStatDumper(
		void *deviceStatEnv,
		std::function<void(
			void *env,
			const SemtechUDPPacket &value
	)> onDeviceStatDump);
};

#endif
