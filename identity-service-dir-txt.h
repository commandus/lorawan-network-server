#ifndef IDENTITY_DIR_TXT_H_
#define IDENTITY_DIR_TXT_H_ 1

#include <vector>
#include "identity-service-file-json.h"

class DirTxtIdentityService: public JsonFileIdentityService {
	private:
		int load();
		int save();
};

#endif
