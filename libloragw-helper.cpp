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
        return onOpenClose->openDevice(fileName, mode);
    return -1;
}

int LibLoragwHelper::close(
    int fd
)
{
    if (onOpenClose)
        return onOpenClose->closeDevice(fd);
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
    if (!onLog)
        return;
    std::string msg = logBuffer.str();
    int emergency = LOG_INFO;
    if (msg.find("ERROR:") == 0) {
        if (msg.find("failed to write", 7) == 7) {
            emergency = LOG_ALERT;
        } else {
            emergency = LOG_ERR;
        }
    }
    onLog->onInfo(this, emergency, LOG_EMBEDDED_GATEWAY, 0, msg);
    logBuffer.str("");
    logBuffer.clear();
}

void LibLoragwHelper::bind(
    LogIntf *aOnLog,
    LibLoragwOpenClose *aOnOpenClose
)
{
    onLog = aOnLog;
    onOpenClose = aOnOpenClose;
    globalLibLoragwHelper = this;
}

void LibLoragwHelper::unbind()
{
    globalLibLoragwHelper = nullptr;
}
