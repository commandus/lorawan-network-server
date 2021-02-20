#ifndef LORA_PACKET_PROCESSOR_H
#define LORA_PACKET_PROCESSOR_H 1

#include <map>
#include <functional>
#include "lora-packet-handler-abstract.h"
#include "identity-service-abstract.h"
#include "packet-queue.h"
#include "packet-handler-abstract.h"

/**
 * Handle uplink messages
 */ 
class LoraPacketProcessor: public LoraPacketHandler, PacketHandler {
	private:
		IdentityService* identityService;
		PacketQueue packetQueue;
		std::function<void(
			int level,
			int modulecode,
			int errorcode,
			const std::string &message
		)> onLog;
	public:
		LoraPacketProcessor();
		~LoraPacketProcessor();
		void setIdentityService(IdentityService* identityService);
		/**
		 * Add packet to be processed
		 */
		int put(semtechUDPPacket &packet);
		void setLogger(
			std::function<void(
				int level,
				int modulecode,
				int errorcode,
				const std::string &message
		)> value);
		int onPacket(semtechUDPPacket &value);
};

#endif
