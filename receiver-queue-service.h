#ifndef RECEIVER_QUEUE_SERVICE_H_
#define RECEIVER_QUEUE_SERVICE_H_ 1

#include <map>
#include <vector>
#include <string>
#include <sys/time.h>

class ReceiverQueueKey {
	public:
		timeval time;	// key 1
		int id;			// key 2
		ReceiverQueueKey();
		ReceiverQueueKey(const ReceiverQueueKey &value);
};

class ReceiverQueueValue {
	public:
		std::string payload;
		std::string jsonPayload() const;
		void setJsonPayload(const std::string &jsonValue);
		std::vector<int> dbids;	/// Database identifiers
		ReceiverQueueValue();
		ReceiverQueueValue(const ReceiverQueueValue &value);
		int popDbId(int dbid);	/// return remaining database count
};

struct ReceiverQueueKeyCompare
{
	bool operator() (const ReceiverQueueKey& l, const ReceiverQueueKey& r) const;
};

class ReceiverQueueEntry {
	public:
		ReceiverQueueKey key;
		ReceiverQueueValue value;\
		ReceiverQueueEntry();
		ReceiverQueueEntry(
			const ReceiverQueueKey &key,
			const ReceiverQueueValue &value
		);
		void set(
			const ReceiverQueueKey &key,
			const ReceiverQueueValue &value
		);
};

/**
 * Recieved message queue service interface
 */ 

class ReceiverQueueService {
	protected:
		int cnt;
		int next();
	public:
		ReceiverQueueService();
		virtual void setDbs(const std::vector<int> &values) = 0;
		// return messages in queue
		virtual int count() = 0;
		// Return 0 if success
		virtual int get(int onum, ReceiverQueueEntry &retval) = 0;
		// Force remove entry
		virtual void remove(int onum) = 0;
		// Remove all entries older than parameter, if 0- remove all
		void clear(time_t olderthan);
		// Remove all entries
		void clear();

		// Add entry
		virtual void push(ReceiverQueueEntry &value) = 0;
		// Return 0 if success
		virtual int pop(const int &dbid, ReceiverQueueEntry &retval) = 0;
		// List entries
		void list(std::vector<ReceiverQueueEntry> &retval, size_t offset, size_t size);
		// force save
		virtual void flush() = 0;
		// reload
		virtual int init(const std::string &option, void *data) = 0;
		// close resources
		virtual void done() = 0;
};

#endif
