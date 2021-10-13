#ifndef LORA_PACKET_PROCESSOR_H
#define LORA_PACKET_PROCESSOR_H 1

#include <map>
#include <functional>

#include "lora-packet-handler-abstract.h"
#include "identity-service-abstract.h"
#include "device-stat-service-abstract.h"
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
		uint8_t reservedFPort;
		IdentityService *identityService;
		DeviceStatService *deviceStatService;
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
		void setDeviceStatService(DeviceStatService *deviceStatService);

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
			const struct timeval &time,
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
			const struct timeval &time,
			semtechUDPPacket &value
		);
		int enqueueMAC(
			const struct timeval &time,
			semtechUDPPacket &value
		);
		/**
		 * Enqueue control network service message
		 * @param time receive time
		 * @param value Semtech packet
		 * @return 0- success
		 */ 
		int enqueueControl(
			const struct timeval &time,
			semtechUDPPacket &value
		);

	void setRecieverQueueProcessor(RecieverQueueProcessor *value);

	// Reserve FPort number for network service purposes
	void reserveFPort(
		uint8_t value
	);
};

#endif
