#ifndef IDENTITY_FILE_JSON_LMDB_H_
#define IDENTITY_FILE_JSON_LMDB_H_ 1

#include <vector>
#include "identity-service-abstract.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class JsonFileIdentityService: public IdentityService {
	private:
		std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare> storage;
		std::string filename;
		void clear();
		int load();
		int save();
	public:
		JsonFileIdentityService();
		~JsonFileIdentityService();
		int get(DEVADDR &devaddr, DeviceId &retval);
		int getNetworkIdentity(NetworkIdentity &retval, const DEVEUI &eui);
		// List entries
		void list(std::vector<NetworkIdentity> &retval, size_t offset, size_t size);
		void put(DEVADDR &devaddr, DEVICEID &id);
		void rm(DEVADDR &addr);
		
		int init(const std::string &option, void *data);
		void flush();
		void done();
		int parseIdentifiers(
			std::vector<TDEVEUI> &retval,
			const std::vector<std::string> &list,
			bool useRegex
		);
		int parseNames(
			std::vector<TDEVEUI> &retval,
			const std::vector<std::string> &list,
			bool useRegex
		);
		// debug only
		std::string toJsonString();
		int errcode;
		std::string errmessage;
};

#endif
