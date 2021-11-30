#include <sstream>
#include "utilcurl.h"
#include "device-stat-service-post.h"
#include "errlist.h"

/**
 * Device statistics service append statistics to the file
 * specified in the option parameter of init() method
 */
int DeviceStatServicePost::save()
{
    if (list.empty())
        return ERR_CODE_PARAM_INVALID;
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
    return postString(ret, storageName, "", ss.str());
}
