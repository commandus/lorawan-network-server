#ifndef RECEIVER_QUEUE_SERVICE_H_
#define RECEIVER_QUEUE_SERVICE_H_ 1

#include <vector>
#include <map>
#include <string>
#include <sys/time.h>

#include "utillora.h"

/**
 * 
 * ReceiverQueueService manage queue of ReceiverQueueEntry.
 * ReceiverQueueEntry is key-value pair ReceiverQueueKey and ReceiverQueueValue.
 * ReceiverQueueKey has a time and serial number (id) in case two or more data at the same time
 * ReceiverQueueValue has a payload, device identifier and target databases
 * 
 * LoraPacketProcessor calls ReceiverQueueService->push() method to insert received from end-device payload
 * 
 * ReceiverQueueService is abstract base class for some impementations:
 * 
 *                       / --> DirTxtReceiverQueueServiceOptions (file collection)
 * ReceiverQueueService -  --> JsonFileReceiverQueueService (in-memory collection)
 *                       \ --> LmdbReceiverQueueService (not implemented yet)
 * 
 */

class ReceiverQueueKey {
	public:
		timeval time;	// key 1
		int id;			// key 2 just in case of of two simultaneous events
		ReceiverQueueKey();
		ReceiverQueueKey(const ReceiverQueueKey &value);
		void clear();
		void setProperties(std::map<std::string, std::string> &retval);
};

class ReceiverQueueValue {
	public:
		DeviceId deviceId;	// device identifier
		std::string payload;
		std::string jsonPayload() const;
		std::vector<int> dbids;	/// Database identifiers

		ReceiverQueueValue();
		ReceiverQueueValue(const ReceiverQueueValue &value);

		bool setJsonPayload(const std::string &jsonValue);
		int popDbId(int dbid);	/// return remaining database count
		bool hasDbId(int dbid);	/// @return true if database exists
 		void clear();
		void setProperties(std::map<std::string, std::string> &retval);
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
		void setProperties(
			std::map<std::string, std::string> *values,
			const std::map<std::string, std::string> *aliases
		);
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
		void push(const DeviceId &deviceId, const std::string &payload, const timeval &time);
		
		// Return 0 if success
		virtual int pop(const int &dbid, ReceiverQueueEntry &retval) = 0;
		// Return 0 if success
		virtual int peek(const int &dbid, ReceiverQueueEntry &retval) = 0;
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
