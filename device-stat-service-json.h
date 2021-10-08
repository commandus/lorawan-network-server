#ifndef IDENTITY_STAT_SERVICE_JSON_H_
#define IDENTITY_STAT_SERVICE_JSON_H_ 1

#include <vector>
#include <mutex>
#include "device-stat-service-abstract.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class JsonFileDeviceStatService: public DeviceStatService {
	private:
		virtual int load();
		virtual int save();
	protected:
		std::mutex mutexMap;
		std::map<DEVADDRINT, DEVICESTAT, DEVADDRINTCompare> storage;
		std::string path;
		void clear();		
	public:
		int errcode;
		std::string errmessage;

		JsonFileDeviceStatService();
		~JsonFileDeviceStatService();
		int get(DEVADDR &devaddr, DeviceStat &retval);
		void put(DEVADDR &devaddr, DEVICESTAT &value);
		void putUp(DEVADDR &devaddr, time_t &time, uint32_t fcntup);
		void putDown(DEVADDR &devaddr, time_t &time, uint32_t fcntdown);
		void rm(DEVADDR &addr);
		// List entries
		void list(std::vector<DeviceStat> &retval, size_t offset, size_t size);
		void flush();
		int init(const std::string &option, void *data);
		void done();
		// debug only
		std::string toJsonString();
};

#endif
