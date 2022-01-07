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

int main(int argc, char **argv)
{
    testSet(0, 1, 0x1ffffff);
    testSet(1, 1, 0xffffff);
    testSet(2, 1, 0xfffff);
    testSet(3, 1, 0x3ffff);
    testSet(4, 1, 0xffff);
    testSet(5, 1, 0x1fff);
    testSet(6, 1, 0x3ff);
    testSet(7, 1, 0x7f);

    exit(0);

    testSet(0, 1, 12);
    testSet(1, 1, 12);
    testSet(2, 1, 12);
    testSet(3, 1, 12);
    testSet(4, 1, 12);
    testSet(5, 1, 12);
    testSet(6, 1, 12);
    testSet(7, 1, 12);

    testSet(0, 1, 13);
    testSet(1, 1, 14);
    testSet(2, 1, 15);
    testSet(3, 1, 16);
    testSet(4, 1, 17);
    testSet(5, 1, 18);
    testSet(6, 1, 19);
    testSet(7, 1, 20);

}
