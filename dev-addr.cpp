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

void DevAddr::get(DEVADDR &retval) const
{
    memmove(&retval, &devaddr, sizeof(DEVADDR));
}

std::string DevAddr::toString() const
{
    return DEVADDR2string(devaddr);
}

uint32_t DevAddr::get() const
{
    return *(uint32_t *) &devaddr;;
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
int DevAddr::getNetIdType() const
{
    uint8_t typePrefix8 = devaddr[3];
    if (typePrefix8 == 0xfe)
        return 7;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0x7e)
        return 6;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0x3e)
        return 5;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0x1e)
        return 4;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0xe)
        return 3;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 6)
        return 2;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 2)
        return 1;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0)
        return 0;
    return -1;
}

int DevAddr::setNetIdType(uint8_t value)
{
    switch (value) {
    case 0:
        devaddr[3] &= 0x7f; // 0b
        break;
    case 1:
        devaddr[3] = (devaddr[3] & 0x3f) | 0x80;   // 10b
        break;
    case 2:
        devaddr[3] = (devaddr[3] & 0x1f) | 0xc0;   // 110b
        break;
    case 3:
        devaddr[3] = (devaddr[3] & 0xf) | 0xe0;   // 1110b
        break;
    case 4:
        devaddr[3] = (devaddr[3] & 7) | 0xf0;  // 11110b
        break;
    case 5:
        devaddr[3] = (devaddr[3] & 3) | 0xf8;  // 111110b
        break;
    case 6:
        devaddr[3] = (devaddr[3] & 1) | 0xfc;  // 1111110b
        break;
    case 7:
        devaddr[3] = 0xfe;  // 11111110b
        break;
    default:
        return TYPE_OUT_OF_RANGE;
    }
    return 0;
}

uint32_t DevAddr::getNwkId() const
{
    switch (getNetIdType())
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
        return INVALID_ID;
    }
}

uint32_t DevAddr::getNwkAddr() const
{
    switch (getNetIdType())
    {
    case 0:
        return ((devaddr[3] & 1) << 24) | (devaddr[2] << 16) | (devaddr[1] << 8) | devaddr[0];
    case 1:
        return (devaddr[2] << 16) | (devaddr[1] << 8) | devaddr[0];
    case 2:
        return ((devaddr[2] & 0xf) << 16) | (devaddr[1] << 8) | devaddr[0];
    case 3:
        return ((devaddr[2] & 3) << 16) | (devaddr[1] << 8) | devaddr[0];
    case 4:
        return (devaddr[1] << 8) | devaddr[0];
    case 5:
        return ((devaddr[1] & 0x1f ) << 8) | devaddr[0];
    case 6:
        return ((devaddr[1] & 3) << 8) | devaddr[0];
    case 7:
        return devaddr[0] & 0x7f;
    default:
        return INVALID_ID;
    }
}

typedef ALIGN struct {
    uint8_t v[4];
} PACKED INT32_ARRAY;		// 4 bytes

/*
 * NwkId clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    10000001111111111111111111111111 0x81FFFFFF
 * 1    11000000111111111111111111111111 0xC0FFFFFF
 * 2    11100000000011111111111111111111 0xE00FFFFF
 * 3    11110000000000111111111111111111 0xF003FFFF
 * 4    11111000000000001111111111111111 0xF800FFFF
 * 5    11111100000000000111111111111111 0xFC007FFF
 * 6    11111110000000000000001111111111 0xFE0003FF
 * 7    11111111000000000000000001111111 0xFF00007F
 *      LSB
 * 0    11111111111111111111111110000001 0xFFFFFF81
 * 1    11111111111111111111111111000000 0xFFFFFFC0
 * 2    11111111111111110000111111100000 0xFFFF0FE0
 * 3    11111111111111110000001111110000 0xFFFF03F0
 * 4    11111111111111110000000011111000 0xFFFF00F8
 * 5    11111111011111110000000011111100 0xFF7F00FC
 * 6    11111111000000110000000011111110 0xFF0300FE
 * 7    01111111000000000000000011111111 0x7F0000FF
 */

#if BYTE_ORDER == BIG_ENDIAN
#define NWKID_CLEAR_0 0x81FFFFFF
#define NWKID_CLEAR_1 0xC0FFFFFF
#define NWKID_CLEAR_2 0xE00FFFFF
#define NWKID_CLEAR_3 0xF003FFFF
#define NWKID_CLEAR_4 0xF800FFFF
#define NWKID_CLEAR_5 0xFC007FFF
#define NWKID_CLEAR_6 0xFE0003FF
#define NWKID_CLEAR_7 0xFF00007F
#else
#define NWKID_CLEAR_0 0xFFFFFF81
#define NWKID_CLEAR_1 0xFFFFFFC0
#define NWKID_CLEAR_2 0xFFFF0FE0
#define NWKID_CLEAR_3 0xFFFF03F0
#define NWKID_CLEAR_4 0xFFFF00F8
#define NWKID_CLEAR_5 0xFF7F00FC
#define NWKID_CLEAR_6 0xFF0300FE
#define NWKID_CLEAR_7 0x7F0000FF
#endif

#define DEVADDR_CLEAR_NWK_ID(typ) *(uint32_t *) &devaddr &= NWKID_CLEAR_ ## typ

