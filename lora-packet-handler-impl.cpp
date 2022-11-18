#include "lora-packet-handler-impl.h"
#include "udp-socket.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"
#include "device-channel-plan.h"

#include <iostream>

// tme window delay, s
#define DEF_TIME_WINDOW_1	1
#define DEF_TIME_WINDOW_2	2
#define DEF_TIME_WINDOW_2	2
// default queue timeout in microseconds, less than 1s, e.g. 1/4s
#define DEF_TIMEOUT_US	250 * 1000

int LoraPacketProcessor::enqueuePayload(
    const struct timeval &time,
    SemtechUDPPacket &value
)
{
	std::stringstream ss;
	std::string p = value.payload;
	if (!receiverQueueService) {
        if (onLog) {
            ss << ERR_MESSAGE
               << ERR_CODE_INIT_QUEUE << ": " << ERR_INIT_QUEUE;
            onLog(this, LOG_ERR, LOG_PACKET_HANDLER, ERR_CODE_INIT_QUEUE, ss.str());
        }
        return ERR_CODE_INIT_QUEUE;
    }
    // ReceiverQueueService deduplicate repeated packets received from gateways and then store data to the database asynchronously
    if (receiverQueueService->push(value, time)) {
        if (onLog) {
            ss << MSG_ENQUEUE_DB
               << " (queue size: " << receiverQueueService->count()
               << ") " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
               //		<< " " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.devEUI)
               << " " << value.devId.toJsonString() << ": " << hexString(p);
            onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
        }
    } else {
        if (onLog) {
            std::stringstream ss2;
            ss2 << ERR_DUPLICATED_PACKET
                << " " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
                //		<< " " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.devEUI)
                // << " FCnt: " << value.getRfmHeader()->fcnt
                << " RFM: " << RFMHeader(*value.getRfmHeader()).toJson()
                << ", device: " << value.devId.toJsonString() << ", data: " << hexString(p);
            onLog(this, LOG_INFO, LOG_PACKET_HANDLER, ERR_CODE_DUPLICATED_PACKET, ss2.str());
        }
    }
	return 0;
}

/**
 * Enqueue control network service message to send MAC commands to the destination device
 * or control network service itself
 * @param time receive time
 * @param value Semtech packet
 * @return 0- success
 */ 
int LoraPacketProcessor::enqueueControl(
    const struct timeval &time,
    SemtechUDPPacket &value
)
{
	std::stringstream ss;
	ss << MSG_MAC_COMMAND_RECEIVED
       << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
       << ", " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.devEUI) << ", "
       << MSG_PAYLOAD << ": " << hexString(value.payload);
	onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

	// wait until gateways all send packet
	struct timeval t;
	t.tv_sec = time.tv_sec;
	t.tv_usec = time.tv_usec;
	incTimeval(t, 0, DEF_TIMEOUT_US);
	packetQueue.push(0, MODE_CONTROL_NS, t, value);
	packetQueue.wakeUp();
	return LORA_OK;
}

/**
 * @param time receive time
 * @param value Semtech packet
 * @return 0- success
 */
int LoraPacketProcessor::putMACRequests(
        const struct timeval &time,
        SemtechUDPPacket &value
)
{
    std::string ms = "\6";
    value.appendMACs(ms);
    std::stringstream ss;
    ss << "Put MAC commands " << hexString(ms) << ", size " << ms.size() << ", MACs: "
        << hexString(value.getMACs())  << MSG_TO_BE_SEND_TO
        << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
        << ", " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.devEUI);
    onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

    return LORA_OK;
}

/**
 * @param time receive time
 * @param value Semtech packet
 * @return 0- success
 */ 
int LoraPacketProcessor::enqueueMAC(
    const struct timeval &time,
    SemtechUDPPacket &value
)
{
	std::stringstream ss;
	std::string macs = value.getMACs();
	ss << MSG_MAC_COMMAND_RECEIVED
       << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
       << ", " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.devEUI) << ", "
		// << " " << value.devId.toJsonString()
        << ", " << MSG_PAYLOAD << ": " << hexString(value.payload)
        << ", MACs: " << hexString(macs);
	onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
	// wait until gateways all send packet
	struct timeval t;
	t.tv_sec = time.tv_sec;
	t.tv_usec = time.tv_usec;
	incTimeval(t, 0, DEF_TIMEOUT_US);
	packetQueue.push(0, MODE_REPLY_MAC, t, value);
	packetQueue.wakeUp();
	return LORA_OK;
}

