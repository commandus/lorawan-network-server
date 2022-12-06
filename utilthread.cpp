/**
 * Set thread name
 * @see https://stackoverflow.com/questions/10121560/stdthread-naming-your-thread
 * @see https://learn.microsoft.com/ru-ru/previous-versions/visualstudio/visual-studio-2015/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN
 */
#include "utilthread.h"

#ifdef _WIN32
#include <windows.h>
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO {
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void setThreadNameById(
    uint32_t dwThreadID,
    const char* threadName
)
{
  // DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;
   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}

void setThreadName(
    std::thread *thread,
    const char* threadName
)
{
    if (!thread)
        return;
    DWORD threadId = ::GetThreadId( static_cast<HANDLE>(thread->native_handle() ) );
    setThreadNameById(threadId, threadName);
}

#else

#include <pthread.h>
void setThreadName(
    std::thread* thread,
    const char* threadName
)
{
    auto handle = thread->native_handle();
    pthread_setname_np(handle,threadName);
}

#endif
