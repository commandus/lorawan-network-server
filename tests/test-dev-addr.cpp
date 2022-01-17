#include <string>
#include <iostream>
#include <bitset>

#include "errlist.h"
#include "dev-addr.h"
#include <sys/time.h>
#include <identity-service-file-json.h>
#include <iomanip>

static void printBits(const DevAddr &value)
{
    std::cout 
        << std::bitset<8>{value.devaddr[3]}.to_string() << " "
        << std::bitset<8>{value.devaddr[2]}.to_string() << " "
        << std::bitset<8>{value.devaddr[1]}.to_string() << " "
        << std::bitset<8>{value.devaddr[0]}.to_string();
}

static void printDetails(const DevAddr &value)
{
    std::cout << std::hex
              << "Type: " << (int) value.getNetIdType() << " "
              << "NwkId: " << value.getNwkId() << " "
              << "NetAddr: " << value.getNwkAddr();
}

static void testSet(
    uint8_t netTypeId, 
    uint32_t nwkId,
    uint32_t nwkAddr
)
{
    DevAddr a;
    int r = a.set(netTypeId, nwkId, nwkAddr);
    if (r) {
        std::cerr << "Set error " << r << std::endl;
        return;
    }
    std::cout << a.toString() << std::endl;
    printBits(a);
    std::cout << std::endl;
    std::cout << std::hex
        << "Type: " << (int) netTypeId << " "
        << "NwkId: " << nwkId << " "
        << "NetAddr: " << nwkAddr << std::endl;
    printDetails(a);
    std::cout << std::endl;
}

static void testNetIdSet(
    uint8_t netTypeId, 
    uint32_t netId,
    uint32_t nwkAddr
)
{
    NetId netid;
    int r = netid.set(netTypeId, netId);
    if (r) {
        std::cerr << "Set error " << r << std::endl;
        return;
    }
    std::cout 
        << std::bitset<8>{netid.netid[2]}.to_string() << " "
        << std::bitset<8>{netid.netid[1]}.to_string() << " "
        << std::bitset<8>{netid.netid[0]}.to_string() << std::endl;

    std::cout << std::hex
        << "NetId type: " << (int) netid.getType() << " "
        << "NetId: " << netid.getNetId() << " "
        << "NwkId: " << netid.getNwkId() << std::endl;
    
    DevAddr a;
    r = a.set(netid, nwkAddr);
    if (r) {
        std::cerr << "Set error " << r << std::endl;
        return;
    }
    std::cout << a.toString() << std::endl;
    printBits(a);
    std::cout << std::endl;
    std::cout << std::hex
        << "Type: " << (int) netTypeId << " "
        << "NetId: " << netId << " "
        << "NetAddr: " << nwkAddr << std::endl;
    printDetails(a);
    std::cout << std::endl;
}

static void testIncrement(
    uint8_t netTypeId,
    uint32_t netId,
    uint32_t nwkAddr
)
{
    nwkAddr -= 10;
    DevAddr a(netTypeId, netId, nwkAddr);

    while (true) {
        std::cout << a.toString() << "    ";
        printBits(a);
        std::cout << "    ";
        printDetails(a);
        std::cout << std::endl;

        if (a.increment())
            break;
    }
}

/**
 * @param result
 * @param x
 * @param y
 * @return
 * @see https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html
 */
static int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait. tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}

static void testIncrementSpeed2(
        uint8_t netTypeId,
        uint32_t netId
)
{
    JsonFileIdentityService identityService;
    // set network identifier
    NetId ni(netTypeId, netId);
    identityService.setNetworkId(ni);

    DevAddr a(netTypeId, netId, 0);
    struct timeval t0;
    gettimeofday(&t0, NULL);
    NetworkIdentity netwIdent;
    while (true) {
        if (a.increment())
            break;
        // a.get(netwIdent.devaddr);
        if (identityService.next(netwIdent) == 0) {
            // ok
            /*
            std::cout << a.toString() << "    ";
            printBits(a);
            std::cout << "    ";
            printDetails(a);
            std::cout << std::endl;
             */
        }
    }
    struct timeval t1;
    gettimeofday(&t1, NULL);

    struct timeval df;
    timeval_subtract(&df, &t1, &t0);

    std::cout
        << "Elapsed time: " << std::dec << df.tv_sec << "." << df.tv_usec % 1000000
        <<  ", size: 0x" << std::hex << a.size() << std::endl;
}

static void fillUp
(
    IdentityService &service,
    DevAddr &addr,
    int start,
    int finish
)
{
    struct timeval t0;
    gettimeofday(&t0, NULL);

    DEVICEID id;
    for (int i = start; i <= finish; i++) {
        addr.setAddr(i);
        service.put(addr.devaddr, id);
    }

    struct timeval t1;
    gettimeofday(&t1, NULL);

    struct timeval df;
    timeval_subtract(&df, &t1, &t0);

    std::cout
            << "Fill up elapsed time: " << std::dec << df.tv_sec << "."
            << std::setfill('0') << std::setw(6) << df.tv_usec % 1000000
            <<  ", size: 0x" << std::hex << (finish - start) << std::endl;
}

static void testIncrementSpeed(
        uint8_t netTypeId,
        uint32_t netId
)
{
    JsonFileIdentityService identityService;
    // set network identifier
    NetId ni(netTypeId, netId);
    identityService.setNetworkId(ni);
    DevAddr a(netTypeId, netId, 0);
    fillUp(identityService, a, 0, 0xffffff);

    struct timeval t0;
    gettimeofday(&t0, NULL);
    NetworkIdentity netwIdent;
    if (identityService.next(netwIdent) == 0) {
        // ok
        /*
        std::cout << a.toString() << "    ";
        printBits(a);
        std::cout << "    ";
        printDetails(a);
        std::cout << std::endl;
         */
    }
    struct timeval t1;
    gettimeofday(&t1, NULL);

    struct timeval df;
    timeval_subtract(&df, &t1, &t0);

    std::cout
            << "Test increment elapsed time: " << std::dec << df.tv_sec << "."
            << std::setfill('0') << std::setw(6) << df.tv_usec % 1000000
            <<  ", size: 0x" << std::hex << a.size() << std::endl;
}

int main(int argc, char **argv)
{
    testSet(0, 0x3f, 0x1ffffff);
    /*
    testSet(1, 0x3f, 0xffffff);
    testSet(2, 0x1ff, 0xfffff);
    testSet(3, 0x3ff, 0x3ffff);
    testSet(4, 0x7ff, 0xffff);
    testSet(5, 0x1fff, 0x1fff);
    testSet(6, 0x7fff, 0x3ff);
    testSet(7, 0x1ffff, 0x7f);
    */
    /*
    testNetIdSet(0, 0x3f, 0x1ffffff);
    testNetIdSet(1, 0x3f, 0xffffff);
    testNetIdSet(2, 0x1ff, 0xfffff);
    testNetIdSet(3, 0x1fffff, 0x3ffff);
    testNetIdSet(4, 0x1fffff, 0xffff);
    testNetIdSet(5, 0x1fffff, 0x1fff);
    testNetIdSet(6, 0x1fffff, 0x3ff);
    testNetIdSet(7, 0x1fffff, 0x7f);
    */

    // testIncrement(0, 0x3f, 0x1ffffff);

    testIncrementSpeed(0, 0x3f);
    // testIncrementSpeed(7, 0x1fffff);
}
