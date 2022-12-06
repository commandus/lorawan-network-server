#ifdef _MSC_VER
#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <io.h>
#else
#include <sys/time.h>
#include <sys/eventfd.h>
#include <unistd.h>
#endif

#include <sstream>
#include <iostream>

#include "platform.h"
#include "packet-queue.h"
#include "utildate.h"
#include "utilstring.h"
#include "utilthread.h"
#include "errlist.h"
#include "udp-socket.h"
#include "lorawan-mac.h"
#include "identity-service.h"
#include "control-packet.h"
#include "lora-encrypt.h"

SemtechUDPPacketItem::SemtechUDPPacketItem()
	: processMode(MODE_NONE)
{
}

SemtechUDPPacketItem::SemtechUDPPacketItem(
	const SemtechUDPPacket &aPacket
)
	: processMode(MODE_NONE), packet(aPacket)
{
	gettimeofday(&time2send, NULL);
}

SemtechUDPPacketItem::SemtechUDPPacketItem(
	int socket,
	ITEM_PROCESS_MODE mode,
	const struct timeval &time,
	const SemtechUDPPacket &aPacket
)
	: processMode(mode), time2send(time), packet(aPacket)
{
	
}

DEVADDRINT SemtechUDPPacketItem::getAddr() const
{
	return packet.getDeviceAddr();
}

std::string SemtechUDPPacketItem::toJsonString() const
{
	std::stringstream ss;
	ss << "{\"time\": \"" << timeval2string(time2send)
        << "\", \"devaddr\": \"" << packet.getDeviceAddrStr()
        << "\", \"metadata\": " << packet.metadataToJsonString()
        << "}";
	return ss.str();
}

