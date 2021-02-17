
/**
 * Usage
 * ./test-packet-queue
 */

#include <iostream>
#include <iomanip>

#include "packet-queue.h"
#include "errlist.h"

int t1()
{
	PacketQueue q;
	int r = 0;

	for (int i = 0; i < 16; i++) {
		semtechUDPPacket v;
		std::stringstream ss;
		ss << "010203" << std::hex << std::setw(2) << std::setfill('0') << i;
		v.setDeviceAddr(ss.str());
		/*
		for (int j = 0; j <= i; j++) {
			q.put(v);
		}
		*/
		for (int j = 0; j < 2; j++) {
			q.put(v);
		}
	}
	std::cerr << std::endl;
	std::cerr << q.toString();
	return r;
}

int main(int argc, char **argv) {
	int r = t1();
	if (r)
		std::cerr << r << ": " << strerror_client(r) << std::endl;
}
