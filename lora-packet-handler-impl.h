#ifndef LORA_PACKET_PROCESSOR_H
#define LORA_PACKET_PROCESSOR_H 1

#include <map>
#include <sstream>
#include <functional>

#include "lora-packet-handler-abstract.h"
#include "identity-service.h"
#include "device-history-service-abstract.h"
#include "gateway-list.h"
#include "packet-queue.h"
#include "packet-handler-abstract.h"
#include "receiver-queue-service.h"
#include "receiver-queue-processor.h"
#include "db-any.h"
#include "device-channel-plan.h"
#include "regional-parameter-channel-plan.h"

/**
 * Handle uplink messages
 */ 
class LoraPacketProcessor: public LoraPacketHandler, PacketHandler {
	private:
		uint8_t reservedFPort;
		IdentityService *identityService;
		DeviceHistoryService *deviceHistoryService;
		GatewayList *gatewayList;
        const DeviceChannelPlan *deviceChannelPlan;

		// ReceiverQueueService enqueueTxPacket data payload packets received from gateways (with deduplication)
		ReceiverQueueService *receiverQueueService;
		// ReceiverQueueProcessor get payload from the queue, parseRX and put parsed data
		ReceiverQueueProcessor *receiverQueueProcessor;
		PacketQueue packetQueue;

		LogIntf *onLog;
	public:
		static void addTimeWindow1(struct timeval &value);
		static void addTimeWindow2(struct timeval &value);
		LoraPacketProcessor();
		~LoraPacketProcessor();
		void setIdentityService(IdentityService* identityService);
		void setDeviceHistoryService(DeviceHistoryService *aDeviceHistoryService);

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
                SemtechUDPPacket &packet
		);
		void setLogger(LogIntf *value);
		int enqueuePayload(
                const struct timeval &time,
                SemtechUDPPacket &value
		);
		int enqueueMAC(
                const struct timeval &time,
                SemtechUDPPacket &value
		);
        int enqueueJoinResponse(
                const struct timeval &time,
                const DEVADDR &addr,
                SemtechUDPPacket &value
        );
        int putMACRequests(
                const struct timeval &time,
                SemtechUDPPacket &value
        );
		/**
		 * Enqueue control network service message
		 * @param time receive time
		 * @param value Semtech packet
		 * @return 0- success
		 */ 
		int enqueueControl(
                const struct timeval &time,
                SemtechUDPPacket &value
		);

	    void setReceiverQueueProcessor(ReceiverQueueProcessor *value);

	    // Reserve FPort number for network service purposes
	    void reserveFPort(
		    uint8_t value
	    );

        void setDeviceChannelPlan(const DeviceChannelPlan *value);

        int join(
            const struct timeval &time,
            int socket,
            const sockaddr_in *socketAddress,
            SemtechUDPPacket &packet
        );

    int setJoinAcceptDelay(
        timeval &retval,
        SemtechUDPPacket &value,
        const timeval &time,
        bool firstWindow
    );
};

#endif
