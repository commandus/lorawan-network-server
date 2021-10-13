#ifndef IDENTITY_SERVICE_LMDB_H_
#define IDENTITY_SERVICE_LMDB_H_ 1

#include "identity-service-abstract.h"
#include "db-identity.h"

class LmdbIdentityService: public IdentityService {
	private:
		dbenv *env;
		std::string filename;
		void clear();
		int open();
		int close();
	public:
		LmdbIdentityService();
		~LmdbIdentityService();
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
		bool canControlService(
			const DEVADDR &addr
		);
};

#endif
