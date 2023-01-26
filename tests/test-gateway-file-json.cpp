#include <string>
#include <iostream>
#include <cassert>

#include "gateway-file-json.h"
#include "utilstring.h"

#ifdef WIN32
#else
#include <execinfo.h>
#include <signal.h>
#endif

void testEmpty()
{
    GatewayConfigFileJson c;
    std::cout << c.toString() << std::endl;
}

void testLoadFile(const std::string &fn)
{
    std::string v = file2string(fn.c_str());
    GatewayConfigFileJson c;
    int r = c.parseString(v);
    if (r) {
        std::cerr << "Error " << r << std::endl;
        std::cerr << "Parse error " << c.errorDescription << " at " << c.errorOffset << std::endl;
        return;
    }

    std::cout << c.toString() << std::endl;
    std::string check = c.toString();
    GatewayConfigFileJson c2;
    c2.parseString(check);
    std::cout << c2.toString() << std::endl;
    assert(c == c2);
}

#define TRACE_BUFFER_SIZE   256

static void printTrace() {
    void *t[TRACE_BUFFER_SIZE];
#ifndef WIN32
    size_t size = backtrace(t, TRACE_BUFFER_SIZE);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
#endif
}

void signalHandler(int signal)
{
    switch (signal)
    {
        case SIGINT:
            std::cerr << "Interrupt" << std::endl;
            exit(signal);
        case SIGSEGV:
            std::cerr << "Segmentation fault" << std::endl;
            printTrace();
            exit(signal);
        default:
            break;
    }
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = &signalHandler;
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGSEGV, &action, nullptr);
}
#endif

int main(int argc, char **argv)
{
    setSignalHandler();
    // testEmpty();
    testLoadFile("/home/andrei/git/rak_common_for_gateway/lora/rak2287/sx1302_hal/packet_forwarder/global_conf.json");
}
