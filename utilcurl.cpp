#include <curl/curl.h>
#include "utilcurl.h"

/**
  * @brief CURL write callback
  */
static size_t writeString(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief POST data string 
 */
int postString(
    std::string &retval,
    const std::string &url,
    const std::string &data,
    const std::string &authorizationHeader,
    const std::string &contentType
) {
    CURL *curl = curl_easy_init();
    if (!curl)
        return CURLE_FAILED_INIT;
    struct curl_slist *chunk = NULL;
    if (contentType.empty())
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
    else
        chunk = curl_slist_append(chunk, ("Content-Type: " + contentType).c_str());
    if (!authorizationHeader.empty())
        chunk = curl_slist_append(chunk, std::string("Authorization: " + authorizationHeader).c_str());
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
    return (int) http_code;
}