/**
 * Enqueue Join Request to be replied in 5s or 6s
 * @param time time received Join Request
 * @param value Join Request packet
 * @return 0- success
 */
int LoraPacketProcessor::enqueueJoinResponse(
    const struct timeval &time,
    const DEVADDR &addr,
    SemtechUDPPacket &value
)
{
    std::stringstream ss;
    std::string macs = value.getMACs();
    const JOIN_REQUEST_FRAME *joinRequestFrame = value.getJoinRequestFrame();

    // delay time
    timeval t;

    // 5s window
    setJoinAcceptDelay(t, value, time, true);
    if (deviceHistoryService)
        value.header.header.fcnt = deviceHistoryService->incrementDown(addr, time.tv_sec);  // downstream from the network server to the device
    packetQueue.push(0, MODE_JOIN_RESPONSE, t, value);

    // 6s window
    if (deviceHistoryService)
        value.header.header.fcnt = deviceHistoryService->incrementDown(addr, time.tv_sec);  // downstream from the network server to the device
    setJoinAcceptDelay(t, value, time, false);
    packetQueue.push(0, MODE_JOIN_RESPONSE, t, value);

    packetQueue.wakeUp();

    // log Join event
    ss << MSG_ENQUEUE_JOIN_REQUEST << MSG_TO_REQUEST
       << JOIN_REQUEST_FRAME2string(*joinRequestFrame)
       << ", gateway address: " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
       << ", " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.devEUI)
       << ", received time: " << timeval2string(time);
    onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

    return LORA_OK;
}

LoraPacketProcessor::LoraPacketProcessor()
	: reservedFPort(0), identityService(nullptr), gatewayList(nullptr), onLog(nullptr), receiverQueueService(nullptr),
      receiverQueueProcessor(nullptr)
{
	packetQueue.start(*this);
}

LoraPacketProcessor::~LoraPacketProcessor()
{
	packetQueue.stop();
	if (receiverQueueProcessor)
		receiverQueueProcessor->stop();
}

// send ACK via queue
int LoraPacketProcessor::ack
(
	int socket,
	const sockaddr_in* gwAddress,
	const SEMTECH_PREFIX_GW &dataprefix
)
{
	packetQueue.ack(socket, (struct sockaddr *) gwAddress, dataprefix);
	/*
	SemtechUDPPacket p((const struct sockaddr *) gwAddress, &dataprefix, NULL, "", identityService);
	struct timeval t = { 0, 0 };
	packetQueue.push(socket, MODE_ACK, t, p);
	packetQueue.wakeUp();
	*/
	return LORA_OK;
}

/**
 * Identify device, if device identified successfully, enqueueTxPacket packet or MAC
 * @param time received time
 * @param packet Semtech gateway packet
 */ 
