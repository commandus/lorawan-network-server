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

#include <string>
#include <sstream>

class LibLoragwLog {
    public:
        void msg(const std::string &message);
};

class LibLoragwOpen {
    public:
        int open(const char *fileName, int mode);
};

class LibLoragwClose {
    public:
        int close(int fd);
};

class LibLoragwHelper {
    private:
        std::stringstream logBuffer;
    public:
        // int fd; ///< USB gateway file descriptor
        // std::string fileName; ///< USB gateway file name called from open() - not used
        LibLoragwOpen *onOpen;
        LibLoragwClose *onClose;
        LibLoragwLog *onLog;

        LibLoragwHelper();
        LibLoragwHelper(const LibLoragwHelper&value);
        virtual ~LibLoragwHelper();

        int open(const char *fileName, int mode);
        int close(int fd);
        int log(char ch);
        void flush();
        void bind();    ///< bind to the library using global
        void unbind();    ///< bind to the library using global
};
