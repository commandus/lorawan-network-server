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
#define GW_SIZE_PER_STEP 128
/**
 * Gateway statistics service append statistics to the file
 * specified in the option parameter of init() method
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
        bool get(GatewayStat &retval, size_t id) override;
        size_t size() override;
        void put(GatewayStat *stat) override;
		// force save
		void flush() override;
		// reload
		int init(const std::string &filename, void *data) override;
		// close resources
		void done() override;
        void runner();
        virtual void save();
        void tuneDelay();
};

#endif
