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
private:
	GatewayList *gatewayList;
protected:
    int *sysSignalPtr;

    int parseBuffer(
        const std::string &buffer,
        size_t bytesReceived,
        int socket,
        const struct timeval &receivedTime,
        const struct sockaddr_in6 &gwAddress
    );
public:
	int verbosity;
	IdentityService *identityService;
	DeviceHistoryService *deviceHistoryService;

	bool stopped;

	std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
	)> onLog;

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
	~PacketListener();

	virtual void setBufferSize(size_t value);

    virtual std::string toString() const;

    virtual bool add(const std::string &value, int hint) = 0;
    int add(const std::vector<std::string> &value, int hint);
	virtual void clear();

	virtual int listen() = 0;

	void setLogger(
		int verbosity,
		std::function<void(
			void *env,
			int level,
			int modulecode,
			int errorcode,
			const std::string &message
	)> onLog);

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