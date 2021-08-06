#include "lora-packet-handler-impl.h"
#include "udp-socket.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"

#include <iostream>

int LoraPacketProcessor::enqueuePayload(
	struct timeval &time,
	DeviceId id,
	semtechUDPPacket &value
)
{
	std::stringstream ss;
	std::string p = value.getPayload();
	ss << timeval2string(time) << MSG_DEVICE_EUI << DEVEUI2string(id.deviceEUI) << ", " << UDPSocket::addrString((const struct sockaddr *) &value.clientAddress)
		<< " " << value.devId.toJsonString() << ": " << hexString(p);
	onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

	// ReceiverQueueService deduplicate repeated packets received from gateways and then store data to the database asynchronously
	if (receiverQueueService)
		receiverQueueService->push(value, time);
	return 0;
}

int LoraPacketProcessor::enqueueMAC(
	struct timeval &time,
	DeviceId id,
	semtechUDPPacket &value
)
{
	std::stringstream ss;
	std::string p = value.getPayload();
	ss << timeval2string(time) << MSG_DEVICE_EUI << DEVEUI2string(id.deviceEUI) << ", " << UDPSocket::addrString((const struct sockaddr *) &value.clientAddress)
		<< " " << value.devId.toJsonString() << ": MAC command(s)" << hexString(p);
	onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());


	// MAC commands
	onLog(this, LOG_ERR, LOG_PACKET_HANDLER, 0, "MAC command does not processed yet");

	return 0;
}


LoraPacketProcessor::LoraPacketProcessor()
	: onLog(NULL), identityService(NULL), receiverQueueService(NULL),
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

int LoraPacketProcessor::put
(
	struct timeval &time,
	semtechUDPPacket &packet
)
{
	int r;
	if (identityService) {
		DeviceId id;
		DEVADDR *addr = &packet.getHeader()->header.devaddr;
		r = identityService->get(*addr, id);
		if (r == 0) {
			if (packet.hasApplicationPayload()) 
				enqueuePayload(time, id, packet);
			if (packet.hasMACPayload())
				enqueueMAC(time, id, packet);
		} else {
			// device id NOT identified
			if (onLog) {
				// report error
				std::stringstream ss;
				ss << ERR_DEVICE_ADDRESS_NOTFOUND << r << ": " 
					<< strerror_client(r) << " " 
					<< ", " << MSG_DEVICE_EUI << DEVADDR2string(packet.getHeader()->header.devaddr)
					<< ", " << UDPSocket::addrString((const struct sockaddr *) &packet.clientAddress);
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
