#ifndef UTILCURL_H_
#define UTILCURL_H_ 1

#include <string>

/**
 * @brief POST data string 
 */
int postString(
    std::string &retval,
    const std::string &url,
    const std::string &data,
    const std::string &authorizationHeader,  ///< e.g. "key=authToken"
    const std::string &contentType = ""
);

#endif
