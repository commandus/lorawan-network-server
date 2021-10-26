#ifndef GATEWAY_STAT_SERVICE_FILE_H_
#define GATEWAY_STAT_SERVICE_FILE_H_ 1

#include <thread>
#include <vector>
#include <mutex>
#include "gateway-stat-service-abstract.h"

#define MAX_GW_STAT_BUFFER_SIZE 1024
#define MIN_GW_STAT_TIMEOUT_SECONDS 1
#define MAX_GW_STAT_TIMEOUT_SECONDS 5 * 60
#define DEF_GW_STAT_TIMEOUT_SECONDS 60
#define SIZE_PER_STEP 100
/**
 * Gateway statistics service append statistics to the file
 * specified in the option paramater of init() method
 */
class GatewayStatServiceFile : public GatewayStatService {
private:
        int state;  // 0- stopped, 1- run, 2- stop request
        int timeoutSeconds;
        std::thread *threadRun;
    protected:
        std::string storageName;
        std::vector<GatewayStat> list;
        std::mutex listMutex;
	public:
        GatewayStatServiceFile();
		virtual void put(GatewayStat *stat);
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
