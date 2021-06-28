#ifndef RECEIVER_QUEUE_SERVICE_H_
#define RECEIVER_QUEUE_SERVICE_H_ 1

#include <vector>
#include <string>
#include <sys/time.h>

#include "utillora.h"

class ReceiverQueueKey {
	public:
		timeval time;	// key 1
		int id;			// key 2
		ReceiverQueueKey();
		ReceiverQueueKey(const ReceiverQueueKey &value);
		void clear();
};

class ReceiverQueueValue {
	public:
		DeviceId deviceId;
		std::string payload;
		std::string jsonPayload() const;
		std::vector<int> dbids;	/// Database identifiers

		ReceiverQueueValue();
		ReceiverQueueValue(const ReceiverQueueValue &value);

		bool setJsonPayload(const std::string &jsonValue);
		int popDbId(int dbid);	/// return remaining database count
		void clear();
};

struct ReceiverQueueKeyCompare
{
	bool operator() (const ReceiverQueueKey& l, const ReceiverQueueKey& r) const;
};

class ReceiverQueueEntry {
	public:
		ReceiverQueueKey key;
		ReceiverQueueValue value;
		ReceiverQueueEntry();
		ReceiverQueueEntry(
			const ReceiverQueueKey &key,
			const ReceiverQueueValue &value
		);
		void set(
			const ReceiverQueueKey &key,
			const ReceiverQueueValue &value
		);
		std::string toJsonString() const;
		void clear();
};

/**
 * Recieved message queue service interface
 */ 

class ReceiverQueueService {
	protected:
		int cnt;
		std::vector<int> dbs;
		int next();
	public:
		ReceiverQueueService();
		void setDbs(const std::vector<int> &values);
		// return messages in queue
		virtual int count() = 0;
		// Return 0 if success
		virtual int get(int onum, ReceiverQueueEntry &retval) = 0;
		// Force remove entry
		virtual void remove(int onum) = 0;
		// Remove all entries older than parameter, if 0- remove all
		virtual void clear(time_t olderthan) = 0;
		// Remove all entries
		virtual void clear() = 0;
		// Remove all entries with databases
		virtual void clearDbs(const std::vector <int> &dbs) = 0;
		
		// Add entry
		virtual void pushEntry(ReceiverQueueEntry &value) = 0;
		void push(const DeviceId deviceId, const std::string &payload, const timeval &time);
		
		// Return 0 if success
		virtual int pop(const int &dbid, ReceiverQueueEntry &retval) = 0;
		// List entries
		virtual void list(std::vector<ReceiverQueueEntry> &retval, size_t offset, size_t size) = 0;
		// force save
		virtual void flush() = 0;
		// reload
		virtual int init(const std::string &option, void *data) = 0;
		// close resources
		virtual void done() = 0;
};

#endif
