#include <string>
#include <iostream>
#include <bitset>

#include "errlist.h"
#include "dev-addr.h"


static void printBits(const DevAddr &value)
{
    std::cout 
        << std::bitset<8>{value.devaddr[3]}.to_string() << " "
        << std::bitset<8>{value.devaddr[2]}.to_string() << " "
        << std::bitset<8>{value.devaddr[1]}.to_string() << " "
        << std::bitset<8>{value.devaddr[0]}.to_string() << std::endl;
}

static void printDetails(const DevAddr &value)
{
    std::cout << std::hex
        << "Type: " << value.getNetIdType() << " "
        << "NwkId: " << value.getNwkId() << " "
        << "NetAddr: " << value.getNwkAddr() << std::endl;
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
    netid.set(netTypeId, netId);
    DevAddr a;
    int r = a.set(netid, nwkAddr);
    if (r) {
        std::cerr << "Set error " << r << std::endl;
        return;
    }
    std::cout << a.toString() << std::endl;
    printBits(a);
    std::cout << std::hex
        << "Type: " << (int) netTypeId << " "
        << "NetId: " << netId << " "
        << "NetAddr: " << nwkAddr << std::endl;
    printDetails(a);
    std::cout << std::endl;
}

int main(int argc, char **argv)
{
    /*
    testSet(0, 0x3f, 0x1ffffff);
    testSet(1, 0x3f, 0xffffff);
    testSet(2, 0x1ff, 0xfffff);
    testSet(3, 0x3ff, 0x3ffff);
    testSet(4, 0x7ff, 0xffff);
    testSet(5, 0x1fff, 0x1fff);
    testSet(6, 0x7fff, 0x3ff);
    testSet(7, 0x1ffff, 0x7f);
    */

    testNetIdSet(0, 0x3f, 0x1ffffff);
    testNetIdSet(1, 0x3f, 0xffffff);
    testNetIdSet(2, 0x1ff, 0xfffff);
    testNetIdSet(3, 0x3ff, 0x3ffff);
    testNetIdSet(4, 0x7ff, 0xffff);
    testNetIdSet(5, 0x1fff, 0x1fff);
    testNetIdSet(6, 0x7fff, 0x3ff);
    testNetIdSet(7, 0x1ffff, 0x7f);

    exit(0);

 
    testSet(0, 1, 0x1ffffff);
    testSet(1, 2, 0xffffff);
    testSet(2, 3, 0xfffff);
    testSet(3, 4, 0x3ffff);
    testSet(4, 5, 0xffff);
    testSet(5, 6, 0x1fff);
    testSet(6, 7, 0x3ff);
    testSet(7, 8, 0x7f);
   
    testSet(0, 0x3f, 0);
    testSet(1, 0x3f, 0);
    testSet(2, 0x1ff, 0);
    testSet(3, 0x3ff, 0);
    testSet(4, 0x7ff, 0);
    testSet(5, 0x1fff, 0);
    testSet(6, 0x7fff, 0);
    testSet(7, 0x1ffff, 0);
    
    exit(0);

    testSet(0, 1, 0x1234567);
    testSet(1, 1, 0x123456);
    testSet(2, 1, 0x12345);
    testSet(3, 1, 0x12345);
    testSet(4, 1, 0x1234);
    testSet(5, 1, 0x1234);
    testSet(6, 1, 0x123);
    testSet(7, 1, 0x12);

    testSet(0, 1, 0x1);
    testSet(1, 1, 0x1);
    testSet(2, 1, 0x1);
    testSet(3, 1, 0x1);
    testSet(4, 1, 0x1);
    testSet(5, 1, 0x1);
    testSet(6, 1, 0x1);
    testSet(7, 1, 0x1);

    testSet(0, 1, 0);
    testSet(1, 1, 0);
    testSet(2, 1, 0);
    testSet(3, 1, 0);
    testSet(4, 1, 0);
    testSet(5, 1, 0);
    testSet(6, 1, 0);
    testSet(7, 1, 0);

}
