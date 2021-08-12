#include "packet-queue.h"
#include <sys/time.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <iostream>

#include "utildate.h"
#include "errlist.h"
#include "udp-socket.h"

SemtechUDPPacketItem::SemtechUDPPacketItem()
	: processMode(MODE_NONE)
{
}

SemtechUDPPacketItem::SemtechUDPPacketItem(
	const semtechUDPPacket &apacket
)
	: processMode(MODE_NONE), packet(apacket)
{
	gettimeofday(&timeAdded, NULL);
}

SemtechUDPPacketItem::SemtechUDPPacketItem(
	int socket,
	ITEM_PROCESS_MODE mode,
	const struct timeval &time,
	const semtechUDPPacket &apacket
)
	: processMode(mode), timeAdded(time), packet(apacket)
{
	
}

DEVADDRINT SemtechUDPPacketItem::getAddr() const
{
	return packet.getDeviceAddr();
}

std::string SemtechUDPPacketItem::toString() const
{
	std::stringstream ss;
	ss << timeval2string(timeAdded) << " " << packet.getDeviceAddrStr() << " " << packet.metadataToJsonString();
	return ss.str();
}

std::string SemtechUDPPacketItems::toString() const
{
	std::stringstream ss;
	for (std::vector <SemtechUDPPacketItem>::const_iterator it(packets.begin()); it != packets.end(); it++) {
		ss << it->toString() << " ";
	}
	return ss.str();
}

PacketQueue::PacketQueue()
	: packetsRead(0), delayMicroSeconds(DEF_DELAY_MS * 1000), mode(0), fdWakeup(0), onLog(NULL)
{
}

PacketQueue::PacketQueue(
	int delayMillisSeconds
)
	: packetsRead(0), mode(0), threadSend(NULL), fdWakeup(0), onLog(NULL)
{
	setDelay(delayMillisSeconds);
}

void PacketQueue::setLogger(
	std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value) {
	onLog = value;
}

void PacketQueue::setDelay(
	int delayMilliSeconds
) {
	if (delayMilliSeconds < MIN_DELAY_MS)
		delayMilliSeconds = MIN_DELAY_MS;
	if (delayMilliSeconds > MAX_DELAY_MS)
		delayMilliSeconds = MAX_DELAY_MS;
	delayMicroSeconds = delayMilliSeconds * 1000;
}

PacketQueue::~PacketQueue()
{
	stop();
}

void PacketQueue::push(
	int socket,
	ITEM_PROCESS_MODE mode,
	const struct timeval &time,
	const semtechUDPPacket &value
) {
	SemtechUDPPacketItem item(socket, mode, time, value);
	DEVADDRINT a(item.getAddr());
	mutexq.lock();
	std::map<DEVADDRINT, SemtechUDPPacketItems>::iterator it(packets.find(a));
	// add first packet, add metadata only for others
	if (it != packets.end()) {
		// there are already some packets from the device
		if (it->second.packets.size() == 0)
			// actually, no ;(
			it->second.packets.push_back(item);
		else {
			// sure, there are some packets already!
			// find out same packet (if more than 1)
			bool found = false;
			for (std::vector<SemtechUDPPacketItem>::iterator itp(it->second.packets.begin()); itp != it->second.packets.end(); itp++)
			{
				if (itp->packet.header.fport == value.header.fport) 
				{
					// we need metadata only for calc best gateway with strongest signal
					if (value.metadata.size())
						itp->packet.metadata.push_back(value.metadata[0]);
					found = true;
					break;
				}
			}
			if (!found)
				// add a new packet as a new one
				packets[a].packets.push_back(item);
		}
	} else {
		// this is first packet recieved from the device
		packets[a].packets.push_back(item);
		addrs.push_back(a);
	}
	mutexq.unlock();
}

/**
 * @return time dirrerence in milliseconds 
 **/
int PacketQueue::diffMicroSeconds(
	struct timeval &t1,
	struct timeval &t2
)
{
	int r = 1000000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec);
	if (r < 0)
		r = 0;
	return r;
}