std::string SemtechUDPPacketItem::toString() const
{
    std::stringstream ss;
    ss << "time: " << timeval2string(time2send) << ", device address: " << packet.getDeviceAddrStr()
        << ", metadata: " << packet.metadataToJsonString();
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
      identityService(NULL), deviceHistoryService(NULL), gatewayList(NULL)
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

void PacketQueue::setDeviceHistoryService
(
        DeviceHistoryService *aDeviceStatService
)
{
    deviceHistoryService = aDeviceStatService;
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
	const struct timeval &time2send,
	const SemtechUDPPacket &value
) {
    SemtechUDPPacketItem item(socket, mode, time2send, value);
	DEVADDRINT a(item.getAddr());

	mutexq.lock();
	std::map<DEVADDRINT, SemtechUDPPacketItems>::iterator it(packets.find(a));
	// add first packet, add metadata only for others
	if (it != packets.end()) {
		// there are already some packets to send to the device
		if (it->second.packets.size() == 0)
			// actually, no ;(
			it->second.packets.push_back(item);
		else {
			// sure, there are some packets already!
			// find out same packet (if more than 1)
			bool found = false;
			for (std::vector<SemtechUDPPacketItem>::iterator itp(it->second.packets.begin()); itp != it->second.packets.end(); itp++)
			{
				if ((itp->packet.header.fport == value.header.fport) &&
                    (itp->packet.header.header.fcnt == value.header.header.fcnt))
				{
					// we need metadata only for calc the best gateway with the strongest signal
					if (value.metadata.size()) {
                        // copy gateway MAC address
                        itp->packet.metadata.push_back(rfmMetaData(&value.prefix, value.metadata[0]));
                        itp->packet.header.fopts = item.packet.header.fopts;
                        itp->packet.header.header.fctrl.f.foptslen = item.packet.header.header.fctrl.f.foptslen;
                    }
					found = true;
					break;
				}
			}
			if (!found) {
                // add a new packet as a new one
                packets[a].packets.push_back(item);
                addrs.push_back(a);
            }
		}
	} else {
		// this is first packet received from the device
		packets[a].packets.push_back(item);
		addrs.push_back(a);
	}
	mutexq.unlock();

    if (onLog) {
        std::stringstream ss;
        ss << MSG_PUSH_PACKET_QUEUE << timeval2string(time2send);
        onLog(this, LOG_DEBUG, LOG_PACKET_QUEUE, 0, ss.str());
    }
}

/**
 * Return time difference, in microseconds
 * @param t1 current time
 * @param t2 future time t2 > t1
 * @return time difference in microseconds (>0)
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
	return (int) r;
}

size_t PacketQueue::count()
{
	return packets.size();
}

const int TIME_LEAD_MICROSECONDS = 1000000;

bool PacketQueue::getFirstExpired(
	SemtechUDPPacketItem &retval,
	struct timeval &currentTime
)
{
	if (addrs.empty())
		return false;   // nothing to do

	mutexq.lock();

    // get first address
	DEVADDRINT a = addrs.front();
    // get corresponding packet
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

	// first packet is the earliest packet, check time using first earliest packet
	if (diffMicroSeconds(currentTime, it->second.packets[0].time2send) > TIME_LEAD_MICROSECONDS) {
		// too early. Wait.
		mutexq.unlock();
		return false;
	}

	// get packet with received signal strength indicator. Worst is -85 dBm.
	float lsnr = -3.402823466E+38f;
	uint64_t gwid;
	std::vector<SemtechUDPPacketItem>::const_iterator pit(it->second.packets.begin());
	if (pit == it->second.packets.end()) {
		mutexq.unlock();
		return false;
	}
		
	// validate have another packet (by fcnt)
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
    // return packet
	retval = it->second.packets[idx];

    // remove packet from the queue
	if (hasOtherPacket) {
		// remove first received packet and others with the same fcnt
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

    // remove address from the queue
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
	if (it->second.packets.empty()) {
		return DEF_TIMEOUT_MS * 1000;
	}
	// first packet is the earliest packet
	return diffMicroSeconds(currenttime, it->second.packets[0].time2send);
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
	
	size_t r = sendto(socket, (const char *) &response, sizeof(SEMTECH_ACK), 0,
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
	float snr;
    // get regional parameters
    const RegionalParameterChannelPlan *regionalParameterChannelPlan;
    if (deviceChannelPlan)
        regionalParameterChannelPlan = deviceChannelPlan->get(item.getAddr());
    if (!regionalParameterChannelPlan)
        return ERR_CODE_NO_REGION_BAND;
    int power = (int) regionalParameterChannelPlan->maxUplinkEIRP; //defaultDownlinkTXPower;

    // to reply via the closest gateway, find out gateway with best SNR
    uint64_t gwa = item.packet.getBestGatewayAddress(&snr);
	if (gwa == 0) {
		std::stringstream ss;
		ss << ERR_BEST_GATEWAY_NOT_FOUND;
		if (onLog)
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_BEST_GATEWAY_NOT_FOUND, ss.str());
		return ERR_CODE_BEST_GATEWAY_NOT_FOUND;
	}

	// check just in case gateway and MAC payload
	// .. gateway
	if (!gatewayList)
		return ERR_CODE_WRONG_PARAM;
	// .. MAC
	if (!item.packet.hasMACPayload()){
		std::stringstream ss;
		ss << ERR_NO_MAC << ", packet: " << item.packet.toJsonString();
		if (onLog)
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_GATEWAY_STAT, ss.str());
		return ERR_CODE_NO_MAC;
	}
	
	// find out gateway statistics, required for last gateway port number to send reply
	std::map<uint64_t, GatewayStat>::const_iterator gwit = gatewayList->gateways.find(gwa);
	if (gwit == gatewayList->gateways.end()) {
		std::stringstream ss;
		ss << ERR_GATEWAY_NOT_FOUND << gatewayId2str(gwa);
		if (onLog)
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_GATEWAY_NOT_FOUND, ss.str());
		return ERR_CODE_GATEWAY_NOT_FOUND;
	}

    // check does gateway socket open
    if (gwit->second.socket == 0) {
        std::stringstream ss;
        ss << ERR_GATEWAY_NO_YET_PULL_DATA << gatewayId2str(gwa);
        if (onLog)
            onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_GATEWAY_NO_YET_PULL_DATA, ss.str());
        return ERR_CODE_GATEWAY_NO_YET_PULL_DATA;
    }

	// get MAC commands
	MacPtr macPtr(item.packet.getMACs());
	// print out into log
	std::stringstream ss;
	uint32_t internalTime = item.packet.tmst();

	if (onLog) {
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
        onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
    }

    // make response
	// get identity for NwkS
	DeviceId id;
	if (identityService)
        identityService->get(id, item.packet.header.header.devaddr);
	// Produce MAC command response in the item.packet
	uint32_t fcntdown = 0;
	if (deviceHistoryService) {
		DeviceHistoryItem ds;
		int rs = deviceHistoryService->get(item.packet.header.header.devaddr, ds);
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
	// macPtr.mkRequestMACs(macResponse, item.packet);
	std::string mrp = macResponse.str();

	if (mrp.empty())
		return LORA_OK;

	fcntdown++;

	std::string response = item.packet.mkPullResponse(mrp, id, internalTime, fcntdown, power);
    // std::cerr << "MAC RESPONSE: " << "device addr: " << DEVADDR2string(item.packet.header.header.devaddr) << std::endl << hexString(response) << std::endl;
	size_t r = sendto(gwit->second.socket, response.c_str(), response.size(), 0,
		(const struct sockaddr*) &gwit->second.sockaddr,
		((gwit->second.sockaddr.sin6_family == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)));
	
	if (r == response.size()) {
		if (deviceHistoryService)
			deviceHistoryService->putDown(item.packet.header.header.devaddr, t.tv_sec, fcntdown);
	}

    // log result
	if (onLog) {
		if (r != response.size()) {
			std::stringstream ss;
			ss << ERR_MESSAGE << ERR_CODE_REPLY_MAC << ERR_REPLY_MAC
				<< " socket " << UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr);
			if (r == -1)
				ss << ", sent " << r << " of " << response.size();
			ss << ", errno: " << errno << ": " << strerror(errno);
			if (onLog)
				onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_SEND_ACK, ss.str());
		} else {
			std::stringstream ss;
			ss << MSG_SENT_REPLY_TO
				<< UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr)
				<< " " << MSG_PAYLOAD << ": " << hexString(response) << ", size: " << response.size();
			if (onLog)
				onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
		}
	}
	return LORA_OK;
}

/**
 * Send MAC command response over specified gateway
 * @param item packet
 * @param t current time, not used
 */
int PacketQueue::replyControl(
	SemtechUDPPacketItem &item,
	struct timeval &t
) {

	// to reply via the closest gateway, find out gateway with best SNR

    const RegionalParameterChannelPlan *regionalParameterChannelPlan;
    if (deviceChannelPlan)
        regionalParameterChannelPlan = deviceChannelPlan->get(item.getAddr());
    if (!regionalParameterChannelPlan)
        return ERR_CODE_NO_REGION_BAND;
    int power = (int) regionalParameterChannelPlan->maxUplinkEIRP; //defaultDownlinkTXPower;

	// check just in case
	// .. gateway
	if (!gatewayList)
		return ERR_CODE_WRONG_PARAM;

	size_t sz = item.packet.payload.size();
	if (sz < sizeof(CONTROL_DEVICE_PACKET)) {
		if (onLog) {
			std::stringstream ss;
			ss << ERR_INVALID_CONTROL_PACKET << ", " << MSG_SIZE << ": " << sz << MSG_IS_TOO_SMALL;
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_GATEWAY_STAT, ss.str());
		}
		return ERR_CODE_INVALID_CONTROL_PACKET;
	}

	CONTROL_DEVICE_PACKET *controlPacket = (CONTROL_DEVICE_PACKET *) item.packet.payload.c_str();

	size_t macPayloadSize = sz - sizeof(CONTROL_DEVICE_HEADER);

	std::string macPayload = std::string((const char *) controlPacket->data, macPayloadSize);

    if (controlPacket->header.gwid == 0) {  // special case gateway id = 0 means use the best gateway
        // find out best gateway
        float snr;
        controlPacket->header.gwid = item.packet.getBestGatewayAddress(&snr);
        if (controlPacket->header.gwid == 0) {
            if (onLog) {
                std::stringstream ss;
                ss << ERR_BEST_GATEWAY_NOT_FOUND;
                onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_BEST_GATEWAY_NOT_FOUND, ss.str());
            }
            return ERR_CODE_BEST_GATEWAY_NOT_FOUND;
        } else {
            if (onLog) {
                std::stringstream ss;
                ss << MSG_GATEWAY_SNR << snr << ", gateway: " << uint64_t2string(controlPacket->header.gwid);
                onLog(this, LOG_DEBUG, LOG_PACKET_QUEUE, 0, ss.str());
            }
        }
    }

	if (onLog) {
		std::stringstream ss;
		ss << "Control packet EUI: " << DEVEUI2string(controlPacket->header.eui)
            << ", gateway id: " << uint64_t2string(controlPacket->header.gwid)
            << ", tag: " << (int) controlPacket->header.tag
            << ", MAC " << MSG_PAYLOAD << " size: " << macPayloadSize
            << ", MAC " << MSG_PAYLOAD << ": " << hexString(macPayload);
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_GATEWAY_STAT, ss.str());
	}

	// find out gateway statistics, required for last gateway port number to send reply
	std::map<uint64_t, GatewayStat>::const_iterator gwit = gatewayList->gateways.find(controlPacket->header.gwid);
	if (gwit == gatewayList->gateways.end()) {
		std::stringstream ss;
		ss << ERR_GATEWAY_NOT_FOUND << gatewayId2str(controlPacket->header.gwid);
		if (onLog)
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_GATEWAY_NOT_FOUND, ss.str());
		return ERR_CODE_GATEWAY_NOT_FOUND;
	}
	
	// get MAC commands
	MacPtr macPtr(macPayload, true);
	// print out
	std::stringstream ss;
	ss << MSG_SEND_MAC_REPLY
		<< MSG_BEST_GATEWAY << gatewayId2str(gwit->second.gatewayId) 
		<< " (" << gwit->second.name << ")"
		<< ", address: "
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
	NetworkIdentity nid;
	if (identityService)
		identityService->getNetworkIdentity(nid, controlPacket->header.eui);
	// Produce MAC command response in the item.packet
	uint32_t fCntDown = 0;
	if (deviceHistoryService) {
		DeviceHistoryItem ds;
		int rs = deviceHistoryService->get(nid.devaddr, ds);
		if (rs == 0) {
            fCntDown = ds.fcntdown;
		} else {
			if (onLog) {
				ss << ERR_MESSAGE << ERR_CODE_NO_FCNT_DOWN << ": " << ERR_NO_FCNT_DOWN;
				onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_FCNT_DOWN, ss.str());
			}
		}
	}
	fCntDown++;

    // std::cerr << "SEND MAC command to device addr: " << DEVADDR2string(nid.devaddr) << std::endl;
	
	std::string response = item.packet.mkMACRequest((DEVEUI *) &gwit->second.gatewayId, macPayload, nid, fCntDown, power);

	size_t r = sendto(gwit->second.socket, response.c_str(), response.size(), 0,
		(const struct sockaddr*) &gwit->second.sockaddr,
		((gwit->second.sockaddr.sin6_family == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)));
	
	if (r == response.size()) {
		if (deviceHistoryService)
			deviceHistoryService->putDown(item.packet.header.header.devaddr, t.tv_sec, fCntDown);
        if (onLog) {
            std::stringstream ss;
            ss << "Successfully sent " << hexString(response)
                << ", size: " << response.size()
                << " to " << UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr);
            onLog(this, LOG_DEBUG, LOG_PACKET_QUEUE, 0, ss.str());
        }
    }

	if (onLog) {
		if (r != response.size()) {
			std::stringstream ss;
			ss << ERR_MESSAGE << ERR_CODE_REPLY_MAC << ": " << ERR_REPLY_MAC
				<< " to " << UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr);
			if (r == -1)
				ss << ", sent " << r << " of " << response.size()
					<< ", errno: " << errno << ": " << strerror(errno)
					<< ", " << MSG_PAYLOAD << ": " << hexString(response);
			onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_SEND_ACK, ss.str());
		} else {
			std::stringstream ss;
			ss << MSG_SENT_REPLY_TO
				<< UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr)
				<< " " << MSG_PAYLOAD << ": " << hexString(response) << ", size: " << response.size();
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
		int retval = select(fdWakeup + 1, &fh, nullptr, nullptr, &timeout);
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
		gettimeofday(&t, nullptr);
		while (getFirstExpired(item, t))
        {
			switch (item.processMode)
			{
			case MODE_ACK:
				ack(item.socket, (sockaddr *) &item.packet.gatewayAddress, item.packet.prefix);
				break;
			case MODE_REPLY_MAC:
				replyMAC(item, t);
				break;
            case MODE_JOIN_RESPONSE:
                {
                    int r = replyJoinRequest(item, t);
                    if (r) {
                        if (onLog) {
                            std::stringstream ss;
                            ss << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r);
                            onLog(this, LOG_ERR, LOG_PACKET_QUEUE, r, ss.str());
                        }
                    }
                }
                break;
			case MODE_CONTROL_NS:
				// control packet
				if (onLog) {
					std::stringstream ss;
					ss << "Control message processing, " << MSG_PAYLOAD << ": "
						<< hexString(item.packet.payload)
						<< ", socket " << UDPSocket::addrString((const sockaddr *) &item.packet.gatewayAddress);
					onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
				}
				replyControl(item, t);
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
		// nothing to do
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
    setThreadName(threadSend, MODULE_NAME_PACKET_QUEUE_SEND);
    threadSend->detach();
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

void PacketQueue::setDeviceChannelPlan(const DeviceChannelPlan *value) {
    deviceChannelPlan = value;
}

/**
 * Send Join Accept to Semtech packet forwarder protocol in
 * JOIN_ACCEPT_DELAY1 = 5s or JOIN_ACCEPT_DELAY2 = 6s
 *
 * Called from PacketQueue::runner()
 *
 * @param item Join Request packet
 * @param t time received
 * @return 0- success
 */
int PacketQueue::replyJoinRequest(
        SemtechUDPPacketItem &item,
        struct timeval &t
)
{
    // get Join request
    JOIN_REQUEST_FRAME *joinRequestFrame = item.packet.getJoinRequestFrame();
    // check does it exist
    if (!joinRequestFrame)
        return ERR_CODE_BAD_JOIN_REQUEST;
    // check identity service
    if (!identityService)
        return ERR_CODE_FAIL_IDENTITY_SERVICE;
    // get network identity
    NetworkIdentity nid;
    int r = identityService->getNetworkIdentity(nid, joinRequestFrame->devEUI);
    if (r) {
        // Device not found. Device must be registered in the database before Join procedure
        return ERR_CODE_DEVICE_EUI_NOT_FOUND;
    }
    // Check does Join EUI matched
    if (memcmp(nid.appEUI, joinRequestFrame->joinEUI, sizeof(DEVEUI)) != 0) {
        // Device record is invalid. Set correct Join(App)EUI
        if (onLog) {
            std::stringstream ss;
            ss << ERR_JOIN_EUI_NOT_MATCHED
                << " received Join EUI " << DEVEUI2string(joinRequestFrame->joinEUI)
                << ", expected Join (App) EUI " << DEVEUI2string(nid.appEUI);
            onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_JOIN_EUI_NOT_MATCHED, ss.str());
        }
        return ERR_CODE_JOIN_EUI_NOT_MATCHED;
    }
    // check just in case gateway
    if (!gatewayList)
        return ERR_CODE_WRONG_PARAM;
    // get default regional parameters
    const RegionalParameterChannelPlan *regionalParameterChannelPlan;
    if (deviceChannelPlan)
        regionalParameterChannelPlan = deviceChannelPlan->get();
    if (!regionalParameterChannelPlan) {
        if (onLog)
            onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_REGION_BAND, ERR_NO_REGION_BAND);
        return ERR_CODE_NO_REGION_BAND;
    }

    // find out best gateway by the best SNR
    float snr;
    // to reply via the closest gateway, find out gateway with best SNR
    uint64_t gwa = item.packet.getBestGatewayAddress(&snr);
    if (gwa == 0) {
        std::stringstream ss;
        ss << ERR_BEST_GATEWAY_NOT_FOUND;
        if (onLog)
            onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_BEST_GATEWAY_NOT_FOUND, ss.str());
        return ERR_CODE_BEST_GATEWAY_NOT_FOUND;
    }

    // find out gateway statistics, required for last gateway port number to send reply
    std::map<uint64_t, GatewayStat>::const_iterator gwit = gatewayList->gateways.find(gwa);
    if (gwit == gatewayList->gateways.end()) {
        std::stringstream ss;
        ss << ERR_GATEWAY_NOT_FOUND << gatewayId2str(gwa);
        if (onLog)
            onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_GATEWAY_NOT_FOUND, ss.str());
        return ERR_CODE_GATEWAY_NOT_FOUND;
    }

    // check does gateway socket open
    if (gwit->second.socket == 0) {
        std::stringstream ss;
        ss << ERR_GATEWAY_NO_YET_PULL_DATA << gatewayId2str(gwa);
        if (onLog)
            onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_GATEWAY_NO_YET_PULL_DATA, ss.str());
        return ERR_CODE_GATEWAY_NO_YET_PULL_DATA;
    }

    // get RX1 delay in secs from regional settings

    // get default power from regional settings
    int power = (int) regionalParameterChannelPlan->maxUplinkEIRP; //defaultDownlinkTXPower;

    // log end-device internal time and elected gateway
    std::stringstream ss;
    uint32_t internalTime = item.packet.tmst();

    // make response
    JOIN_ACCEPT_FRAME frame;
    // fill up address in the frame
    int rj = identityService->joinAccept(frame.hdr, nid);
    if (rj) {
        if (onLog) {
            ss << ERR_MESSAGE << rj << ": " << strerror_lorawan_ns(rj);
            onLog(this, LOG_ERR, LOG_PACKET_QUEUE, rj, ss.str());
        }
        // do not reply to the device
        return rj;
    }
    // copy assigned address to the item
    memmove(&item.packet.header.header.devaddr, nid.devaddr, sizeof(DEVADDR));

    // fill up regional settings in the frame
    frame.hdr.rxDelay = regionalParameterChannelPlan->bandDefaults.ReceiveDelay1;
    frame.hdr.dlSettings.RX2DataRate = regionalParameterChannelPlan->bandDefaults.RX2DataRate;
    frame.hdr.dlSettings.RX1DROffset = 0;
    // optNeg = 0: LoraWAN v.1.0
    // The device derives FNwkSIntKey & AppSKey from the NwkKey
    // The device sets SNwkSIntKey & NwkSEncKey equal to FNwkSIntKey
    frame.hdr.dlSettings.optNeg = 0;
    // The MIC value of the join-accept message is calculated as follows: 2
    // cmac = aes128_cmac(NwkKey, MHDR | JoinNonce | NetID | DevAddr | DLSettings | RxDelay | CFList )

    // set header
    frame.mhdr.f.mtype = MTYPE_JOIN_ACCEPT;
    frame.mhdr.f.rfu = 0;
    frame.mhdr.f.major = LORAWAN_MAJOR_VERSION; // 0

    KEY128 JSIntKey;

    // Calculate MIC starting with header
    if (frame.hdr.dlSettings.optNeg == 0) {
        // version 1.0 uses NwkKey
        frame.mic = calculateMICJoinResponse(frame, nid.nwkKey);
    } else {
        // version 1.1 uses derived key
        deriveJSIntKey(JSIntKey, nid.nwkKey, nid.devEUI);
        frame.mic = calculateOptNegMICJoinResponse(frame,
           nid.devEUI,  // ?!!
           nid.devNonce,
           JSIntKey
        );
    }

    uint32_t fcntDown = 0;
    if (deviceHistoryService) {
        DeviceHistoryItem ds;
        int rs = deviceHistoryService->get(item.packet.header.header.devaddr, ds);
        if (rs == 0) {
            fcntDown = ds.fcntdown;
        } else {
            if (onLog) {
                ss << ERR_MESSAGE << ERR_CODE_NO_FCNT_DOWN << ": " << ERR_NO_FCNT_DOWN;
                onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_NO_FCNT_DOWN, ss.str());
            }
            return ERR_CODE_NO_FCNT_DOWN;
        }
    }

    if (frame.hdr.dlSettings.optNeg == 0) {
        // version 1.0 uses NwkKey
        encryptJoinAcceptResponse(frame, nid.nwkKey);
    } else {
        encryptJoinAcceptResponse(frame, JSIntKey);
    }
    item.packet.header.header.fcnt = fcntDown;
    // item.packet.header.header.macheader.f.mtype = MTYPE_JOIN_ACCEPT;
    std::string response = item.packet.mkJoinAcceptResponse(frame, internalTime, power);
    size_t sz = sendto(gwit->second.socket, response.c_str(), response.size(), 0,
          (const struct sockaddr*) &gwit->second.sockaddr,
          ((gwit->second.sockaddr.sin6_family == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)));

    if (sz == response.size()) {
        if (deviceHistoryService)
            deviceHistoryService->putDown(item.packet.header.header.devaddr, t.tv_sec, fcntDown);
    }

    // log result
    if (onLog) {
        if (sz != response.size()) {
            std::stringstream ss;
            ss << ERR_MESSAGE << ERR_CODE_REPLY_MAC << ": " << ERR_REPLY_MAC
                    << ", tmst: " << internalTime
                    << ", " << MSG_BEST_GATEWAY << gatewayId2str(gwit->second.gatewayId)
                    << " (" << gwit->second.name << ")"
                    << MSG_GATEWAY_SNR << snr << ", address: "
                    << UDPSocket::addrString((const sockaddr *) &gwit->second.sockaddr)
                    << ", device address: " << DEVADDR2string(item.packet.header.header.devaddr)
                    << ", " << MSG_PAYLOAD << ": " << hexString(response) << " (" << response.size()
                    << " bytes) to " << UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr);
            if (r == -1)
                ss << ", sent " << r << " bytes of " << response.size();
            if (errno)
                ss << ", system errno: " << errno << ": " << strerror(errno);
            if (onLog)
                onLog(this, LOG_ERR, LOG_PACKET_QUEUE, ERR_CODE_SEND_ACK, ss.str());
        } else {
            std::stringstream ss;
            // log assigned network address
            if (onLog) {
                ss << MSG_SEND_JOIN_REQUEST_REPLY
                   << "tmst: " << internalTime
                   << ", " << MSG_BEST_GATEWAY << gatewayId2str(gwit->second.gatewayId)
                   << " (" << gwit->second.name << ")"
                   << MSG_GATEWAY_SNR << snr << ", address: "
                   << UDPSocket::addrString((const sockaddr *) &gwit->second.sockaddr)
                   << UDPSocket::addrString((const struct sockaddr *) &gwit->second.sockaddr)
                   << ", device address: " << DEVADDR2string(item.packet.header.header.devaddr)
                   << ", " << MSG_PAYLOAD << ": " << hexString(response) << ", size: " << response.size();
                onLog(this, LOG_INFO, LOG_PACKET_QUEUE, 0, ss.str());
            }
        }
    }
    return LORA_OK;
}
