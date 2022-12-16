#include "libloragw-helper.h"

#include "errlist.h"

// libloragw.a: subst-call-c.c calls to LibLoragwHelper

LibLoragwHelper *globalLibLoragwHelper = nullptr;

LibLoragwHelper::LibLoragwHelper()
    : onOpenClose(nullptr), onLog(nullptr)
{

}

LibLoragwHelper::LibLoragwHelper(
    const LibLoragwHelper&value
)
    : onOpenClose(value.onOpenClose), onLog(value.onLog)
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
    if (onOpenClose)
        return onOpenClose->open(fileName, mode);
    return -1;    
}

int LibLoragwHelper::close(
    int fd
)
{
    if (onOpenClose)
        return onOpenClose->close(fd);
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
        onLog->logMessage(this, LOG_INFO, LOG_EMBEDDED_GATEWAY, 0, logBuffer.str());
    logBuffer.str("");
    logBuffer.clear();
}

void LibLoragwHelper::bind()
{
    globalLibLoragwHelper = this;
}

void LibLoragwHelper::unbind()
{
    globalLibLoragwHelper = nullptr;
}
