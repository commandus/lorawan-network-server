#ifndef RECEIVER_QUEUE_SERVICE_DIR_TXT_H_
#define RECEIVER_QUEUE_SERVICE_DIR_TXT_H_	1

#include <vector>
#include <mutex>
#include "filewatch.hpp"

#include "receiver-queue-service.h"

class DirTxtReceiverQueueService;

typedef void (OnFileUpdate)
(
	DirTxtReceiverQueueService *service,
	const std::string &path,
	const filewatch::Event &event
);

class DirTxtReceiverQueueService: public ReceiverQueueService {
	private:
		filewatch::FileWatch<std::string> *fileWatcher;
		OnFileUpdate *onFileUpdate;

		static int loadFile(
			std::string &payload,
			time_t &retTime, 
			int format,
			const std::string &path
		);
		// Write payload to the file
		int storeFile(
			const std::string &payload,
			int format
		);
	protected:
		std::mutex mutexMap;
		std::string path;
	public:
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
		// List entries
		void list(std::vector<ReceiverQueueEntry> &retval, size_t offset, size_t size);
		// force save
		void flush();
		// reload
		int init(const std::string &option, void *data);
		// close resources
		void done();

		int startListen(OnFileUpdate *callback);
		int stopListen();
};

#endif
