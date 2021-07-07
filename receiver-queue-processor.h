#ifndef QUEUE_PROCESSOR_H
#define QUEUE_PROCESSOR_H 1

#include <map>
#include <functional>
#include "packet-queue.h"
#include "db-any.h"
#include "receiver-queue-service.h"

/**
 * Handle uplink messages
 */ 
class RecieverQueueProcessor {
	private:
		void *pkt2env;
		DatabaseByConfig *databaseByConfig;

		std::function<void(
			void *env,
			int level,
			int modulecode,
			int errorcode,
			const std::string &message
		)> onLog;
	public:
		RecieverQueueProcessor();
		~RecieverQueueProcessor();
		int onPacket(
			struct timeval &time,
			DeviceId id,
			semtechUDPPacket &value
		);
		void setLogger(
			std::function<void(
				void *env,
				int level,
				int modulecode,
				int errorcode,
				const std::string &message
		)> value);
		void setPkt2Env(void *pkt2env);
		void setDatabaseByConfig(DatabaseByConfig *value);

		void start(ReceiverQueueService *queueService);
		void stop();
};

#endif
