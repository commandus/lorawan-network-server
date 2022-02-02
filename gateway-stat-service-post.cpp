#include <sstream>
#include "utilcurl.h"
#include "gateway-stat-service-post.h"

/**
 * Device statistics service append statistics to the file
 * specified in the option parameter of init() method
 */
void GatewayStatServicePost::save()
{
    if (list.empty())
        return;
    std::vector<GatewayStat> copyList;
    listMutex.lock();
    copyList = list;
    list.clear();
    listMutex.unlock();

    std::stringstream ss;
    bool needComma = false;
    ss << "[";
    for (std::vector<GatewayStat>::iterator it (copyList.begin()); it != copyList.end(); it++) {
        if (needComma)
            ss << ", ";
        else
            needComma = true;
        ss << it->toJsonString() << std::endl;
    }
    ss << "]";
    std::string ret;
    int r = postString(ret, storageName, "", ss.str());
}
