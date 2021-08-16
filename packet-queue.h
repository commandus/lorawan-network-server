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
#include "gateway-list.h"

// default delay in ms
#define DEF_DELAY_MS 1000
#define MIN_DELAY_MS 200
#define MAX_DELAY_MS 1000

// queue default timeout
#define DEF_TIMEOUT_MS 200

typedef enum {
	MODE_NONE = 0,
	MODE_ACK = 1,
	MODE_REPLY_MAC = 2
} ITEM_PROCESS_MODE;

class SemtechUDPPacketItem {
	public:
		struct timeval timeAdded;
		semtechUDPPacket packet;
		ITEM_PROCESS_MODE processMode;
		// socket
		int socket;
		// device address actually is gatreway address
		DEVADDRINT getAddr() const;
		SemtechUDPPacketItem();
		SemtechUDPPacketItem(const semtechUDPPacket &packet);
		SemtechUDPPacketItem(
			int socket,
			ITEM_PROCESS_MODE mode,
			const struct timeval &time,
			const semtechUDPPacket &packet
		);
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
		std::function<void(
			void *env,
			int level,
			int modulecode,
			int errorcode,
			const std::string &message
		)> onLog;

		int fdWakeup;
		IdentityService *identityService;
		GatewayList *gatewayList;
		void runner();
	public:
		std::map<DEVADDRINT, SemtechUDPPacketItems, DEVADDRINTCompare> packets;
		std::deque <DEVADDRINT> addrs;
		// statistics, packets read since last start()
		size_t packetsRead;

		PacketQueue();
		PacketQueue(int delayMilliSeconds);
		~PacketQueue();
		void setIdentityService(IdentityService* identityService);
		void setGatewayList(GatewayList *value);
		void setDelay(int delayMilliSeconds);
		int ack(
			int socket,
			const sockaddr_in* gwAddress,
			const SEMTECH_DATA_PREFIX &dataprefix
		);
		void push(
			int socket,
			ITEM_PROCESS_MODE mode,
			const struct timeval &time,
			const semtechUDPPacket &value
		);
		size_t count();
		/**
		 * @param retval returned packet from queue
		 * @param retmode returned what to do with packet
		 * @param currenttime time
		 * @return  true if packet ready for send returned in the retval parameter
		 */
		bool getFirstExpired(
			SemtechUDPPacketItem &retval,
			struct timeval &currenttime
		);
		int getNextTimeout(struct timeval &currenttime);
		void start(PacketHandler &value);
		void stop();
		void wakeUp();
		int diffMicroSeconds(struct timeval &t1, struct timeval &t2);
		std::string toString() const;
		void setLogger(
			std::function<void(
				void *env,
				int level,
				int modulecode,
				int errorcode,
				const std::string &message
		) > value);
};

#endif