int LoraPacketProcessor::put(
    const struct timeval &time,
    SemtechUDPPacket &packet
)
{
	DEVADDR addr;
	memmove(&addr, &packet.getHeader()->header.devaddr, sizeof(DEVADDR));

	if (!identityService) {
		if (onLog) {
			// report error
			std::stringstream ss;
			ss << ERR_MESSAGE << ERR_CODE_INIT_IDENTITY << ": " << ERR_INIT_IDENTITY
				<< ", " << MSG_DEVICE_EUI << DEVADDR2string(addr)
				<< ", " << UDPSocket::addrString((const struct sockaddr *) &packet.gatewayAddress);
			onLog(this, LOG_ERR, LOG_IDENTITY_SVC, ERR_CODE_INIT_IDENTITY, ss.str());
		}
		return ERR_CODE_INIT_IDENTITY;
	}

	// try to identify end-device
	int r = identityService->get(packet.devId, addr);
	if (r == 0) {
		if (reservedFPort != 0 && packet.header.fport == reservedFPort) {
            // reserved for control packet FPort number matched
			// check is device authorized to control network service
			if (identityService->canControlService(addr)) {
				// device granted
				if (onLog) {
                    // log event
					std::stringstream ss;
					ss << MSG_RECEIVED_CONTROL_FRAME
						<< ", " << MSG_DEVICE_EUI << DEVADDR2string(addr)
						<< ", " << UDPSocket::addrString((const struct sockaddr *) &packet.gatewayAddress);
					onLog(this, LOG_DEBUG, LOG_IDENTITY_SVC, 0, ss.str());
                    // enqueueTxPacket packet route to the target device or network server itself
					enqueueControl(time, packet);
				}
			} else {
				// device denied, report error
				if (onLog) {
					std::stringstream ss;
					ss << ERR_MESSAGE << ERR_CODE_CONTROL_NOT_AUTHORIZED << ": " << ERR_CONTROL_NOT_AUTHORIZED
						<< ", " << MSG_DEVICE_EUI << DEVADDR2string(addr)
						<< ", " << UDPSocket::addrString((const struct sockaddr *) &packet.gatewayAddress);
					onLog(this, LOG_ERR, LOG_IDENTITY_SVC, ERR_CODE_INIT_IDENTITY, ss.str());
				}
			}
		} else {
			// device has been identified
			if (deviceHistoryService)				// collect statistics if statistics collector is running
				deviceHistoryService->putUp(addr, time.tv_sec, packet.header.header.fcnt);
			if (packet.hasApplicationPayload()) {   // store payload to the database(s) if exists
                enqueuePayload(time, packet);
            }

			if (packet.hasMACPayload()) {
                // provide MAC reply to the end-device if MAC command present in the packet
                enqueueMAC(time, packet);
            } else {
                // request MAC
                putMACRequests(time, packet);
            }
		}
	} else {
		// device id is NOT identified
		if (onLog) {
			// report error
			std::stringstream ss;
			ss << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r)
				<< ", " << MSG_DEVICE_EUI << DEVADDR2string(addr)
				<< ", " << UDPSocket::addrString((const struct sockaddr *) &packet.gatewayAddress);
			onLog(this, LOG_ERR, LOG_IDENTITY_SVC, r, ss.str());
		}
	}
	return r;
}

void LoraPacketProcessor::setIdentityService
(
	IdentityService* value
)
{
	identityService = value;
	packetQueue.setIdentityService(value);
}

void LoraPacketProcessor::setGatewayList
(
	GatewayList *value
)
{
	gatewayList = value;
	packetQueue.setGatewayList(value);
}

void LoraPacketProcessor::setReceiverQueueService
(
	ReceiverQueueService *value
)
{
	receiverQueueService = value;
	if (receiverQueueProcessor) {
		if (receiverQueueService)
			receiverQueueProcessor->start(receiverQueueService);
		else
			receiverQueueProcessor->stop();
	}
}

void LoraPacketProcessor::setDeviceHistoryService(
        DeviceHistoryService *aDeviceHistoryService
)
{
    deviceHistoryService = aDeviceHistoryService;
    packetQueue.setDeviceHistoryService(aDeviceHistoryService);
}

void LoraPacketProcessor::setLogger(
	std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value)
{
	onLog = value;
	packetQueue.setLogger(value);
}

void LoraPacketProcessor::setReceiverQueueProcessor
(
    ReceiverQueueProcessor *value
)
{
	if (receiverQueueProcessor)
		receiverQueueProcessor->stop();
    receiverQueueProcessor = value;
	if (receiverQueueProcessor) {
		if (receiverQueueService)
			receiverQueueProcessor->start(receiverQueueService);
		else
			receiverQueueProcessor->stop();
	}
}

void LoraPacketProcessor::addTimeWindow1
(
	struct timeval &value
)
{
	incTimeval(value, DEF_TIME_WINDOW_1);
}

void LoraPacketProcessor::addTimeWindow2
(
	struct timeval &value
)
{
	incTimeval(value, DEF_TIME_WINDOW_2);
}

// Reserve FPort number for network service purposes
void LoraPacketProcessor::reserveFPort
(
	uint8_t value
)
{
	reservedFPort = value;
}

void LoraPacketProcessor::setDeviceChannelPlan(const DeviceChannelPlan *value)
{
    deviceChannelPlan = value;
    packetQueue.setDeviceChannelPlan(value);
}