size_t PacketQueue::count()
{
	return packets.size();
}

bool PacketQueue::getFirstExpired(
	SemtechUDPPacketItem &retval,
	struct timeval &currenttime
)
{
	if (!addrs.size())
		return false;

	mutexq.lock();

	DEVADDRINT a = addrs.front();
	std::map<DEVADDRINT, SemtechUDPPacketItems>::iterator it(packets.find(a));
	if (it == packets.end()) {
		mutexq.unlock();
		return false;
	}

	// always keep at least 1 item
	if (!it->second.packets.size()) {
		mutexq.unlock();
		return false;
	}

	// first packet is earliest packet, check time using first earliest packet
	if (diffMicroSeconds(it->second.packets[0].timeAdded, currenttime) < delayMicroSeconds) {
		mutexq.unlock();
		return false;
	}

	// return packet with received signal strength indicator. Worst is -85 dBm.
	int bestIndex = 0;
	std::vector<SemtechUDPPacketItem>::const_iterator pit(it->second.packets.begin());
	// validate have an other packet (by fcnt)
	uint16_t fcntFirst = pit->packet.header.header.fcnt;
	int16_t rssi = pit->packet.header.header.fcnt;
	bool hasOtherPacket = false;
	pit++;
	int idx = 0;
	for (; pit != it->second.packets.end(); pit++) {
		if (pit->packet.header.header.fcnt != fcntFirst) {
			hasOtherPacket = true;
		} else {
			if (pit->packet.metadata[0].rssi > rssi) {
				bestIndex = idx;
				rssi = pit->packet.metadata[0].rssi;

			}
		}
		idx++;
	}
	retval = it->second.packets[idx];

	if (hasOtherPacket) {
		// remove first received packet and aothers wit the sama fcnt
		for (std::vector<SemtechUDPPacketItem>::iterator pit(it->second.packets.begin()); pit != it->second.packets.end();) {
			if (pit->packet.header.header.fcnt == fcntFirst) {
				pit = it->second.packets.erase(pit);
			} else {
				pit++;
			}
		}
	} else {
		// entirely remove all packets
		packets.erase(it);
	}
	addrs.pop_front();

	mutexq.unlock();

	return true;
}

int PacketQueue::getNextTimeout(struct timeval &currenttime)
{
	DEVADDRINT a = addrs.front();
	std::map<DEVADDRINT, SemtechUDPPacketItems>::iterator it(packets.find(a));
	if (it == packets.end()) {
		return DEF_TIMEOUT_MS;
	}

	// always keep at least 1 item
	if (!it->second.packets.size()) {
		return DEF_TIMEOUT_MS;
	}
	// first packet is earliest packet
	return diffMicroSeconds(it->second.packets[0].timeAdded, currenttime);
}

std::string PacketQueue::toString() const
{
	std::stringstream ss;
	/*
	for (std::deque <DEVADDRINT>::const_iterator it(addrs.begin()); it != addrs.end(); it++) {
		ss << DEVADDRINT2string(*it) << std::endl;
	}
	ss << std::endl;
	*/
	for (std::map<DEVADDRINT, SemtechUDPPacketItems, DEVADDRINTCompare>::const_iterator it(packets.begin()); it != packets.end(); it++) {
		ss << DEVADDRINT2string(it->first) << ": " 
			<< it->second.toString()
			<< std::endl;
	}
	return ss.str();
}

