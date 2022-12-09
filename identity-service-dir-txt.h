#ifndef IDENTITY_DIR_TXT_H_
#define IDENTITY_DIR_TXT_H_ 1

/**
 * DirTxtIdentityService class load device's passport text files.
 * Each passport text file contain lines, som of them are device 
 * idientifiers and keys
 */

#include <vector>
#ifdef ENABLE_WATCHER
#include "filewatch.hpp"
#endif

#include "identity-service-file-json.h"

class DirTxtIdentityService;

typedef void (OnIdentitiesUpdate)
(
	DirTxtIdentityService *service,
#ifdef ENABLE_WATCHER
    const filewatch::Event &event
#endif
    const std::string &path
);

class DirTxtIdentityService: public JsonFileIdentityService {
	private:
#ifdef ENABLE_WATCHER
    filewatch::FileWatch<std::string> *fileWatcher;
#endif
		OnIdentitiesUpdate *onIdentitiesUpdate;
		int loadTxtFile(const std::string &path);
		int load() override;
		int save() override;
	public:
		int startListen(OnIdentitiesUpdate *callback);
		int stopListen();
        // Entries count
        size_t size() override;
};

#endif
