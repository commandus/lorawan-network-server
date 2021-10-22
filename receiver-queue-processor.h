#ifndef QUEUE_PROCESSOR_H
#define QUEUE_PROCESSOR_H 1

#include <map>
#include <functional>
#include <mutex>
#include <thread>

#include "packet-queue.h"
#include "db-any.h"
#include "receiver-queue-service.h"

// default delay in ms
#define DB_MIN_DELAY_MS 200

// queue default timeout
#define DB_DEF_TIMEOUT_MS 500

/**
 * ReceiverQueueService enqueue aplication data payload,
 * RecieverQueueProcessor get payload from the queue, parse and put parsed data 
 * to the database(s) async
 */ 
class RecieverQueueProcessor {
	private:
		bool isStarted;
		bool isDone;
		std::mutex mutexq;
		std::thread *threadDb;
		
		void *pkt2env;
		DatabaseByConfig *databaseByConfig;
		ReceiverQueueService *receiverQueueService;

		std::function<void(
			void *env,
			int level,
			int modulecode,
			int errorcode,
			const std::string &message
		)> onLog;
		void runner();
		void processQueue();
	public:
		RecieverQueueProcessor();
		~RecieverQueueProcessor();
		int onPacket(
                struct timeval &time,
                DeviceId id,
                SemtechUDPPacket &value
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
