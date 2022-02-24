#ifndef DEVICE_HISTORY_ITEM_SERVICE_JSON_H_
#define DEVICE_HISTORY_ITEM_SERVICE_JSON_H_ 1

#include <vector>
#include <mutex>
#include "device-history-service-abstract.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class JsonFileDeviceHistoryService: public DeviceHistoryService {
	private:
		virtual int load();
		virtual int save();
	protected:
		std::mutex mutexMap;
		std::map<DEVADDRINT, DEVICE_HISTORY_ITEM, DEVADDRINTCompare> storage;
		std::string path;
		void clear();		
	public:
		int errcode;
		std::string errmessage;

		JsonFileDeviceHistoryService();
		~JsonFileDeviceHistoryService();
		int get(DEVADDR &devaddr, DeviceHistoryItem &retval);
		void put(DEVADDR &devaddr, DEVICE_HISTORY_ITEM &value);
		void putUp(DEVADDR &devaddr, const time_t &time, uint32_t fcntup);
		void putDown(DEVADDR &devaddr, const  time_t &time, uint32_t fcntdown);
        // increment downstream from network server to the device
        uint32_t incrementDown(const DEVADDR &devaddr, const time_t &time);
        void rm(DEVADDR &addr);
		// List entries
		void list(std::vector<DeviceHistoryItem> &retval, size_t offset, size_t size);
		void flush();
		int init(const std::string &option, void *data);
		void done();
		// debug only
		std::string toJsonString();
};

#endif
