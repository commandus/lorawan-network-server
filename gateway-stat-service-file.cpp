#include <sstream>
#include "gateway-stat-service-file.h"
#include "utilstring.h"

/**
 * Gateway statistics service append statistics to the file
 * specified in the option parameter of init() method
 */
GatewayStatServiceFile::GatewayStatServiceFile()
    : state(0), timeoutSeconds(DEF_GW_STAT_TIMEOUT_SECONDS), threadRun(NULL)
{

}

void GatewayStatServiceFile::put(GatewayStat *stat)
{
    if (list.size() > MAX_GW_STAT_BUFFER_SIZE) {
        // TODO
        flush();
    }

    listMutex.lock();
    list.push_back(*stat);
    listMutex.unlock();
}

// force save
void GatewayStatServiceFile::flush()
{
    save();
}

// reload
int GatewayStatServiceFile::init(
	const std::string &aFileName,
	void *data
)
{
    state = 1;  // 0- stopped, 1- run, 2- stop request
    storageName = aFileName;
    threadRun = new std::thread(&GatewayStatServiceFile::runner, this);
    threadRun->detach();
    return 0;
}

// close resources
 void GatewayStatServiceFile::done()
 {
    if (threadRun && (state == 1)) {
        state = 2;  // 0- stopped, 1- run, 2- stop request
    }
 }

// close resources
void GatewayStatServiceFile::runner()
{
    // MAX_GW_STAT_TIMEOUT_SECONDS
    while (state == 1) { // 0- stopped, 1- run, 2- stop request
        struct timeval timeout;
        if (timeoutSeconds <= 0)
            timeoutSeconds = MIN_GW_STAT_TIMEOUT_SECONDS;
        timeout.tv_sec = timeoutSeconds;
        timeout.tv_usec = 0;
        int r = select(0, NULL, NULL, NULL, &timeout);
        switch (r) {
            case -1:
                break;
            case 0:
                // timeout
                tuneDelay();
                save();
                continue;
            default:
                break;
        }
    }
    state = 0;
    threadRun = NULL;
}

void GatewayStatServiceFile::tuneDelay()
{
    size_t count = list.size();
    if (count < GW_SIZE_PER_STEP) {
        // increase
        timeoutSeconds *= 2;
    } else {
        // decrease
        timeoutSeconds /= 2;
    }
    if (timeoutSeconds == 0)
        timeoutSeconds = MIN_GW_STAT_TIMEOUT_SECONDS;
    else {
        if (timeoutSeconds > MAX_GW_STAT_TIMEOUT_SECONDS)
            timeoutSeconds = MAX_GW_STAT_TIMEOUT_SECONDS;
    }
}

void GatewayStatServiceFile::save()
{
    if (list.empty())
        return;
    std::vector<GatewayStat> copyList;
    listMutex.lock();
    copyList = list;
    list.clear();
    listMutex.unlock();
    std::stringstream ss;
    for (std::vector<GatewayStat>::iterator it (copyList.begin()); it != copyList.end(); it++) {
        ss << it->toJsonString() << std::endl;
    }
    append2file(storageName, ss.str());
}
