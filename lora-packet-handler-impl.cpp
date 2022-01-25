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
// default queue timeout in microseconds, less than 1s, e.g. 1/4s
#define DEF_TIMEOUT_US	250 * 1000

int LoraPacketProcessor::enqueuePayload(
        const struct timeval &time,
        SemtechUDPPacket &value
)
{
	std::stringstream ss;
	std::string p = value.payload;
	// ReceiverQueueService deduplicate repeated packets received from gateways and then store data to the database asynchronously
	if (receiverQueueService)
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
                std::stringstream ss;
                ss << ERR_DUPLICATED_PACKET
                   << " " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
                   //		<< " " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.devEUI)
                   << " " << value.devId.toJsonString() << ": " << hexString(p);
                onLog(this, LOG_INFO, LOG_PACKET_HANDLER, ERR_CODE_DUPLICATED_PACKET, ss.str());
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
       << "payload: " << hexString(value.payload);
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
        << hexString(value.getMACs())  << " to be send to  "
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
        << ", payload: " << hexString(value.payload)
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

int LoraPacketProcessor::enqueueJoinResponse(
    const struct timeval &time,
    SemtechUDPPacket &value
)
{
    std::stringstream ss;
    std::string macs = value.getMACs();
    const JOIN_REQUEST_FRAME *joinRequestFrame = value.getJoinRequestFrame();

    // log Join event
    ss << MSG_ENQUEUE_JOIN_REQUEST << MSG_TO_REQUEST
        << JOIN_REQUEST_FRAME2string(*joinRequestFrame)
        << ", gateway address: " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
        << ", " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.devEUI);
    onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

    // delay time
    struct timeval t;
    t.tv_sec = time.tv_sec;
    t.tv_usec = time.tv_usec;

    incTimeval(t, 0, DEF_TIMEOUT_US);

    packetQueue.push(0, MODE_JOIN_REQUEST, t, value);
    packetQueue.wakeUp();
    return LORA_OK;
}

LoraPacketProcessor::LoraPacketProcessor()
	: reservedFPort(0), identityService(NULL), gatewayList(NULL), onLog(NULL), receiverQueueService(NULL),
      receiverQueueProcessor(NULL)
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
 * Identify device, if device identified successfully, enqueue packet or MAC
 * @param time received time
 * @param packet Semtech gateway packet
 */ 
int LoraPacketProcessor::put
(
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
                    // enqueue packet route to the target device or network server itself
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

int LoraPacketProcessor::join(
        const struct timeval &time,
        int socket,
        const sockaddr_in *socketAddress,
        SemtechUDPPacket &packet
)
{
    DEVADDR addr;
    memmove(&addr, &packet.getHeader()->header.devaddr, sizeof(DEVADDR));

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
            ss << ERR_MESSAGE << ERR_CODE_BAD_JOIN_REQUEST << ": " << ERR_BAD_JOIN_REQUEST;
            onLog(this, LOG_CRIT, LOG_IDENTITY_SVC, ERR_CODE_INIT_IDENTITY, ss.str());
        }
        return ERR_CODE_BAD_JOIN_REQUEST;
    }

    // try to identify end-device
    NetworkIdentity networkIdentity;
    int r = identityService->getNetworkIdentity(networkIdentity, joinRequestFrame->devEUI);
    if (r == 0) {
        // device has been identified
        if (deviceHistoryService)				// collect statistics if statistics collector is running
            deviceHistoryService->putUp(addr, time.tv_sec, packet.header.header.fcnt);
        // set device identifier
        packet.devId = networkIdentity;
        // enqueue response
        enqueueJoinResponse(time, packet);
    } else {
        // device EUI is NOT identified
        if (onLog) {
            // report error
            std::stringstream ss;
            ss << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r)
               << ", " << MSG_DEVICE_EUI << DEVEUI2string(joinRequestFrame->joinEUI)
               << ", " << UDPSocket::addrString((const struct sockaddr *) &packet.gatewayAddress);
            onLog(this, LOG_ERR, LOG_IDENTITY_SVC, r, ss.str());
        }
    }
    return r;
}
