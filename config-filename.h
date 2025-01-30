#ifndef CONFIG_FILENAME_H
#define CONFIG_FILENAME_H	1

#include <string>

std::string getDefaultConfigFileName(
    const char *programPath,
    const std::string &filename
);

#endif
