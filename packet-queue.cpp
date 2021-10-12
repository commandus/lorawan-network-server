#include "packet-queue.h"
#include <sys/time.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <iostream>

#include "utildate.h"
#include "utilstring.h"
#include "errlist.h"
#include "udp-socket.h"
#include "lorawan-mac.h"
#include "identity-service-abstract.h"

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
	: packetsRead(0), delayMicroSeconds(DEF_DELAY_MS * 1000), mode(0), fdWakeup(0), onLog(NULL), identityService(NULL), gatewayList(NULL)
{
}

PacketQueue::PacketQueue(
	int delayMillisSeconds
)
	: packetsRead(0), mode(0), threadSend(NULL), fdWakeup(0), onLog(NULL), 
	identityService(NULL), deviceStatService(NULL), gatewayList(NULL)
{
	setDelay(delayMillisSeconds);
}

void PacketQueue::setIdentityService
(
	IdentityService* value
)
{
	identityService = value;
}

void PacketQueue::setDeviceStatService
(
	DeviceStatService *value
)
{
	deviceStatService = value;
}

void PacketQueue::setGatewayList
(
	GatewayList *value
)
{
	gatewayList = value;
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
	if (onLog) {
		std::stringstream ss;
		ss << MSG_PUSH_PACKET_QUEUE << timeval2string(time);
		onLog(this, LOG_DEBUG, LOG_PACKET_QUEUE, 0, ss.str());
	}

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
						// copy gateway MAC address
						itp->packet.metadata.push_back(rfmMetaData(&value.prefix, value.metadata[0]));
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
 * @param t1 current time
 * @param t2 furure time
 * @return time dirrerence in microseconds (>0)
 **/
int PacketQueue::diffMicroSeconds(
	struct timeval &t1,
	struct timeval &t2
)
{
	int ds = t2.tv_sec - t1.tv_sec;
	if (ds > 2147)
		return 2147483647;	// return max int
	if (ds < -2147)
		return 0;	// return min positive int
	int64_t r = 1000000 * ds + (t2.tv_usec - t1.tv_usec);
	if (r < 0)
		r = 0;
	return r;
}

size_t PacketQueue::count()
{
	return packets.size();
}

const int TIME_LEAD_MICROSECONDS = 1000;

bool PacketQueue::getFirstExpired(
	SemtechUDPPacketItem &retval,
	struct timeval &currenttime
)
{
	if (!addrs.size()) {
		return false;
	}

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
	if (diffMicroSeconds(currenttime, it->second.packets[0].timeAdded) > TIME_LEAD_MICROSECONDS) {
		// not ready, wait
		mutexq.unlock();
		return false;
	}

	// return packet with received signal strength indicator. Worst is -85 dBm.
	float lsnr = -3.402823466E+38;
	uint64_t gwid;
	std::vector<SemtechUDPPacketItem>::const_iterator pit(it->second.packets.begin());
	if (pit == it->second.packets.end()) {
		mutexq.unlock();
		return false;
	}
		
	// validate have an other packet (by fcnt)
	uint16_t fcntFirst = pit->packet.header.header.fcnt;
	bool hasOtherPacket = false;
	int idx = 0;
	int i = 0;
	pit++;
	for (; pit != it->second.packets.end(); pit++) {
		if (pit->packet.header.header.fcnt != fcntFirst) {
			hasOtherPacket = true;
		} else {
			uint64_t bgwid;
			float bsnr;
			bgwid = pit->packet.getBestGatewayAddress(&bsnr);
			if (bgwid != 0 && bsnr > lsnr) {
				lsnr = bsnr;
				gwid = bgwid;
				idx = i;
			}
		}
		i++;
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

/**
 * Return time wait until time ready to send next packet
 * @param currenttime current time
 * @return microseconds to serve next packet from the queue
 */
int PacketQueue::getNextTimeout(struct timeval &currenttime)
{
	DEVADDRINT a = addrs.front();
	std::map<DEVADDRINT, SemtechUDPPacketItems>::iterator it(packets.find(a));
	if (it == packets.end()) {
		return DEF_TIMEOUT_MS * 1000;
	}

	// always keep at least 1 item
	if (!it->second.packets.size()) {
		return DEF_TIMEOUT_MS * 1000;
	}
	// first packet is earliest packet
	return diffMicroSeconds(currenttime, it->second.packets[0].timeAdded);
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
	struct sockaddr* gwAddress,
	const SEMTECH_PREFIX_GW &dataprefix
)
{
	SEMTECH_ACK response;
	response.version = 2;
	response.token = dataprefix.token;
	switch(dataprefix.tag) {
		case 2:	// PULL_DATA
			response.tag = 4;	// PULL_ACK, PULL_RESP = 3
			break;
		default:
			response.tag = dataprefix.tag + 1;
			break;
	}
	
	size_t r = sendto(socket, &response, sizeof(SEMTECH_ACK), 0,
		(const struct sockaddr*) gwAddress,
		((gwAddress->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)));

	int rr = sizeof(SEMTECH_ACK) ? LORA_OK : ERR_CODE_SEND_ACK;
	if (onLog) {
		if (rr) {
			std::stringstream ss;
			ss << ERR_MESSAGE << ERR_CODE_SEND_ACK << " "
				<< UDPSocket::addrString((const struct sockaddr *) gwAddress)
				<< " " << rr << ": " << strerror_lorawan_ns(rr)
				<< ", errno: " << errno << ": " << strerror(errno);
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_SEND_ACK, ss.str());
		} else {
			std::stringstream ss;
			ss << MSG_SENT_ACK_TO
				<< UDPSocket::addrString((const struct sockaddr *) gwAddress)
				<< ", tag: " << (int) response.tag
				<< ", token: " << std::hex << dataprefix.token
				<< ", data: " <<  hexString(std::string((const char *) &response, sizeof(SEMTECH_ACK)));
			onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
		}
	}
	return rr;
}

/**
 * Send MAC command response, to the best gateway over UDP socket
 * @param item packet
 * @param t current time, not used
 */
int PacketQueue::replyMAC(
	SemtechUDPPacketItem &item,
	struct timeval &t
) {
	// to reply via closest gateway, find out gatewsy with best SNR
	float snr;
	int power = 14;
	uint64_t gwa = item.packet.getBestGatewayAddress(&snr);
	if (gwa == 0) {
		std::stringstream ss;
		ss << ERR_BEST_GATEWAY_NOT_FOUND;
		if (onLog)
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_BEST_GATEWAY_NOT_FOUND, ss.str());
		return ERR_CODE_BEST_GATEWAY_NOT_FOUND;
	}

	// check just in case
	// .. gateway
	if (!gatewayList)
		return ERR_CODE_WRONG_PARAM;
	// .. MAC
	if (!item.packet.hasMACPayload()){
		std::stringstream ss;
		ss << ERR_NO_MAC;
		if (onLog)
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_GATEWAY_STAT, ss.str());
		return ERR_CODE_NO_MAC;
	}
	
	// find out gateway statistics, required for last gateway port number to send reply
	std::map<uint64_t, GatewayStat>::const_iterator gwit = gatewayList->gateways.find(gwa);
	if (gwit == gatewayList->gateways.end()) {
		std::stringstream ss;
		ss << ERR_NO_GATEWAY_STAT << " " << gatewayId2str(gwa);
		if (onLog)
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_GATEWAY_STAT, ss.str());
		return ERR_CODE_NO_GATEWAY_STAT;
	}
	
	// get MAC commands
	MacPtr macPtr(item.packet.payload);
	// print out
	std::stringstream ss;
	uint32_t internalTime = item.packet.tmst();
	ss << MSG_SEND_MAC_REPLY
		<< " tmst: " << internalTime
		<< ", "  << MSG_BEST_GATEWAY << gatewayId2str(gwit->second.gatewayId) 
		<< " (" << gwit->second.name << ")"
		<< MSG_GATEWAY_SNR  << snr << ", address: "
		<< UDPSocket::addrString((const sockaddr *) &gwit->second.sockaddr);

	ss << ", \"mac\": " << macPtr.toJSONString();
	if (macPtr.errorcode) {
		ss << ", \"mac_error_code\": " << macPtr.errorcode
			<< ", \"mac_error\": \"" << strerror_lorawan_ns(macPtr.errorcode) << "\"";
	}
	if (onLog)
		onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
	// make response

	// get identity for NwkS
	DeviceId id;
	if (identityService)
		identityService->get(item.packet.header.header.devaddr, id);
	// Produce MAC command response in the item.packet
	uint32_t fcntdown = 0;
	if (deviceStatService) {
		DeviceStat ds;
		int rs = deviceStatService->get(item.packet.header.header.devaddr, ds);
		if (rs == 0) {
			fcntdown = ds.fcntdown;
		} else {
			if (onLog) {
				ss << ERR_MESSAGE << ERR_CODE_NO_FCNT_DOWN << ": " << ERR_NO_FCNT_DOWN;
				onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_FCNT_DOWN, ss.str());
			}
		}
	}

	std::stringstream macResponse;
	// Get response on MAC commands
	macPtr.mkResponseMACs(macResponse, item.packet);
	// Add MAR request from server-side (if exists)
	macPtr.mkRequestMACs(macResponse, item.packet);
	std::string mrp = macResponse.str();

	if (mrp.empty())
		return LORA_OK;

	fcntdown++;

	std::string response = item.packet.mkPullResponse(mrp, id, internalTime, fcntdown, power);
