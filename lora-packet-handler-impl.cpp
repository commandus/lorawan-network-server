#include "lora-packet-handler-impl.h"
#include "udp-socket.h"
#include "utilstring.h"
#include "errlist.h"

int LoraPacketProcessor::onPacket(semtechUDPPacket &value)
{
	return 0;
}

LoraPacketProcessor::LoraPacketProcessor()
	: onLog(NULL), identityService(NULL)
{
	packetQueue.start(*this);
}

LoraPacketProcessor::~LoraPacketProcessor()
{
	packetQueue.stop();
}

int LoraPacketProcessor::put
(
	semtechUDPPacket &packet
)
{
	int r;
	if (identityService) {
		DeviceId id;
		r = identityService->get(packet.getHeader()->header.devaddr, id);
		if (r) {
			// report error
			std::stringstream ss;
			ss << ERR_MESSAGE << r << ": " 
				<< strerror_client(r) << " " << UDPSocket::addrString((const struct sockaddr *) &packet.clientAddress);
			onLog(LOG_ERR, LOG_IDENTITY_SVC, r, ss.str());
			return r;
		}
	} else {
		if (onLog) {
			std::stringstream ss;
			ss << " " << UDPSocket::addrString((const struct sockaddr *) &packet.clientAddress);
			onLog(LOG_INFO, LOG_PACKET_HANDLER, 0, ss.str());
		}
	}
}

void LoraPacketProcessor::setIdentityService
(
	IdentityService* value
)
{
	identityService = value;
}

void LoraPacketProcessor::setLogger(
	std::function<void(
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value) {
	onLog = value;
}
