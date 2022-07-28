#ifndef RECEIVER_QUEUE_SERVICE_DIR_TXT_H_
#define RECEIVER_QUEUE_SERVICE_DIR_TXT_H_	1

#include <vector>
#include <mutex>

#include "receiver-queue-service.h"

typedef enum {
	DIRTXT_FORMAT_BIN = 0,
	DIRTXT_FORMAT_HEX = 1,
	DIRTXT_FORMAT_BASE64 = 2,
} DIRTXT_FORMAT;

class DirTxtReceiverQueueServiceOptions {
	public:
		DIRTXT_FORMAT format;	// 0- bin(default), 1- hex, 2- base64
		DirTxtReceiverQueueServiceOptions();
};

class DirTxtReceiverQueueService: public ReceiverQueueService {
	private:
		DIRTXT_FORMAT writeFormat;
		static int loadFile(
			std::string &payload,
			time_t &retTime, 
			DIRTXT_FORMAT format,
			const std::string &path
		);
		// Write payload to the file
		int storeFile(
			const std::string &payload,
			DIRTXT_FORMAT format
		);
	protected:
		std::mutex mutexMap;
		std::string path;
	public:
		DirTxtReceiverQueueService();
		int count();
		// Return 0 if success
		int get(int onum, ReceiverQueueEntry &retval);
		// Force remove entry
		void remove(int onum);
		// Remove all entries older than parameter, if 0- remove all
		void clear(time_t olderthan);
		// Remove all entries
		void clear();
		// Remove all entries with databases
		void clearDbs(const std::vector <int> &dbs);
		// Add entry
		void pushEntry(ReceiverQueueEntry &value);
		// Return 0 if success
		int pop(const int &dbid, ReceiverQueueEntry &retval);
		// Return 0 if success
		int peek(const int &dbid, ReceiverQueueEntry &retval);
		// List entries
		void list(std::vector<ReceiverQueueEntry> &retval, size_t offset, size_t size);
		// force save
		void flush();
		// reload
		int init(const std::string &option, void *data);
		// close resources
		void done();

		std::string toJsonString();
};

#endif