std::cerr << "==MAC RESPONSE: " << "device addr: " << DEVADDR2string(item.packet.header.header.devaddr) << std::endl;
std::cerr << "==MAC RESPONSE: " << hexString(response) << std::endl;
	size_t r = sendto(gwit->second.socket, response.c_str(), response.size(), 0,
		(const struct sockaddr*) &gwit->second.sockaddr,
		((gwit->second.sockaddr.sin6_family == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)));
	
	if (r == response.size()) {
		if (deviceStatService)
			deviceStatService->putDown(item.packet.header.header.devaddr, t.tv_sec, fcntdown);
	}

	if (onLog) {
		if (r != response.size()) {
			std::stringstream ss;
			ss << ERR_CODE_REPLY_MAC
				<< UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr);
			if (r == -1)
				ss << ", sent " << r << " of " << response.size();
			ss << ", errno: " << errno << ": " << strerror(errno);
			if (onLog)
				onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_SEND_ACK, ss.str());
		} else {
			std::stringstream ss;
			ss << MSG_SENT_REPLY_TO
				<< UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr)
				<< " payload: " << hexString(response) << ", size: " << response.size();
			if (onLog)
				onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
		}
	}
	return LORA_OK;
}

void PacketQueue::runner()
{
	// PacketHandler value;
	packetsRead = 0;
	int timeoutMicroSeconds = DEF_TIMEOUT_MS * 1000;

	// mode 0- stopped, 1- running, -1- stop request
	mode = 1;
	fd_set fh;
	while (mode == 1) {
		FD_ZERO(&fh);
		FD_SET(fdWakeup, &fh);
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = timeoutMicroSeconds;
		int retval = select(fdWakeup + 1, &fh, NULL, NULL, &timeout);
		if (retval == -1) {
			// select error
			if (onLog) {
				std::stringstream ss;
				ss << ERR_MESSAGE << ERR_CODE_SELECT << ": " << ERR_SELECT << ", errno " << errno << ": " << strerror(errno) 
					<< ", handler " << fdWakeup << ", timeout: " << timeoutMicroSeconds;
				onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_SELECT, ss.str());
			}
			abort();
		}
		if (FD_ISSET(fdWakeup, &fh)) {
			if (onLog) {
				std::stringstream ss;
				ss << "wakeup is set, reset";
				onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
			}
			uint8_t u = 1;
			while (read(fdWakeup, &u, sizeof(u)) == sizeof(u));
		}

		if (!count())
			continue;

		SemtechUDPPacketItem item;
		struct timeval t;
		gettimeofday(&t, NULL);
		while (getFirstExpired(item, t)) {
			switch (item.processMode)
			{
			case MODE_ACK:
				ack(item.socket, (sockaddr *) &item.packet.gatewayAddress, item.packet.prefix);
				break;
			case MODE_REPLY_MAC:
				replyMAC(item, t);
				break;
			default:
				if (onLog) {
					std::stringstream ss;
					ss << ERR_MESSAGE << ERR_CODE_WRONG_PARAM << ": " << ERR_WRONG_PARAM << " mode: " << (int) mode
						<< ", socket " << UDPSocket::addrString((const sockaddr *) &item.packet.gatewayAddress);
					onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
				}
				break;
			}
			packetsRead++;
		}

		gettimeofday(&t, NULL);
		timeoutMicroSeconds = getNextTimeout(t);
		if (onLog) {
			std::stringstream ss;
			ss << "next timeout: " << timeoutMicroSeconds << " microseconds, retval " << retval << std::endl;
			onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
		}
	}
	mode = 0;	// mode 0- stopped, 1- running, -1- stop request
}

void PacketQueue::wakeUp() 
{
	// mode 0- stopped, 1- running, -1- stop request
	if (mode != 1)
		return;
	uint8_t u = 1;
	if (write(fdWakeup, &u, sizeof(u)) != sizeof(u)) {
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
