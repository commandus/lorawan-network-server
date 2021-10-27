#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H 1

#include <string>
#include <vector>
#include <functional>
#include "udp-socket.h"
#include "utillora.h"
#include "lora-packet-handler-abstract.h"
#include "identity-service-abstract.h"
#include "device-history-service-abstract.h"
#include "gateway-list.h"

/**
 * Listen UDP port(s) for packets sent by Semtech's gateway on interfaces with IPv4 or IPv6 address
 * from gateways listed in the GatewayList.
 * Gateway send PULL request with address and port on which gateway now listen, UDPListener 
 * save received address and port in the GatewayList.
 * Devices are identified by IdentityService.
 * Identified packets with application payload with optional MAC commands passed to the LoraPacketHandler.
 * LoraPacketHandler handles MAC commands in another thread, application payload pass further via queue.
 */
class UDPListener
{
private:
	std::string buffer;
	int largestSocket();
	int *sysSignalPtr;
	GatewayList *gatewayList;
public:
	int verbosity;
	IdentityService *identityService;
	DeviceHistoryService *deviceHistoryService;

	std::vector<UDPSocket> sockets;
	bool stopped;
	struct sockaddr_in6 remotePeerAddress;

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

	UDPListener();
	~UDPListener();

	void setBufferSize(size_t value);

	std::string toString();

	bool add(
		const std::string &address,
		MODE_FAMILY familyHint
	);
	void clear();

	int listen();
	int parseBuffer(
		const std::string &buffer,
		size_t bytesReceived,
		int socket,
		const struct timeval &recievedTime,
		const struct sockaddr_in6 &gwAddress
	);

	int peerAddrIndex(
		struct sockaddr *remotePeerAddr);

	void setLastRemoteAddress(
		struct sockaddr *value
	);

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
