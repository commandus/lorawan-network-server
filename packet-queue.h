#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H 1


#include <deque>
#include <vector>
#include <map>
#include <vector>
#include <mutex>
#include <time.h>

#include "utillora.h"

// minimum delay in ms
#define MIN_DELAY_MS 200
// maximum delay in ms
#define MAX_DELAY_MS 1000

class SemtechUDPPacketItem {
	public:
		struct timeval timeAdded;
		semtechUDPPacket packet;
		DEVADDRINT getAddr() const;
		SemtechUDPPacketItem(const semtechUDPPacket &packet);
		std::string toString() const;
};

class SemtechUDPPacketsAddr {
	public:
		std::vector <SemtechUDPPacketItem> packets;
		std::string toString() const;
};

class PacketQueue {
	private:
		int delayMS;
		std::mutex mutexq;
	public:
		std::map<DEVADDRINT, SemtechUDPPacketsAddr, DEVADDRINTCompare> packets;
		std::deque <DEVADDRINT> addrs;
		PacketQueue();
		PacketQueue(int delayMs);
		void put(const semtechUDPPacket &value);
		size_t count();
		bool getFirstExpired(semtechUDPPacket &retval, struct timeval &currenttime);
		void start();
		void stop();
		int diffMS(struct timeval &t1, struct timeval &t2);
		std::string toString() const;
};

#endif