/**
 * Prepare response to the Join Request.
 * Check does device identifier exists, if it does,
 * enqueueTxPacket Join Request to be replied in 5s or 6s
 *
 * Called from UDPListener::parseBuffer()
 *
 * @param time time received
 * @param socket network socket to the gateway
 * @param socketAddress gateway address
 * @param packet Join Request packet
 * @return 0- success
 */
int LoraPacketProcessor::join(
        const struct timeval &time,
        int socket,
        const sockaddr_in *socketAddress,
        SemtechUDPPacket &packet
)
{
    if (!identityService) {
        if (onLog) {
            // report error
            std::stringstream ss;
            ss << ERR_MESSAGE << ERR_CODE_INIT_IDENTITY << ": " << ERR_INIT_IDENTITY;
            onLog(this, LOG_ERR, LOG_IDENTITY_SVC, ERR_CODE_INIT_IDENTITY, ss.str());
        }
        return ERR_CODE_INIT_IDENTITY;
    }

    JOIN_REQUEST_FRAME *joinRequestFrame = packet.getJoinRequestFrame();
    if (!joinRequestFrame) {
        if (onLog) {
            // report error
            std::stringstream ss;
            ss << ERR_MESSAGE << ERR_CODE_BAD_JOIN_REQUEST << ": " << ERR_BAD_JOIN_REQUEST
                << ", " << MSG_PAYLOAD << ": " << hexString(packet.payload)
                << " (" << packet.payload.size() << " " << MSG_BYTES << ")."
                << " " << MSG_EXPECTED << " " << sizeof(JOIN_REQUEST_FRAME) << " " << MSG_BYTES;


            onLog(this, LOG_ERR, LOG_IDENTITY_SVC, ERR_CODE_INIT_IDENTITY, ss.str());
        }
        return ERR_CODE_BAD_JOIN_REQUEST;
    }

    // try to identify end-device
    NetworkIdentity networkIdentity;
    int r = identityService->getNetworkIdentity(networkIdentity, joinRequestFrame->devEUI);
    if (r == 0) {
        // device has been identified
        // set device identifier
        packet.devId = networkIdentity;
        // enqueueTxPacket response to be replied in 5s or 6s
        enqueueJoinResponse(time, networkIdentity.devaddr, packet);
    } else {
        // device EUI is NOT identified
        if (onLog) {
            // report error
            std::stringstream ss;
            ss << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r)
                << ". " << MSG_JOIN_REQUEST
                << " " << MSG_DEVICE_EUI << DEVEUI2string(joinRequestFrame->devEUI)
                << " " << MSG_JOIN_EUI << DEVEUI2string(joinRequestFrame->joinEUI)
                << " " << MSG_DEV_NONCE << DEVNONCE2string(joinRequestFrame->devNonce)
                << ", gateway address " << UDPSocket::addrString((const struct sockaddr *) &packet.gatewayAddress);
            onLog(this, LOG_ERR, LOG_IDENTITY_SVC, r, ss.str());
        }
    }
    return r;
}

/**
 * Set time to send, packet's tmst and frequency
 * @param retval return time to send
 * @param value modify Semtech packet tmst and frequency
 * @param time recieve time
 * @param firstWindow true- first window(5s), false- second window=(6s)
 * @return delay time in seconds
 */
int LoraPacketProcessor::setJoinAcceptDelay(
    timeval &retval,
    SemtechUDPPacket &value,
    const timeval &time,
    bool firstWindow
)
{
    int delaySecs = firstWindow ? 5 : 6;
    std::vector<rfmMetaData>::iterator it(value.metadata.begin());
    uint32_t frequency;
    if (this->deviceChannelPlan) {
        const RegionalParameterChannelPlan *plan = this->deviceChannelPlan->get();
        if (plan) {
            if (firstWindow) {
                delaySecs = plan->joinAcceptDelay1();
                if (it != value.metadata.end()) {
                    frequency = it->freq;
                }
            } else {
                delaySecs = plan->joinAcceptDelay2();
                frequency = plan->bandDefaults.RX2Frequency;
            }
        }
    }
    retval.tv_sec = time.tv_sec;
    retval.tv_usec = time.tv_usec;

    incTimeval(retval, delaySecs, 0);
    if (it != value.metadata.end()) {
        // increment tmst
        it->tmst += delaySecs * 1000000;
        // just in case
        it->t += delaySecs;
        // set frequency
        it->freq = frequency;
    }
    return delaySecs;
}

