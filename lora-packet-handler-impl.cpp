#include "lora-packet-handler-impl.h"
#include "udp-socket.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"

#include <iostream>

// tme window delay, s
#define DEF_TIME_WINDOW_1	1
#define DEF_TIME_WINDOW_2	2

int LoraPacketProcessor::enqueuePayload(
	struct timeval &time,
	semtechUDPPacket &value
)
{
	std::stringstream ss;
	std::string p = value.getPayload();
	ss << timeval2string(time)
		<< " " << UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
//		<< " " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.deviceEUI) 
		<< " " << value.devId.toJsonString() << ": " << hexString(p);
	onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

	// ReceiverQueueService deduplicate repeated packets received from gateways and then store data to the database asynchronously
	if (receiverQueueService)
		receiverQueueService->push(value, time);
	return 0;
}

int LoraPacketProcessor::enqueueMAC(
	struct timeval &time,
	semtechUDPPacket &value
)
{
	std::stringstream ss;
	std::string p = value.getPayload();
	ss << MSG_MAC_COMMAND_RECEIVED 
		<< UDPSocket::addrString((const struct sockaddr *) &value.gatewayAddress)
		<< ", " << MSG_DEVICE_EUI << DEVEUI2string(value.devId.deviceEUI) << ", " 
		<< " " << value.devId.toJsonString() << "payload: " << hexString(p);
	onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

	addTimeWindow1(time);
	packetQueue.push(0, MODE_REPLY_MAC, time, value);
	// MAC commands
	onLog(this, LOG_ERR, LOG_PACKET_HANDLER, 0, "MAC command does not processed yet");

	return 0;
}


LoraPacketProcessor::LoraPacketProcessor()
	: identityService(NULL), gatewayList(NULL),  onLog(NULL), receiverQueueService(NULL),
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

// immediately send ACK
int LoraPacketProcessor::ackImmediately
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
				<< " " << rr << ": " << strerror_lorawan_ns(rr)
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

// send ACK via queue
int LoraPacketProcessor::ack
(
	int socket,
	const sockaddr_in* gwAddress,
	const SEMTECH_DATA_PREFIX &dataprefix
)
{
	semtechUDPPacket p((const struct sockaddr *) gwAddress, &dataprefix, NULL, "", identityService);
	struct timeval t = { 0, 0 };
	packetQueue.push(socket, MODE_ACK, t, p);
	packetQueue.wakeUp();
	return LORA_OK;
}

int LoraPacketProcessor::put
(
	struct timeval &time,
	semtechUDPPacket &packet
)
{
	int r;
	if (identityService) {
		DEVADDR *addr = &packet.getHeader()->header.devaddr;
		r = identityService->get(*addr, packet.devId);
		if (r == 0) {
			if (packet.hasApplicationPayload()) 
				enqueuePayload(time, packet);
			if (packet.hasMACPayload())
				enqueueMAC(time, packet);
		} else {
			// device id NOT identified
			if (onLog) {
				// report error
				std::stringstream ss;
				ss << ERR_DEVICE_ADDRESS_NOTFOUND << r << ": " 
					<< strerror_lorawan_ns(r) << " " 
					<< ", " << MSG_DEVICE_EUI << DEVADDR2string(packet.getHeader()->header.devaddr)
					<< ", " << UDPSocket::addrString((const struct sockaddr *) &packet.gatewayAddress);
				onLog(this, LOG_ERR, LOG_IDENTITY_SVC, r, ss.str());
			}
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
	ReceiverQueueService* value
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

void LoraPacketProcessor::setLogger(
	std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value) {
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
