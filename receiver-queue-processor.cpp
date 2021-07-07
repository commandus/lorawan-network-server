#include "receiver-queue-processor.h"
#include "udp-socket.h"
#include "utilstring.h"
#include "utildate.h"
#include "errlist.h"

#include <iostream>

int RecieverQueueProcessor::onPacket(
	struct timeval &time,
	DeviceId id,
	semtechUDPPacket &value
)
{
	std::stringstream ss;
	ss << timeval2string(time) << MSG_DEVICE_EUI << DEVEUI2string(id.deviceEUI) << ", " << UDPSocket::addrString((const struct sockaddr *) &value.clientAddress);
	onLog(this, LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());

	return 0;
}

RecieverQueueProcessor::RecieverQueueProcessor()
	: onLog(NULL),
	pkt2env(NULL), databaseByConfig(NULL)
{
}

RecieverQueueProcessor::~RecieverQueueProcessor()
{
}

void RecieverQueueProcessor::setLogger(
	std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value) {
	onLog = value;
}

void RecieverQueueProcessor::setPkt2Env
(
	void *value
)
{
	pkt2env = value;
}

void RecieverQueueProcessor::setDatabaseByConfig
(
	DatabaseByConfig *value
)
{
	databaseByConfig = value;
}

void RecieverQueueProcessor::start(
	ReceiverQueueService *queueService
)
{
	std::cerr << "RecieverQueueProcessor::start()" << std::endl;
}

void RecieverQueueProcessor::stop()
{
	std::cerr << "RecieverQueueProcessor::stop()" << std::endl;
}
