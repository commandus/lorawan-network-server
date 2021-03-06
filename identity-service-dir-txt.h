#ifndef IDENTITY_DIR_TXT_H_
#define IDENTITY_DIR_TXT_H_ 1

/**
 * DirTxtIdentityService class load device's passport text files.
 * Each passport text file contain lines, som of them are device 
 * idientifiers and keys
 */

#include <vector>
#include "filewatch.hpp"

#include "identity-service-file-json.h"

class DirTxtIdentityService;

typedef void (OnIdentitiesUpdate)
(
	DirTxtIdentityService *service,
	const std::string &path,
	const filewatch::Event &event
);

class DirTxtIdentityService: public JsonFileIdentityService {
	private:
		filewatch::FileWatch<std::string> *fileWatcher;
		OnIdentitiesUpdate *onIdentitiesUpdate;
		int loadTxtFile(const std::string &path);
		int load();
		int save();
	public:
		int startListen(OnIdentitiesUpdate *callback);
		int stopListen();
};

#endif
