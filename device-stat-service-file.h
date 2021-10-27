#ifndef DEVICE_STAT_SERVICE_FILE_H_
#define DEVICE_STAT_SERVICE_FILE_H_ 1

#include <thread>
#include <vector>
#include <mutex>
#include "device-stat-service-abstract.h"

#define MAX_DEVICE_STAT_BUFFER_SIZE 1024
#define MIN_DEVICE_STAT_TIMEOUT_SECONDS 1
#define MAX_DEVICE_STAT_TIMEOUT_SECONDS 5 * 60
#define DEF_DEVICE_STAT_TIMEOUT_SECONDS 60
#define DEVICE_SIZE_PER_STEP 256
/**
 * Gateway statistics service append statistics to the file
 * specified in the option paramater of init() method
 */
class DeviceStatServiceFile : public DeviceStatService {
private:
        int state;  // 0- stopped, 1- run, 2- stop request
        int timeoutSeconds;
        std::thread *threadRun;
    protected:
        std::string storageName;
        std::vector<SemtechUDPPacket> list;
        std::mutex listMutex;
	public:
        DeviceStatServiceFile();
		virtual void put(const SemtechUDPPacket *packet);
		// force save
		virtual void flush();
		// reload
		virtual int init(const std::string &filename, void *data);
		// close resources
		virtual void done();

    void runner();

    virtual void save();

    void tuneDelay();
};

#endif
