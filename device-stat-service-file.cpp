#include <sstream>
#include "device-stat-service-file.h"
#include "utilstring.h"
#include "utilthread.h"
#include "errlist.h"

/**
 * Device statistics service append statistics to the file
 * specified in the option parameter of init() method
 */
DeviceStatServiceFile::DeviceStatServiceFile()
    : state(0), timeoutSeconds(DEF_DEVICE_STAT_TIMEOUT_SECONDS), threadRun(nullptr)
{

}

bool DeviceStatServiceFile::get(
    SemtechUDPPacket &retval,
    size_t position
)
{
    if (position < list.size()) {
        retval = list.at(position);
        return true;
    }
    return false;
}

size_t DeviceStatServiceFile::size()
{
    return list.size();
}

void DeviceStatServiceFile::put(
    const SemtechUDPPacket *packet
)
{
    if (list.size() > MAX_DEVICE_STAT_BUFFER_SIZE) {
        // TODO
        flush();
    }

    listMutex.lock();
    list.push_back(*packet);
    listMutex.unlock();
}

// force save
void DeviceStatServiceFile::flush()
{
    save();
}

// reload
int DeviceStatServiceFile::init(
	const std::string &aFileName,
	void *data
)
{
    state = 1;  // 0- stopped, 1- run, 2- stop request
    storageName = aFileName;
    threadRun = new std::thread(&DeviceStatServiceFile::runner, this);
    setThreadName(threadRun, MODULE_NAME_DEVICE_STAT_SVC_FILE);
    threadRun->detach();
    return 0;
}

// close resources
 void DeviceStatServiceFile::done()
 {
    if (threadRun && (state == 1)) {
        state = 2;  // 0- stopped, 1- run, 2- stop request
    }
 }

// close resources
void DeviceStatServiceFile::runner()
{
    // MAX_DEVICE_STAT_TIMEOUT_SECONDS
    while (state == 1) { // 0- stopped, 1- run, 2- stop request
        if (timeoutSeconds <= 0)
            timeoutSeconds = MIN_DEVICE_STAT_TIMEOUT_SECONDS;
        struct timeval timeout {timeoutSeconds, 0 };
        int r = select(0, nullptr, nullptr, nullptr, &timeout);
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
    threadRun = nullptr;
}

void DeviceStatServiceFile::tuneDelay()
{
    size_t count = list.size();
    if (count < DEVICE_SIZE_PER_STEP) {
        // increase
        timeoutSeconds *= 2;
    } else {
        // decrease
        timeoutSeconds /= 2;
    }
    if (timeoutSeconds == 0)
        timeoutSeconds = MIN_DEVICE_STAT_TIMEOUT_SECONDS;
    else {
        if (timeoutSeconds > MAX_DEVICE_STAT_TIMEOUT_SECONDS)
            timeoutSeconds = MAX_DEVICE_STAT_TIMEOUT_SECONDS;
    }
}

int DeviceStatServiceFileJson::save()
{
    if (list.empty())
        return ERR_CODE_PARAM_INVALID;
    std::vector<SemtechUDPPacket> copyList;
    listMutex.lock();
    copyList = list;
    list.clear();
    listMutex.unlock();
    std::stringstream ss;
    for (std::vector<SemtechUDPPacket>::iterator it (copyList.begin()); it != copyList.end(); it++) {
        ss << it->toJsonString() << std::endl;
    }
    if (append2file(storageName, ss.str()))
        return LORA_OK;
    return ERR_CODE_SOCKET_WRITE;
}

int DeviceStatServiceFileCsv::save()
{
    if (list.empty())
        return ERR_CODE_PARAM_INVALID;
    std::vector<SemtechUDPPacket> copyList;
    listMutex.lock();
    copyList = list;
    list.clear();
    listMutex.unlock();
    std::stringstream ss;
    for (std::vector<SemtechUDPPacket>::iterator it (copyList.begin()); it != copyList.end(); it++) {
        ss << it->toCsvString() << std::endl;
    }
    if (append2file(storageName, ss.str()))
        return LORA_OK;
    return ERR_CODE_SOCKET_WRITE;
}
