#ifndef IDENTITY_SERVICE_FILE_JSON_H_
#define IDENTITY_SERVICE_FILE_JSON_H_ 1

#include <vector>
#include <mutex>
#include "identity-service.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class JsonFileIdentityService: public IdentityService {
	private:
		virtual int load();
		virtual int save();
	protected:
		std::mutex mutexMap;
		std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare> storage;
		std::map<DEVADDRINT, uint32_t, DEVADDRINTCompare> rightsMask;
		std::string path;
		void clear();		

	public:
		JsonFileIdentityService();
		~JsonFileIdentityService();
		int get(DeviceId &retval, DEVADDR &devaddr) override;
		int getNetworkIdentity(NetworkIdentity &retval, const DEVEUI &eui) override;
		// List entries
		void list(std::vector<NetworkIdentity> &retval, size_t offset, size_t size) override;
		void put(DEVADDR &devaddr, DEVICEID &id) override;
		void rm(DEVADDR &addr) override;
		
		int init(const std::string &option, void *data) override;
		void flush() override;
		void done() override;
		int parseIdentifiers(
			std::vector<TDEVEUI> &retval,
			const std::vector<std::string> &list,
			bool useRegex
		) override;
		int parseNames(
			std::vector<TDEVEUI> &retval,
			const std::vector<std::string> &list,
			bool useRegex
		) override;
		bool canControlService(
			const DEVADDR &addr
		) override;

		// debug only
		std::string toJsonString();
		int errcode;
		std::string errmessage;

		uint32_t getRightsMask(
			const DEVADDR &addr
		);
		void setRightsMask(
			const DEVADDR &addr,
			uint32_t value
		);
        /**
          * Return next network address if available
          * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
          */
        int next(NetworkIdentity &retval);
};

#endif
