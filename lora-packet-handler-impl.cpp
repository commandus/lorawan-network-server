#include "lora-packet-handler-impl.h"
#include "udp-socket.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"

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
                   //		<< " " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.deviceEUI)
                   << " " << value.devId.toJsonString() << ": " << hexString(p);
                onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
            }
        } else {
            if (onLog) {
                std::stringstream ss;
                ss << ERR_DUPLICATED_PACKET
                   << " " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
                   //		<< " " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.deviceEUI)
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
		<< ", " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.deviceEUI) << ", " 
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
int LoraPacketProcessor::enqueueMAC(
        const struct timeval &time,
        SemtechUDPPacket &value
)
{
	std::stringstream ss;
	std::string p = value.getMACs();
	ss << MSG_MAC_COMMAND_RECEIVED 
		<< UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
		<< ", " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.deviceEUI) << ", " 
		// << " " << value.devId.toJsonString() << ", " 
		<< "payload: " << hexString(p);
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

LoraPacketProcessor::LoraPacketProcessor()
	: reservedFPort(0), identityService(NULL), gatewayList(NULL), onLog(NULL), receiverQueueService(NULL),
	recieverQueueProcessor(NULL)
{
	packetQueue.start(*this);
}

LoraPacketProcessor::~LoraPacketProcessor()
{
	packetQueue.stop();
	if (recieverQueueProcessor)
		recieverQueueProcessor->stop();
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
	int r = identityService->get(addr, packet.devId);
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
			if (deviceStatService)				// collect statistics if statistics collector is running
				deviceStatService->putUp(addr, time.tv_sec, packet.header.header.fcnt);
			if (packet.hasApplicationPayload()) // store payload to the database(s) if exists
				enqueuePayload(time, packet);
			if (packet.hasMACPayload())			// provide MAC reply to the end-device if MAC command present in the packet
				enqueueMAC(time, packet);
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
	if (recieverQueueProcessor) {
		if (receiverQueueService)
			recieverQueueProcessor->start(receiverQueueService);
		else
			recieverQueueProcessor->stop();
	}
}

void LoraPacketProcessor::setDeviceStatService(
	DeviceStatService *value
)
{
	deviceStatService = value;
	packetQueue.setDeviceStatService(value);
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

void LoraPacketProcessor::setRecieverQueueProcessor
(
	RecieverQueueProcessor *value
)
{
	if (recieverQueueProcessor)
		recieverQueueProcessor->stop();
	recieverQueueProcessor = value;
	if (recieverQueueProcessor) {
		if (receiverQueueService)
			recieverQueueProcessor->start(receiverQueueService);
		else
			recieverQueueProcessor->stop();
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
