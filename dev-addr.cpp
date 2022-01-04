#include <cstring>
#include <sstream>

#include "dev-addr.h"
#include "utillora.h"

uint32_t DEVADDR2int(const DEVADDR &value)
{
    uint32_t retval;
    *((uint32_t*) &retval) = NTOH4(*((uint32_t*) &value));
    return retval;
}

typedef struct {
    uint8_t typePrefixLength;
    uint8_t networkIdBits;
    uint8_t devDddrBits;
} DEVADDR_TYPE_SIZE;

static const DEVADDR_TYPE_SIZE DEVADDR_TYPE_SIZES[8] = {
        {.typePrefixLength = 1, .networkIdBits = 6, .devDddrBits = 25 },
        {.typePrefixLength = 2, .networkIdBits = 6, .devDddrBits = 24 },
        {.typePrefixLength = 3, .networkIdBits = 9, .devDddrBits = 20 },
        {.typePrefixLength = 4, .networkIdBits = 10, .devDddrBits = 18 },
        {.typePrefixLength = 5, .networkIdBits = 11, .devDddrBits = 16 },
        {.typePrefixLength = 6, .networkIdBits = 13, .devDddrBits = 13 },
        {.typePrefixLength = 7, .networkIdBits = 15, .devDddrBits = 10 },
        {.typePrefixLength = 8, .networkIdBits = 17, .devDddrBits = 7 }
};

DevAddr::DevAddr() {
    memset(&devaddr, 0, sizeof(DEVADDR));
}

DevAddr::DevAddr(const DEVADDR &value) {
    memmove(&devaddr, &value, sizeof(DEVADDR));
}

DevAddr::DevAddr(const DevAddr &value) {
    memmove(&devaddr, &value.devaddr, sizeof(DEVADDR));
}

uint8_t DevAddr::getType() const
{
    return ((NETID_TYPE*) &devaddr)->networkType;
}

void DevAddr::get(DEVADDR &retval) const
{
    memmove(&retval, &devaddr, sizeof(DEVADDR));
}

DEVADDR *DevAddr::getPtr() const
{
    return (DEVADDR *) &devaddr;
}

std::string DevAddr::toString() const
{
    return DEVADDR2string(devaddr);
}

uint32_t DevAddr::get() const
{
    return 0;
}

void DevAddr::setType(uint8_t value)
{
    ((NETID_TYPE*) &devaddr)->networkType = value;
}

void DevAddr::set(const DEVADDR &value) {
    memmove(&devaddr, &value, sizeof(DEVADDR));
}

void DevAddr::set(uint32_t value) {
    int2DEVADDR(devaddr, value);
}

/**
 * Invalidate DevAddr, set RFU to zeroes
 */
void DevAddr::applyTypeMask()
{
    set((uint32_t) get() & getTypeMask());
}

/**
 * DEVADDR has 8 types;
 * 0..2- NwkId = NetId
 * 3..7- NwkId is a part of NetId
 * @see LoRaWAN Backend Interfaces 1.0 Specification
 *      Chapter 13 DevAddr Assignment Table 3
 * @link link-object https://lora-alliance.org/resource_hub/lorawan-back-end-interfaces-v1-0/ @endlink
 * @link link-object https://lora-alliance.org/wp-content/uploads/2020/11/lorawantm-backend-interfaces-v1.0.pdf @endlink
 * @see NetId
 * @return 0..7 NetId type, -1- invalid dev address
 */
int DevAddr::getNwkIdType() const
{
    uint8_t typePrefix8 = ((DEVADDR_TYPE*) &devaddr)->typePrefix;
    if (typePrefix8 == 0xfe)
        return 7;
    typePrefix8 &= 0x7f;
    if (typePrefix8 == 0x7e)
        return 6;
    typePrefix8 &= 0x3f;
    if (typePrefix8 == 0x3e)
        return 5;
    typePrefix8 &= 0x1f;
    if (typePrefix8 == 0x1e)
        return 4;
    typePrefix8 &= 0xf;
    if (typePrefix8 == 0xe)
        return 3;
    typePrefix8 &= 7;
    if (typePrefix8 == 6)
        return 2;
    typePrefix8 &= 3;
    if (typePrefix8 == 2)
        return 1;
    typePrefix8 &= 1;
    if (typePrefix8 == 0)
        return 0;
    return -1;
}

uint32_t DevAddr::getNwkId() const
{
    switch (getNwkIdType())
    {
    case 0:
        return (devaddr[3] & 0x7f) >> 1;
    case 1:
        return devaddr[3] & 0x3f;
    case 2:
        return ((devaddr[3] & 0x1f) << 4) | (devaddr[2] >> 4);
    case 3:
        return ((devaddr[3] & 0xf) << 7) | (devaddr[2] >> 1);
    case 4:
        return ((devaddr[3] & 7) << 8) | devaddr[2];
    case 5:
        return ((devaddr[3] & 3) << 11) | (devaddr[2] << 3) | ((devaddr[1] & 0xe0) >> 5);
    case 6:
        return ((devaddr[3] & 1) << 14) | (devaddr[2] << 6) | ((devaddr[1] & 0xfc) >> 2);
    case 7:
        return (devaddr[2] << 9) | (devaddr[1] << 1) | ((devaddr[0] & 0x80) >> 7);
    default:
        return 0;
    }
}

int DevAddr::getTypeMask() const
{
    return 0;
}