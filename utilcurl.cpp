#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif

#include "utilcurl.h"
#include "errlist.h"

#ifdef ENABLE_CURL
#include <curl/curl.h>
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
    std::string &retVal,
    const std::string &url,
    const std::string &data,
    const std::string &authorizationHeader,
    const std::string &contentType
) {
    CURL *curl = curl_easy_init();
    if (!curl)
        return CURLE_FAILED_INIT;
    struct curl_slist *chunk = nullptr;
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &retVal);
    CURLcode res = curl_easy_perform(curl);
    long http_code;
    if (res != CURLE_OK) {
        retVal = curl_easy_strerror(res);
        http_code = - res;
    } else {
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    }
    curl_easy_cleanup(curl);
    return (int) http_code;
}
#else

#ifdef _MSC_VER
#include <WinSock2.h>
typedef unsigned long in_addr_t;
#include <io.h>
#else
#define SOCKET int
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

#endif

#include <sstream>
#include "utilfile.h"

/**
 * @brief POST data string
 */
int postString(
    std::string &retVal,
    const std::string &url,
    const std::string &data,
    const std::string &authorizationHeader,
    const std::string &contentType
) {
    URL uri(url);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return ERR_CODE_SOCKET_CREATE;
    }
    struct hostent *he = gethostbyname(uri.host.c_str());
    if (!he) {
        return -2;
    }
    in_addr_t in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(he->h_addr_list)));
    if (in_addr == INADDR_NONE) {
        return ERR_CODE_SOCKET_ADDRESS;
    }
    struct sockaddr_in sa;
    sa.sin_addr.s_addr = in_addr;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(80);    // only http no https
    if (connect(sock, (struct sockaddr*)&sa, sizeof(sockaddr_in)) == -1) {
        return ERR_CODE_SOCKET_CONNECT;
    }

    std::stringstream ss;
    ss << "POST " << uri.host <<  " HTTP/1.0\r\n"
        "Host: " << uri.host << "\r\n"
        "Content-type: " << contentType << "\r\n"
        "Content-length: " << data.size() << "\r\n";
    if (!authorizationHeader.empty())
        ss << "Authorization: " << authorizationHeader << "\r\n";
    ss << "\r\n" << data << "\r\n";

    /// Write the request
    std::string s = ss.str();
    if (write((int) sock, s.c_str(), s.size()) >= 0) {
        /// Read the response
        std::stringstream rss;
        char ch;
        while (read((int) sock, &ch, sizeof(ch)) > 0) {
            rss << ch;
        }
        retVal = ss.str();
        size_t sz = retVal.size();
        if (sz > 4) {
            if (retVal[sz - 4] == '\r') {
                retVal.erase(sz - 4);
            }
        }
    }
    close((int) sock);
    return 0;
}

#endif
