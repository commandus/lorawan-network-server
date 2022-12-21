/**
 * Add next lines to the ~/git/rak_common_for_gateway/lora/rak2287/sx1302_hal/libloragw/inc/config.h:
 * #include "subst-call-c.h"
 * #define open open_c
 * #define close close_c
 * #define printf printf_c
 * #define fprintf(fd, args...) printf_c(args)
 * 
 * Usage:
 *  #include "libloragw-helper.h"
 *
*/

#ifndef LIBLORAGW_HELPER_H_
#define LIBLORAGW_HELPER_H_	1

#include <string>
#include <sstream>

#include "log-intf.h"

class LibLoragwOpenClose {
    public:
        virtual int openDevice(const char *fileName, int mode) = 0;
        virtual int closeDevice(int fd) = 0;
};

class LibLoragwHelper {
    private:
        std::stringstream logBuffer;
    public:
        // int fd; ///< USB gateway file descriptor
        // std::string fileName; ///< USB gateway file name called from open() - not used
        LibLoragwOpenClose *onOpenClose;
        LogIntf *onLog;

        LibLoragwHelper();
        LibLoragwHelper(const LibLoragwHelper&value);
        virtual ~LibLoragwHelper();

        int open(const char *fileName, int mode);
        int close(int fd);
        int log(char ch);
        void flush();
        void bind(LogIntf *aOnLog, LibLoragwOpenClose *aOnOpenClose);    ///< bind to the library using global
        void unbind();    ///< bind to the library using global
};

#endif
