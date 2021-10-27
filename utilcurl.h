#ifndef UTILCURL_H_
#define UTILCURL_H_ 1

#include <string>

/**
 * @brief POST data string 
 */
int postString(
    std::string &retval,
    const std::string &url,
    const std::string &authKey,
    const std::string &data
);

#endif
