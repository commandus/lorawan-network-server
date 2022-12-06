#ifndef PACKET_QUEUE_H_
/**
 * PacketQueue class send message to the end-device via gateway at desired time
 */ 
#define PACKET_QUEUE_H_ 1

#include <deque>
#include <vector>
#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>
#include <time.h>

#include "utillora.h"
#include "packet-handler-abstract.h"
#include "device-history-service-abstract.h"
#include "gateway-list.h"
#include "device-channel-plan.h"
#include "log-intf.h"

// default delay in ms
#define DEF_DELAY_MS 500
#define MIN_DELAY_MS 200
#define MAX_DELAY_MS 1000

// queue default timeout
#define DEF_TIMEOUT_MS 200

typedef enum {
	MODE_NONE = 0,
	MODE_ACK = 1,
	MODE_REPLY_MAC = 2,
	MODE_JOIN_RESPONSE = 3,
	MODE_CONTROL_NS = 4 // manage network service
} ITEM_PROCESS_MODE;

class SemtechUDPPacketItem {
	public:
		struct timeval time2send;
		SemtechUDPPacket packet;
		ITEM_PROCESS_MODE processMode;
		// socket
		int socket;
		// device address actually is gatreway address
		DEVADDRINT getAddr() const;
		SemtechUDPPacketItem();
		SemtechUDPPacketItem(const SemtechUDPPacket &aPacket);
		SemtechUDPPacketItem(
			int socket,
			ITEM_PROCESS_MODE mode,
			const struct timeval &time,
			const SemtechUDPPacket &aPacket
		);
		std::string toJsonString() const;
        std::string toString() const;
};

class SemtechUDPPacketItems {
	public:
		std::vector <SemtechUDPPacketItem> packets;
		std::string toString() const;
};

class PacketQueue {
	private:
		int mode;	// 0- stopped, 1- running, -1- stop request
		int delayMicroSeconds;	// wait for packets from any base station and send response
		std::mutex mutexq;
		std::thread *threadSend;
		// void runner(PacketHandler &value);
		LogIntf *onLog;

		int fdWakeup;
		IdentityService *identityService;
		DeviceHistoryService *deviceHistoryService;
		GatewayList *gatewayList;
        const DeviceChannelPlan *deviceChannelPlan;

        int replyMAC(
			SemtechUDPPacketItem &item,
			struct timeval &t
		);
		int replyControl(
			SemtechUDPPacketItem &item,
			struct timeval &t
		);
        int replyJoinRequest(
            SemtechUDPPacketItem &item,
            struct timeval &time
        );

        void runner();

	public:
        // packets
		std::map<DEVADDRINT, SemtechUDPPacketItems, DEVADDRINTCompare> packets;
        // and their addresses
		std::deque <DEVADDRINT> addrs;
		// statistics, packets read since last start()
		size_t packetsRead;

		PacketQueue();
		PacketQueue(int delayMilliSeconds);
		~PacketQueue();
		void setIdentityService(IdentityService* identityService);
		void setDeviceHistoryService(DeviceHistoryService *deviceStatService);
		void setGatewayList(GatewayList *value);
		void setDelay(int delayMilliSeconds);
		int ack(
			int socket,
			struct sockaddr* gwAddress,
			const SEMTECH_PREFIX_GW &dataprefix
		);
		void push(
			int socket,
			ITEM_PROCESS_MODE mode,
			const struct timeval &time2send,
			const SemtechUDPPacket &value
		);
		size_t count();
		/**
		 * @param retval returned packet from queue
		 * @param retmode returned what to do with packet
		 * @param currentTime time
		 * @return  true if packet ready for send returned in the retval parameter
		 */
		bool getFirstExpired(
			SemtechUDPPacketItem &retval,
			struct timeval &currentTime
		);
		int getNextTimeout(struct timeval &currenttime);
		void start(PacketHandler &value);
		void stop();
		void wakeUp();
		int diffMicroSeconds(struct timeval &t1, struct timeval &t2);
		std::string toString() const;
		void setLogger(LogIntf *value);

    void setDeviceChannelPlan(const DeviceChannelPlan *value);
};

#endif