#define DEVADDR_SET_NWK_ID_1(u32, prefix_len) *(uint32_t *) &devaddr |= HTON4(u32 << (32 - prefix_len))

#define DEVADDR_SET_NWK_ID(typ, u32, prefix_len) \
    *(uint32_t *) &devaddr = (*(uint32_t *) &devaddr & NWKID_CLEAR_ ## typ ) | HTON4(u32 << (32 - prefix_len))

/*
 * NwkAddr clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    11111110000000000000000000000000 0xFE000000
 * 1    11111111000000000000000000000000 0xFF000000
 * 2    11111111111100000000000000000000 0xFFF00000
 * 3    11111111111111000000000000000000 0xFFFC0000
 * 4    11111111111111110000000000000000 0xFFFF0000
 * 5    11111111111111111000000000000000 0xFFFF8000
 * 6    11111111111111111111110000000000 0xFFFFFC00
 * 7    11111111111111111111111110000000 0xFFFFFF80
 *      LSB
 * 0    00000000000000000000000011111110 0x000000FE
 * 1    00000000000000000000000011111111 0x000000FF
 * 2    00000000000000001111000011111111 0x0000F0FF
 * 3    00000000000000001111110011111111 0x0000FCFF
 * 4    00000000000000001111111111111111 0x0000FFFF
 * 5    00000000100000001111111111111111 0x0089FFFF
 * 6    00000000111111001111111111111111 0x00FCFFFF
 * 7    10000000111111111111111111111111 0x80FFFFFF
 */
#define NWKADDR_CLEAR_0 0xFE000000
#define NWKADDR_CLEAR_1 0xFF000000
#define NWKADDR_CLEAR_2 0xFFF00000
#define NWKADDR_CLEAR_3 0xFFFC0000
#define NWKADDR_CLEAR_4 0xFFFF0000
#define NWKADDR_CLEAR_5 0xFFFF8000
#define NWKADDR_CLEAR_6 0xFFFFFC00
#define NWKADDR_CLEAR_7 0xFFFFFF80
/*
#if BYTE_ORDER == BIG_ENDIAN
#else
#define NWKADDR_CLEAR_0 0x000000FE
#define NWKADDR_CLEAR_1 0x000000FF
#define NWKADDR_CLEAR_2 0x0000F0FF
#define NWKADDR_CLEAR_3 0x0000FCFF
#define NWKADDR_CLEAR_4 0x0000FFFF
#define NWKADDR_CLEAR_5 0x0089FFFF
#define NWKADDR_CLEAR_6 0x00FCFFFF
#define NWKADDR_CLEAR_7 0x80FFFFFF
#endif
*/
#define DEVADDR_SET_NWK_ADDR(typ, u32) \
    *(uint32_t *) &devaddr = ((*(uint32_t *) &devaddr) & NWKADDR_CLEAR_ ## typ ) | u32

int DevAddr::setNwkId(uint8_t netIdType, uint32_t value)
{
    switch (netIdType)
    {
    case 0:
        if (value >= 0x40)
            return NWK_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ID(0, value, 1);
        break;
    case 1:
        if (value >= 0x40)
            return NWK_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ID(1, value, 2);
        break;
    case 2:
        if (value >= 0x200)
            return NWK_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ID(2, value, 3);
        break;
    case 3:
        if (value >= 0x400)
            return NWK_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ID(3, value, 4);
        break;
    case 4:
        if (value >= 0x800)
            return NWK_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ID(4, value, 5);
        break;
    case 5:
        if (value >= 0x2000)
            return NWK_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ID(5, value, 6);
        break;
    case 6:
        if (value >= 0x8000)
            return NWK_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ID(6, value, 7);
        break;
    case 7:
        if (value >= 0x20000)
            return NWK_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ID(7, value, 8);
        break;
    default:
        return NWK_OUT_OF_RANGE;
    }
    return 0;
}

int DevAddr::setNwkAddr(uint8_t netIdType, uint32_t value)
{
    switch (netIdType)
    {
    case 0:
        if (value >= 0x2000000)
            return ADDR_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ADDR(0, value);
        break;
    case 1:
        if (value >= 0x1000000)
            return ADDR_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ADDR(1, value);
        break;
    case 2:
        if (value >= 0x100000)
            return ADDR_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ADDR(2, value);
        break;
    case 3:
        if (value >= 0x40000)
            return ADDR_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ADDR(3, value);
        break;
    case 4:
        if (value >= 0x10000)
            return ADDR_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ADDR(4, value);
        break;
    case 5:
        if (value >= 0x2000)
            return ADDR_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ADDR(5, value);
        break;
    case 6:
        if (value >= 0x400)
            return ADDR_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ADDR(6, value);
        break;
    case 7:
        if (value >= 0x80)
            return ADDR_OUT_OF_RANGE;
        DEVADDR_SET_NWK_ADDR(7, value);
        break;
    default:
        return ADDR_OUT_OF_RANGE;
    }
    return 0;
}
    

int DevAddr::set(
    uint8_t netTypeId, 
    uint32_t nwkId,
    uint32_t nwkAddr
)
{
    int r = setNetIdType(netTypeId);
    if (r)
        return r;
   /*
     r = setNwkId(netTypeId, nwkId);
    if (r)
        return r;
    */
    
    return setNwkAddr(netTypeId, nwkAddr);
    return 0;
}

int DevAddr::getTypeMask() const
{
    return 0;
}