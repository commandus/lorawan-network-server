#include <string>
#include <iostream>
#include <bitset>

#include "errlist.h"
#include "dev-addr.h"


static void printBits(const DevAddr &value)
{
    DEVADDR *r = value.getPtr();
    std::cout 
        << std::bitset<8>(r[3]).to_string() << " "
        << std::bitset<8>(r[2]).to_string() << " "
        << std::bitset<8>(r[1]).to_string() << " "
        << std::bitset<8>(r[0]).to_string() << std::endl;
}

static void printDetails(const DevAddr &value)
{
    DEVADDR *r = value.getPtr();
    std::cout << std::hex
        << "Type: " << value.getType() << " "
        << "NwkId: " << value.getNwkId() << " "
        << "NetAddr: " << value.getNwkAddr() << std::endl;
}

int main(int argc, char **argv)
{
    DevAddr a;
    a.set(0, 1, 2);
    std::cout << a.toString() << std::endl;
    printBits(a);
    printDetails(a);    
}
