#ifndef LORA_PACKET_PROCESSOR_H
#define LORA_PACKET_PROCESSOR_H 1

#include <map>
#include <functional>
#include "lora-packet-handler-abstract.h"
#include "identity-service-abstract.h"
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
		ReceiverQueueService *receiverQueueService;
		PacketQueue packetQueue;
		RecieverQueueProcessor *recieverQueueProcessor;

		std::function<void(
			void *env,
			int level,
			int modulecode,
			int errorcode,
			const std::string &message
		)> onLog;
	public:
		LoraPacketProcessor();
		~LoraPacketProcessor();
		void setIdentityService(IdentityService* identityService);
		void setReceiverQueueService(ReceiverQueueService* value);

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
		)> value);
		int onPacket(
			struct timeval &time,
			DeviceId id,
			semtechUDPPacket &value
		);

	void setRecieverQueueProcessor(RecieverQueueProcessor *value);
};

#endif
