#include "packet-queue.h"
#include <sys/time.h>

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
	: delayMS(MIN_DELAY_MS)
{
}

PacketQueue::PacketQueue(
	int delayms
)
{
	delayMS = delayms;
	if (delayMS < MIN_DELAY_MS)
		delayMS = MIN_DELAY_MS;
	if (delayMS > MAX_DELAY_MS)
		delayMS = MAX_DELAY_MS;
}

void PacketQueue::put(
	const semtechUDPPacket &value
) {
	SemtechUDPPacketItem item(value);
	DEVADDRINT a(item.getAddr());
	std::map<DEVADDRINT, SemtechUDPPacketsAddr>::iterator it(packets.find(a));
	if (it != packets.end()) {
		it->second.packets.push_back(item);
	} else {
		packets[a].packets.push_back(item);
		addrs.push_back(a);
	}
}

/**
 * @return time dirrerence in milliseconds 
 **/
int PacketQueue::diffMS(
	struct timeval &t1,
	struct timeval &t2
)
{
	return ((t2.tv_sec - t1.tv_sec) * 1000) + ((t2.tv_usec - t1.tv_usec) / 1000);
}

bool PacketQueue::getFirstExpired(
	semtechUDPPacket &retval,
	struct timeval &currenttime
)
{
	if (!addrs.size())
		return false;
	DEVADDRINT a = addrs.front();
	std::map<DEVADDRINT, SemtechUDPPacketsAddr>::iterator it(packets.find(a));
	if (it == packets.end())
		return false;

	if (diffMS(it->second.packets[0].timeAdded, currenttime) < delayMS)
		return false;
	
	retval = it->second.packets[0].packet;
	packets.erase(it);
	addrs.pop_front();

	return true;
}
