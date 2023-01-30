#ifndef IDENTITY_SERVICE_LMDB_H_
#define IDENTITY_SERVICE_LMDB_H_ 1

#include "identity-service.h"
#include "db-identity.h"

class LmdbIdentityService: public IdentityService {
	private:
		dbenv *env;
		std::string filename;
		void clear();
		int open();
		int close();
		int nextBruteForce(NetworkIdentity &retval);
	public:
		LmdbIdentityService();
		~LmdbIdentityService();
		/**
		* get device identifier(w/o address) by network address. Return 0 if success, retval = EUI and keys
		* @param retval device identifier
		* @param devaddr network address
		* @return LORA_OK- success
		*/
		int get(DeviceId &retval, DEVADDR &devaddr);
		/**
		* get network identity(with address) by network address. Return 0 if success, retval = EUI and keys
		* @param retval network identity(with address)
		* @param eui device EUI
		* @return LORA_OK- success
		*/
		int getNetworkIdentity(NetworkIdentity &retval, const DEVEUI &eui);
		/**
		 * List entries
		 * @param retval return values
		 * @param offset 0..
		 * @param size 0- all
		 */
		void list(std::vector<NetworkIdentity> &retval, size_t offset, size_t size);
		// Entries count
		size_t size();
		// Add or replace Address = EUI and keys pair
		void put(DEVADDR &devaddr, DEVICEID &id);
		// Remove entry
		void rm(DEVADDR &addr);
	    // reload
		int init(const std::string &option, void *data);
	    // force save
		void flush();
		// close resources
		void done();
		// parseRX list of identifiers, wildcards or regexes and copy found EUI into retval
		int parseIdentifiers(
			std::vector<TDEVEUI> &retval,
			const std::vector<std::string> &list,
			bool useRegex
		);
		// parseRX list of names, wildcards or regexes and copy found EUI into retval
		int parseNames(
			std::vector<TDEVEUI> &retval,
			const std::vector<std::string> &list,
			bool useRegex
		);
		bool canControlService(
			const DEVADDR &addr
		);

		int joinAccept(
				JOIN_ACCEPT_FRAME_HEADER &retval,
				NetworkIdentity &networkIdentity
		);

		NetId* getNetworkId();
		void setNetworkId(const NetId &value);

		/**
		 * Return next network address if available
		 * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
		 */
		int next(NetworkIdentity &retval);

};

#endif
