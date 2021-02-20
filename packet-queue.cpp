#include "packet-queue.h"
#include <sys/time.h>
#include <unistd.h>

#include "utildate.h"

SemtechUDPPacketItem::SemtechUDPPacketItem(
	const semtechUDPPacket &apacket
)
{
	gettimeofday(&timeAdded, NULL);
	packet = apacket;
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

std::string SemtechUDPPacketsAddr::toString() const
{
	std::stringstream ss;
	for (std::vector <SemtechUDPPacketItem>::const_iterator it(packets.begin()); it != packets.end(); it++) {
		ss << it->toString() << " ";
	}
	return ss.str();
}

PacketQueue::PacketQueue()
	: packetsRead(0), delayMicroSeconds(DEF_DELAY_MS * 1000), isStarted(false), isDone(false)
{
}

PacketQueue::PacketQueue(
	int delayMillisSeconds
)
	: packetsRead(0), isStarted(false), threadSend(NULL)
{
	setDelay(delayMillisSeconds);
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

void PacketQueue::put(
	const semtechUDPPacket &value
) {
	SemtechUDPPacketItem item(value);
	DEVADDRINT a(item.getAddr());
	mutexq.lock();
	std::map<DEVADDRINT, SemtechUDPPacketsAddr>::iterator it(packets.find(a));
	// add first packet, add metadata only for others
	if (it != packets.end()) {
		if (it->second.packets.size() == 0)
			it->second.packets.push_back(item);
		else {
			if (value.metadata.size())
				it->second.packets[0].packet.metadata.push_back(value.metadata[0]);
		}
	} else {
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
	return 1000000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec);
}

size_t PacketQueue::count()
{
	return packets.size();
}

bool PacketQueue::getFirstExpired(
	semtechUDPPacket &retval,
	struct timeval &currenttime
)
{
	if (!addrs.size())
		return false;

	mutexq.lock();

	DEVADDRINT a = addrs.front();
	std::map<DEVADDRINT, SemtechUDPPacketsAddr>::iterator it(packets.find(a));
	if (it == packets.end()) {
		mutexq.unlock();
		return false;
	}

	// always keep at leats 1 item
	if (!it->second.packets.size()) {
		mutexq.unlock();
		return false;
	}
	// first packet is earliest packet
	if (diffMicroSeconds(it->second.packets[0].timeAdded, currenttime) < delayMicroSeconds) {
		mutexq.unlock();
		return false;
	}
	retval = it->second.packets[0].packet;

	packets.erase(it);
	addrs.pop_front();

	mutexq.unlock();

	return true;
}

int PacketQueue::getNextTimeout(struct timeval &currenttime)
{
	DEVADDRINT a = addrs.front();
	std::map<DEVADDRINT, SemtechUDPPacketsAddr>::iterator it(packets.find(a));
	if (it == packets.end()) {
		return DEF_TIMEOUT_MS;
	}

	// always keep at leats 1 item
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
	for (std::map<DEVADDRINT, SemtechUDPPacketsAddr, DEVADDRINTCompare>::const_iterator it(packets.begin()); it != packets.end(); it++) {
		ss << DEVADDRINT2string(it->first) << ": " 
			<< it->second.toString()
			<< std::endl;
	}
	return ss.str();
}

void PacketQueue::runner()
{
	// PacketHandler value;
	packetsRead = 0;
	int timeoutMicroSeconds = DEF_TIMEOUT_MS * 1000;
	while (isStarted) {
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = timeoutMicroSeconds;
		int retval = select(0, NULL, NULL, NULL, &timeout);
		switch (retval) {
			case -1:
				break;
			case 0:
				// timeout
				if (count()) {
					semtechUDPPacket p;
					struct timeval t;
					gettimeofday(&t, NULL);
					while (getFirstExpired(p, t)) {
						/*
						std::cerr << timeval2string(t) << " "  << p.getDeviceAddrStr() << " "
							<< p.metadataToJsonString() << std::endl;
						*/
						packetsRead++;
						// value.onPacket(p);
					}
					timeoutMicroSeconds = getNextTimeout(t);
				}
				continue;
			default:
				break;
		}
	}
	isDone = true;
}

void PacketQueue::start(
	PacketHandler &value
) 
{
	if (isStarted)
		return;
	isStarted = true;
	isDone = false;
	threadSend = new std::thread(&PacketQueue::runner, this);
}

void PacketQueue::stop()
{
	if (!isStarted)
		return;
	isStarted = false;
	while(!isDone) {
		usleep(100);
	}
}
