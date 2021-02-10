#ifndef LORA_PACKET_PROCESSOR_H
#define LORA_PACKET_PROCESSOR_H 1

#include <map>
#include <functional>
#include "lora-packet-handler-abstract.h"
#include "identity-service-abstract.h"

class LoraPacketProcessor: public LoraPacketHandler {
	private:
		IdentityService* identityService;
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
		int process(semtechUDPPacket &packet);
		void setLogger(
			std::function<void(
				int level,
				int modulecode,
				int errorcode,
				const std::string &message
		)> value);
};

#endif
