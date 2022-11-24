#ifndef UTIL_THREAD_H_
#define UTIL_THREAD_H_ 1

#include <thread>

void setThreadName(
    std::thread* thread,
    const char* threadName
);

#endif
