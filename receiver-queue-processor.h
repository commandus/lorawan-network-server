#ifndef RECEIVER_QUEUE_PROCESSOR_H
#define RECEIVER_QUEUE_PROCESSOR_H 1

#include <map>
#include <functional>
#include <mutex>
#include <thread>

#include "packet-queue.h"
#include "db-any.h"
#include "receiver-queue-service.h"

// default delay in ms
#define DB_MIN_DELAY_MS         200
// default maximum insertions into the database per one step
#define MAX_QUEUE_CNT_PER_STEP  1024
// queue default timeout
#define DB_DEF_TIMEOUT_MS 1 * 1000

/**
 * ReceiverQueueService enqueueTxPacket application data payload,
 * ReceiverQueueProcessor get payload from the queue, parseRX and put parsed data
 * to the database(s) async
 */ 
class ReceiverQueueProcessor {
	private:
		bool isStarted;
		bool isDone;
		std::mutex mutexq;
		std::thread *threadDb;
		
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
        void put2databases();
        void processQueue();
	public:
		ReceiverQueueProcessor();
		~ReceiverQueueProcessor();
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
		void setDatabaseByConfig(DatabaseByConfig *value);

		void start(ReceiverQueueService *queueService);
		void stop();
};

#endif
