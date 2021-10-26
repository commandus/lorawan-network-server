#include <sstream>
#include <curl/curl.h>
#include "gateway-stat-service-post.h"

/**
  * @brief CURL write callback
  */
static size_t writeString(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
* @brief Push notification to device
*/
int postString(
    std::string &retval,
    const std::string &url,
    const std::string &authKey,
    const std::string &data
) {
    CURL *curl = curl_easy_init();
    if (!curl)
        return CURLE_FAILED_INIT;
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    if (!authKey.empty())
        chunk = curl_slist_append(chunk, std::string("Authorization: key=" + authKey).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &retval);
    CURLcode res = curl_easy_perform(curl);
    long http_code;
    if (res != CURLE_OK) {
        retval = curl_easy_strerror(res);
        http_code = - res;
    } else {
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    }
    curl_easy_cleanup(curl);
    return http_code;
}

/**
 * Gateway statistics service append statistics to the file
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
    // std::cerr << "==POST data " << ss.str() << " to " << storageName << " return " << r << " result: " << ret << std::endl;
}
