
/**
 * Usage
 * ./test-packet-queue
 */

#include <iostream>
#include <iomanip>
#include <sys/time.h>
#include <signal.h>
#include <thread>
#include "unistd.h"

#include "packet-queue.h"
#include "errlist.h"
#include "utildate.h"

bool stopped;
size_t packetsSent = 0;
size_t packetsRead = 0;
PacketQueue q(1000);

uint32_t packetAddress = 0;
int putSomePackets()
{
	while (!stopped) {
		for (int i = 0; i < 16; i++) {
			semtechUDPPacket v;
			std::stringstream ss;
			ss << std::hex << std::setw(8) << std::setfill('0') << packetAddress;
			// std::cerr << ss.str() << std::endl;
			packetAddress++;
			v.setDeviceAddr(ss.str());
			for (int j = 0; j < 2; j++) {
				q.put(v);
			}
			packetsSent++;
		}
		usleep(200);
	}
	return 0;
}

void readPackets()
{
	while (!stopped) {
		if (q.count()) {
			semtechUDPPacket p;
			struct timeval t;
			gettimeofday(&t, NULL);
			while (q.getFirstExpired(p, t)) {
				/*
				std::cerr << timeval2string(t) << " "  << p.getDeviceAddrStr() << " "
					<< p.metadataToJsonString() << std::endl;
				*/
				packetsRead++;
			}
		} else {
			usleep(100000);
		}
	}
}

void signalHandler(int signal)
{
	switch (signal)
	{
	case SIGINT:
		stopped = true;
		break;
	default:
		break;
	}
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = &signalHandler;
	sigaction(SIGINT, &action, NULL);
}
#endif

void timeout60s() {
	sleep(3600);
	std::cerr << "Timeout" << std::endl;
	stopped = true;
}

void t1() {
	stopped = false;
	int r = putSomePackets();
	if (r) {
		std::cerr << r << ": " << strerror_client(r) << std::endl;
		exit(r);
	}
	while (q.count()) {
		semtechUDPPacket p;
		struct timeval t;
		gettimeofday(&t, NULL);
		if (q.getFirstExpired(p, t)) {
			/*
			std::cerr << timeval2string(t) << " " << p.getDeviceAddrStr() << " "
				<< p.metadataToJsonString() << std::endl;
			*/
		}
	}
}

void printStat() {
	while (!stopped) {
		std::cout 
			<< "Read " << std::setw(8) << packetsRead
			<< " Sent " << std::setw(8) << packetsSent
			<< " Queue " << std::setw(8) << q.count()
			<< std::endl;
		usleep(200000);
	}
}

void t2() {
	setSignalHandler();
	std::cerr << "Press Ctrl+C to exit or wait 60s" << std::endl;
	std::thread tp(putSomePackets);
	tp.detach();

	std::thread ts(printStat);
	ts.detach();

	std::thread tw(timeout60s);
	tw.detach();

	std::thread tr(readPackets);
	tr.join();
	printStat();
}

int main(int argc, char **argv) {
	t2();
}