// immediately send ACK
int PacketQueue::ack
(
	int socket,
	const sockaddr_in* gwAddress,
	const SEMTECH_DATA_PREFIX &dataprefix
)
{
	SEMTECH_ACK response;
	response.version = 2;
	response.token = dataprefix.token;
	switch(dataprefix.tag) {
		default:
			response.tag = dataprefix.tag + 1;
			break;
	}

	size_t r = sendto(socket, &response, sizeof(SEMTECH_ACK), 0,
		(const struct sockaddr*) gwAddress,
		((gwAddress->sin_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)));

	int rr = sizeof(SEMTECH_ACK) ? LORA_OK : ERR_CODE_SEND_ACK;
	if (onLog) {
		if (rr) {
			std::stringstream ss;
			ss << ERR_MESSAGE << ERR_CODE_SEND_ACK << " "
				<< UDPSocket::addrString((const struct sockaddr *) gwAddress)
				<< " " << rr << ": " << strerror_client(rr)
				<< ", errno: " << errno << ": " << strerror(errno);
			onLog(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SEND_ACK, ss.str());
		} else {
			std::stringstream ss;
			ss << MSG_SENT_ACK_TO
				<< UDPSocket::addrString((const struct sockaddr *) gwAddress);
			onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
		}
	}
	return rr;
}

void PacketQueue::runner()
{
	// PacketHandler value;
	packetsRead = 0;
	int timeoutMicroSeconds = DEF_TIMEOUT_MS * 1000;

	// mode 0- stopped, 1- running, -1- stop request
	mode = 1;
	while (mode == 1) {
		fd_set fh;
		FD_ZERO(&fh);
		FD_SET(fdWakeup, &fh);
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = timeoutMicroSeconds;
		int retval = select(fdWakeup + 1, &fh, NULL, NULL, &timeout);
		switch (retval) {
			case -1:
				// select error
				if (onLog) {
					std::stringstream ss;
					ss << ERR_MESSAGE << ERR_CODE_SELECT << ": " << ERR_SELECT << ", errno " << errno << ": " << strerror(errno) 
						<< ", handler " << fdWakeup << ", timeout: " << timeoutMicroSeconds;
					onLog(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SELECT, ss.str());
					abort();
				}
				break;
			default:
			// case 0:
				// 0- timeout occured
				if (count()) {
					SemtechUDPPacketItem item;
					struct timeval t;
					gettimeofday(&t, NULL);
					while (getFirstExpired(item, t)) {
						switch (item.processMode)
						{
						case MODE_ACK:
							ack(item.socket, (const sockaddr_in *) &item.packet.gatewayAddress, item.packet.prefix);
							break;
						case MODE_REPLY_MAC:
							if (onLog) {
								std::stringstream ss;
								ss << MSG_SENT_REPLY_TO << UDPSocket::addrString((const sockaddr *) &item.packet.gatewayAddress);
								onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
							}
							break;
						default:
							if (onLog) {
								std::stringstream ss;
								ss << ERR_MESSAGE << ERR_CODE_WRONG_PARAM << ": " << ERR_WRONG_PARAM << " mode: " << (int) mode
									<< ", socket " << UDPSocket::addrString((const sockaddr *) &item.packet.gatewayAddress);
								onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
							}
							break;
						}
						packetsRead++;
						// value.onPacket(p);
					}
					timeoutMicroSeconds = getNextTimeout(t);
				}
				continue;
		}
	}
	// mode 0- stopped, 1- running, -1- stop request
	mode = 0;
}

void PacketQueue::wakeUp() 
{
	// mode 0- stopped, 1- running, -1- stop request
	if (mode != 1)
		return;
	uint64_t u = 1;
	if (write(fdWakeup, &u, sizeof(uint64_t)) != sizeof(uint64_t)) {
		// TODO smth
	}
}

void PacketQueue::start(
	PacketHandler &value
) 
{
	// mode 0- stopped, 1- running, -1- stop request
	if (mode == 1)
		return;

	fdWakeup = eventfd(0, EFD_CLOEXEC);

	threadSend = new std::thread(&PacketQueue::runner, this);
}

void PacketQueue::stop()
{
	// mode 0- stopped, 1- running, -1- stop request
	if (mode == 0)
		return;

	mode = -1;
	wakeUp();

	// mode 0- stopped, 1- running, -1- stop request
	while (mode != 0) {
		usleep(100);
	}

	close(fdWakeup);
	fdWakeup = 0;
}
