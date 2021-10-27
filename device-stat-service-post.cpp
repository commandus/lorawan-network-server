#include <sstream>
#include "utilcurl.h"
#include "device-stat-service-post.h"


/**
 * Device statistics service append statistics to the file
 * specified in the option parameter of init() method
 */
void DeviceStatServicePost::save()
{
    if (list.empty())
        return;
    std::vector<SemtechUDPPacket> copyList;
    listMutex.lock();
    copyList = list;
    list.clear();
    listMutex.unlock();

    std::stringstream ss;
    bool needComma = false;
    ss << "[";
    for (std::vector<SemtechUDPPacket>::iterator it (copyList.begin()); it != copyList.end(); it++) {
        if (needComma)
            ss << ", ";
        else
            needComma = true;
        ss << it->toJsonString() << std::endl;
    }
    ss << "]";
    std::string ret;
    int r = postString(ret, storageName, "", ss.str());
    // std::cerr << "==POST data " << ss.str() << " to " << storageName << " return " << r << " result: " << ret << std::endl;
}
