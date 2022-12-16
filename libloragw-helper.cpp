#include "libloragw-helper.h"

// libloragw.a: subst-call-c.c
extern "C" {
int open_c(const char *file, int flags, ...);
int close_c (int fd);
int printf_c(const char* format, ... );
}

LibLoragwHelper *globalLibLoragwHelper = nullptr;

LibLoragwHelper::LibLoragwHelper()
    : onOpen(nullptr), onClose(nullptr), onLog(nullptr)
{

}

LibLoragwHelper::LibLoragwHelper(
    const LibLoragwHelper&value
)
    : onOpen(value.onOpen), onClose(value.onClose), onLog(value.onLog)
{

}

LibLoragwHelper::~LibLoragwHelper()
{
    flush();
}

int LibLoragwHelper::open(
    const char *fileName, int mode
)
{
    if (onOpen)
        return onOpen->open(fileName, mode);
    return -1;    
}

int LibLoragwHelper::close(
    int fd
)
{
    if (onClose)
        return onClose->close(fd);
    return -1;    
}

/**
 * accumulate string then send
*/
int LibLoragwHelper::log(
    char ch
)
{
    if (ch == '\r')
        return 1;   // skip
    if (ch == '\n') {
        flush();
        return 1;
    }
    logBuffer << ch;
    return 1;
}

void LibLoragwHelper::flush()
{
    if (onLog)
        onLog->msg(logBuffer.str());
    logBuffer.str("");
    logBuffer.clear();
}

void LibLoragwHelper::bind()
{
    globalLibLoragwHelper = this;
}

void LibLoragwHelper::bind()
{
    globalLibLoragwHelper = nullptr;
}
