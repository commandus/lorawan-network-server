#ifndef LORA_PACKET_PROCESSOR_H
#define LORA_PACKET_PROCESSOR_H 1

#include <map>
#include <functional>

#include "lora-packet-handler-abstract.h"
#include "identity-service-abstract.h"
#include "gateway-list.h"
#include "packet-queue.h"
#include "packet-handler-abstract.h"
#include "receiver-queue-service.h"
#include "receiver-queue-processor.h"
#include "db-any.h"

/**
 * Handle uplink messages
 */ 
class LoraPacketProcessor: public LoraPacketHandler, PacketHandler {
	private:
		IdentityService *identityService;
		GatewayList *gatewayList;

		// ReceiverQueueService enque data payload packets received from gateways (with deduplication)
		ReceiverQueueService *receiverQueueService;
		// RecieverQueueProcessor get payload from the queue, parse and put parsed data 
		RecieverQueueProcessor *recieverQueueProcessor;
		PacketQueue packetQueue;

		std::function<void(
			void *env,
			int level,
			int modulecode,
			int errorcode,
			const std::string &message
		)> onLog;
	public:
		static void addTimeWindow1(struct timeval &value);
		static void addTimeWindow2(struct timeval &value);
		LoraPacketProcessor();
		~LoraPacketProcessor();
		void setIdentityService(IdentityService* identityService);
		void setGatewayList(GatewayList *value);
		void setReceiverQueueService(ReceiverQueueService* value);

		int ack(
			int socket,
			const sockaddr_in* gwAddress,
			const SEMTECH_PREFIX_GW &dataprefix
		);

		/**
		 * Add packet to be processed
		 */
		int put(
			struct timeval &time,
			semtechUDPPacket &packet
		);
		void setLogger(
			std::function<void(
				void *env,
				int level,
				int modulecode,
				int errorcode,
				const std::string &message
		) > value);
		int enqueuePayload(
			struct timeval &time,
			semtechUDPPacket &value
		);
		int enqueueMAC(
			struct timeval &time,
			semtechUDPPacket &value
		);
	void setRecieverQueueProcessor(RecieverQueueProcessor *value);
};

#endif
