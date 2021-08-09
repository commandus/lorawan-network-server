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
#include <time.h>

#include "utillora.h"
#include "packet-handler-abstract.h"

// default delay in ms
#define DEF_DELAY_MS 1000
#define MIN_DELAY_MS 200
#define MAX_DELAY_MS 1000

// queue default timeout
#define DEF_TIMEOUT_MS 200

class SemtechUDPPacketItem {
	public:
		struct timeval timeAdded;
		semtechUDPPacket packet;
		DEVADDRINT getAddr() const;
		SemtechUDPPacketItem(const semtechUDPPacket &packet);
		SemtechUDPPacketItem(
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
		bool isStarted;
		bool isDone;
		int delayMicroSeconds;	// wait for packets from any base station and send response
		std::mutex mutexq;
		std::thread *threadSend;
		// void runner(PacketHandler &value);
		void runner();
	public:
		std::map<DEVADDRINT, SemtechUDPPacketItems, DEVADDRINTCompare> packets;
		std::deque <DEVADDRINT> addrs;
		// statistics, packets read since last start()
		size_t packetsRead;

		PacketQueue();
		PacketQueue(int delayMilliSeconds);
		~PacketQueue();
		void setDelay(int delayMilliSeconds);
		void push(
			const struct timeval &time,
			const semtechUDPPacket &value
		);
		size_t count();
		bool getFirstExpired(semtechUDPPacket &retval, struct timeval &currenttime);
		int getNextTimeout(struct timeval &currenttime);
		void start(PacketHandler &value);
		void stop();
		int diffMicroSeconds(struct timeval &t1, struct timeval &t2);
		std::string toString() const;
};

#endif
