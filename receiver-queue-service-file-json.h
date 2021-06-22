#ifndef RECEIVER_QUEUE_SERVICE_FILE_JSON_H_
#define RECEIVER_QUEUE_SERVICE_FILE_JSON_H_ 1

#include <vector>
#include <mutex>
#include "receiver-queue-service.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class JsonFileReceiverQueueService: public ReceiverQueueService {
	private:
		int load();
		int save();
	protected:
		std::mutex mutexMap;
		std::map<ReceiverQueueKey, ReceiverQueueValue, ReceiverQueueKeyCompare> storage;
		std::vector<int> dbs;
		std::string path;
	public:
		int errcode;
		std::string errmessage;

		JsonFileReceiverQueueService();
		virtual ~JsonFileReceiverQueueService();

		void setDbs(const std::vector<int> &values);
		// return messages in queue
		int count();
		// Return 0 if success
		int get(int onum, ReceiverQueueEntry &retval);
		// Force remove entry
		void remove(int onum);
		// Remove all entries older than parameter, if 0- remove all
		void clear(time_t olderthan);
		// Remove all entries
		void clear();

		// Add entry
		void push(ReceiverQueueEntry &value);
		// Return 0 if success
		int pop(const int &dbid, ReceiverQueueEntry &value);
		// List entries
		void list(std::vector<ReceiverQueueEntry> &retval, size_t offset, size_t size);
		// force save
		void flush();
		// reload
		int init(const std::string &option, void *data);
		// close resources
		void done();

		// debug only
		std::string toJsonString();
};

#endif
