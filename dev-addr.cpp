#include <cstring>
#include <sstream>
#include <iostream>

#include "dev-addr.h"
#include "utillora.h"
#include "errlist.h"

#define DEFAULT_LORAWAN_BACKEND_VERSION_MINOR   1

uint32_t DEVADDR2int(const DEVADDR &value)
{
    uint32_t retval;
    *((uint32_t*) &retval) = NTOH4(*((uint32_t*) &value));
    return retval;
}

typedef struct {
    uint8_t networkIdBits;
    uint8_t devDddrBits;
} DEVADDR_TYPE_SIZE;

DEVADDRINT::DEVADDRINT()
    : a(0)
{

}

DEVADDRINT::DEVADDRINT(const DEVADDR &v) {
    memmove(&a, &v, sizeof(DEVADDR));
}

bool DEVADDRINTCompare::operator() (const DEVADDRINT& lhs, const DEVADDRINT& rhs) const {
    return lhs.a < rhs.a;
}

bool DEVADDRCompare::operator() (const DEVADDR& lhs, const DEVADDR& rhs) const
{
    return lhs < rhs;
}

// version 1.0
static const DEVADDR_TYPE_SIZE DEVADDR_TYPE_SIZES_1_0[8] = {
        { 6, 25 },    // 0
        { 6, 24 },    // 1
        { 9, 20 },    // 2
        { 10, 18 },   // 3
        { 11, 16 },   // 4
        { 13, 13 },   // 5
        { 15, 10 },   // 6
        { 17, 7 }     // 7
};

// version 1.1
static const DEVADDR_TYPE_SIZE DEVADDR_TYPE_SIZES_1_1[8] = {
        { 6, 25 },    // 0
        { 6, 24 },    // 1
        { 9, 20 },    // 2
        { 11, 17 },   // 3
        { 12, 15 },   // 4
        { 13, 13 },   // 5
        { 15, 10 },   // 6
        { 17, 7 }     // 7
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

DevAddr::DevAddr(const NETID &netid, uint32_t nwkAddr)
{
    set(netid, nwkAddr);
}

DevAddr::DevAddr(const NetId &netid, uint32_t nwkAddr)
{
    set(netid, nwkAddr);
}

DevAddr::DevAddr(uint8_t netTypeId, uint32_t nwkId, uint32_t nwkAddr)
{
    set(netTypeId, nwkId, nwkAddr);
}

/**
 * Maximum nerwork id & address
 * @param netTypeId type identifier
 */
DevAddr::DevAddr(
    const NetId &netId,
    bool retMax
) {
    if (retMax)
        setMaxAddress(netId);
    else
        setMinAddress(netId);
}

void DevAddr::get(DEVADDR &retval) const
{
    memmove(&retval, &devaddr, sizeof(DEVADDR));
}

void DevAddr::get(DEVADDRINT &retval) const
{
    memmove(&retval.a, &devaddr, sizeof(DEVADDR));
}

int DevAddr::setMaxAddress(const NetId &netId)
{
    uint8_t t = netId.getType();
    int r = setNetIdType(t);
    if (r)
        return r;
    r = setNwkId(t, netId.getNwkId());  // getMaxNwkId(t)
    if (r)
        return r;
    return setNwkAddr(netId.getType(), getMaxNwkAddr(t));
}

int DevAddr::setMinAddress(const NetId &netId)
{
    uint8_t t = netId.getType();
    int r = setNetIdType(netId.getType());
    if (r)
        return r;
    r = setNwkId(t, netId.getNwkId());  // 0
    if (r)
        return r;
    return setNwkAddr(t, 0);
}

size_t DevAddr::size()
{
    uint8_t typ = getNetIdType();
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return (1 << DEVADDR_TYPE_SIZES_1_1[typ].devDddrBits) - 1;
#else
    return (1 << DEVADDR_TYPE_SIZES_1_0[typ].devDddrBits) - 1;
#endif

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
 * DEVADDR has 8 types;
 * 0..2- NwkId = NetId
 * 3..7- NwkId is a part of NetId
 * @see LoRaWAN Backend Interfaces 1.0 Specification
 *      Chapter 13 DevAddr Assignment Table 3
 * @link link-object https://lora-alliance.org/resource_hub/lorawan-back-end-interfaces-v1-0/ @endlink
 * @link link-object https://lora-alliance.org/wp-content/uploads/2020/11/lorawantm-backend-interfaces-v1.0.pdf @endlink
 * @see NetId
 * @return 0..7 NetId type, 8- invalid dev address
 */
uint8_t DevAddr::getNetIdType() const
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
    return 8;
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
        return ERR_CODE_TYPE_OUT_OF_RANGE;
    }
    return 0;
}

/**
 * @return NwkId
 */
uint32_t DevAddr::getNwkId() const
{
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return getNwkId_1_1();
#else
    return getNwkId_1_0();
#endif
}

/**
 * @return NwkId
 * Type Bits                             Hex
 *      MSB
 * 0    T6666660 00000000 00000000 00000000
 * 1    TT666666 00000000 00000000 00000000
 * 2    TTT99999 99990000 00000000 00000000
 * 3    TTTTAAAA AAAAAA00 00000000 00000000
 * 4    TTTTTBBB BBBBBBBB 00000000 00000000
 * 5    TTTTTTDD DDDDDDDD DDD00000 00000000
 * 6    TTTTTTTF FFFFFFFF FFFFFF00 00000000
 * 7    TTTTTTTT 11111111 11111111 10000000
 */
uint32_t DevAddr::getNwkId_1_0() const
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
            return ((devaddr[3] & 0xf) << 6) | (devaddr[2] >> 2);
        case 4:
            return ((devaddr[3] & 7) << 8) | devaddr[2];
        case 5:
            return ((devaddr[3] & 3) << 11) | (devaddr[2] << 3) | (devaddr[1] >> 5); // (devaddr[1] & 0xe0
        case 6:
            return ((devaddr[3] & 1) << 14) | (devaddr[2] << 6) | (devaddr[1] >> 2); // (devaddr[1] & 0xfc
        case 7:
            return (devaddr[2] << 9) | (devaddr[1] << 1) | (devaddr[0] >> 7);  // (devaddr[0] & 0x80)
        default:
            return INVALID_ID;
    }
}

/**
 * @return NwkId
 * Type Bits                             Hex
 *      MSB
 * 0    T6666660 00000000 00000000 00000000
 * 1    TT666666 00000000 00000000 00000000
 * 2    TTT99999 99990000 00000000 00000000
 * 3    TTTTBBBB BBBBBBB0 00000000 00000000
 * 4    TTTTTCCC CCCCCCCC C0000000 00000000
 * 5    TTTTTTDD DDDDDDDD DDD00000 00000000
 * 6    TTTTTTTF FFFFFFFF FFFFFF00 00000000
 * 7    TTTTTTTT 11111111 11111111 10000000
 */
uint32_t DevAddr::getNwkId_1_1() const
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
            return ((devaddr[3] & 7) << 9) | (devaddr[2] << 1) | (devaddr[0] >> 7);
        case 5:
            return ((devaddr[3] & 3) << 11) | (devaddr[2] << 3) | (devaddr[1] >> 5);
        case 6:
            return ((devaddr[3] & 1) << 14) | (devaddr[2] << 6) | (devaddr[1] >> 2);
        case 7:
            return (devaddr[2] << 9) | (devaddr[1] << 1) | (devaddr[0] >> 7);
        default:
            return INVALID_ID;
    }
}

uint32_t DevAddr::getNwkAddr() const
{
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return getNwkAddr_1_1();
#else
    return getNwkAddr_1_0();
#endif
}

/**
 * LoraWAN backend interfaces Version 1.0
 * @return NwkAddr
 * Type Bits                             Hex
 *      MSB
 * 0    T6666660 00000000 00000000 00000000
 * 1    TT666666 00000000 00000000 00000000
 * 2    TTT99999 99990000 00000000 00000000
 * 3    TTTTAAAA AAAAAA00 00000000 00000000
 * 4    TTTTTBBB BBBBBBBB 00000000 00000000
 * 5    TTTTTTDD DDDDDDDD DDD00000 00000000
 * 6    TTTTTTTF FFFFFFFF FFFFFF00 00000000
 * 7    TTTTTTTT 11111111 11111111 10000000
 */
uint32_t DevAddr::getNwkAddr_1_0() const
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

/**
 * LoraWAN backend interfaces Version 1.0
 * @return NwkAddr
 * Type Bits                             Hex
 *      MSB
 * 0    T6666660 00000000 00000000 00000000
 * 1    TT666666 00000000 00000000 00000000
 * 2    TTT99999 99990000 00000000 00000000
 * 3    TTTTBBBB BBBBBBB0 00000000 00000000
 * 4    TTTTTCCC CCCCCCCC C0000000 00000000
 * 5    TTTTTTDD DDDDDDDD DDD00000 00000000
 * 6    TTTTTTTF FFFFFFFF FFFFFF00 00000000
 * 7    TTTTTTTT 11111111 11111111 10000000
 */
uint32_t DevAddr::getNwkAddr_1_1() const
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
            return ((devaddr[2] & 1) << 16) | (devaddr[1] << 8) | devaddr[0];
        case 4:
            return ((devaddr[1] & 0x7f)<< 8) | devaddr[0];
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

/*
 * Version 1.0
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
 */

#define NWKID_CLEAR_1_0_0 0x81FFFFFF
#define NWKID_CLEAR_1_0_1 0xC0FFFFFF
#define NWKID_CLEAR_1_0_2 0xE00FFFFF
#define NWKID_CLEAR_1_0_3 0xF003FFFF
#define NWKID_CLEAR_1_0_4 0xF800FFFF
#define NWKID_CLEAR_1_0_5 0xFC007FFF
#define NWKID_CLEAR_1_0_6 0xFE0003FF
#define NWKID_CLEAR_1_0_7 0xFF00007F

/*
 * Version 1.1
 * NwkId clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    10000001111111111111111111111111 0x81FFFFFF
 * 1    11000000111111111111111111111111 0xC0FFFFFF
 * 2    11100000000011111111111111111111 0xE00FFFFF
 * 3    11110000000000011111111111111111 0xF001FFFF
 * 4    11111000000000000111111111111111 0xF8007FFF
 * 5    11111100000000000111111111111111 0xFC007FFF
 * 6    11111110000000000000001111111111 0xFE0003FF
 * 7    11111111000000000000000001111111 0xFF00007F
 */

#define NWKID_CLEAR_1_1_0 0x81FFFFFF
#define NWKID_CLEAR_1_1_1 0xC0FFFFFF
#define NWKID_CLEAR_1_1_2 0xE00FFFFF
#define NWKID_CLEAR_1_1_3 0xF001FFFF
#define NWKID_CLEAR_1_1_4 0xF8007FFF
#define NWKID_CLEAR_1_1_5 0xFC007FFF
#define NWKID_CLEAR_1_1_6 0xFE0003FF
#define NWKID_CLEAR_1_1_7 0xFF00007F

#define DEVADDR_SET_NWK_ID_1_0(typ, u32, prefix_n_mnwkid_len) \
    *(uint32_t *) &devaddr = (*(uint32_t *) &devaddr & NWKID_CLEAR_1_0_ ## typ ) | (u32 << (32 - prefix_n_mnwkid_len))

#define DEVADDR_SET_NWK_ID_1_1(typ, u32, prefix_n_mnwkid_len) \
    *(uint32_t *) &devaddr = (*(uint32_t *) &devaddr & NWKID_CLEAR_1_1_ ## typ ) | (u32 << (32 - prefix_n_mnwkid_len))

/*
 * LoraWAN beckend interfaces Version 1.0
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
 */
#define NWKADDR_CLEAR_V1_0_0 0xFE000000
#define NWKADDR_CLEAR_V1_0_1 0xFF000000
#define NWKADDR_CLEAR_V1_0_2 0xFFF00000
#define NWKADDR_CLEAR_V1_0_3 0xFFFC0000
#define NWKADDR_CLEAR_V1_0_4 0xFFFF0000
#define NWKADDR_CLEAR_V1_0_5 0xFFFF8000
#define NWKADDR_CLEAR_V1_0_6 0xFFFFFC00
#define NWKADDR_CLEAR_V1_0_7 0xFFFFFF80

#define DEVADDR_SET_NWK_ADDR_1_0(typ, u32) \
    *(uint32_t *) &devaddr = ((*(uint32_t *) &devaddr) & NWKADDR_CLEAR_V1_0_ ## typ ) | u32

/*
 * LoraWAN beckend interfaces Version 1.1
 * NwkAddr clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    11111110000000000000000000000000 0xFE000000
 * 1    11111111000000000000000000000000 0xFF000000
 * 2    11111111111100000000000000000000 0xFFF00000
 * 3    11111111111111100000000000000000 0xFFFE0000
 * 4    11111111111111111000000000000000 0xFFFF8000
 * 5    11111111111111111000000000000000 0xFFFF8000
 * 6    11111111111111111111110000000000 0xFFFFFC00
 * 7    11111111111111111111111110000000 0xFFFFFF80
 */
#define NWKADDR_CLEAR_V1_1_0 0xFE000000
#define NWKADDR_CLEAR_V1_1_1 0xFF000000
#define NWKADDR_CLEAR_V1_1_2 0xFFF00000
#define NWKADDR_CLEAR_V1_1_3 0xFFFE0000
#define NWKADDR_CLEAR_V1_1_4 0xFFFF8000
#define NWKADDR_CLEAR_V1_1_5 0xFFFF8000
#define NWKADDR_CLEAR_V1_1_6 0xFFFFFC00
#define NWKADDR_CLEAR_V1_1_7 0xFFFFFF80

#define DEVADDR_SET_NWK_ADDR_1_1(typ, u32) \
    *(uint32_t *) &devaddr = ((*(uint32_t *) &devaddr) & NWKADDR_CLEAR_V1_1_ ## typ ) | u32

/*
 * LoraWAN beckend interfaces Version 1.1
 * NwkAddr clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    11111110000000000000000000000000 0xFE000000
 * 1    11111111000000000000000000000000 0xFF000000
 * 2    11111111111100000000000000000000 0xFFF00000
 * 3    11111111111111100000000000000000 0xFFFE0000
 * 4    11111111111111111000000000000000 0xFFFF8000
 * 5    11111111111111111000000000000000 0xFFFF8000
 * 6    11111111111111111111110000000000 0xFFFFFC00
 * 7    11111111111111111111111110000000 0xFFFFFF80
 */
#define NWKADDR_CLEAR_V1_1_0 0xFE000000
#define NWKADDR_CLEAR_V1_1_1 0xFF000000
#define NWKADDR_CLEAR_V1_1_2 0xFFF00000
#define NWKADDR_CLEAR_V1_1_3 0xFFFE0000
#define NWKADDR_CLEAR_V1_1_4 0xFFFF8000
#define NWKADDR_CLEAR_V1_1_5 0xFFFF8000
#define NWKADDR_CLEAR_V1_1_6 0xFFFFFC00
#define NWKADDR_CLEAR_V1_1_7 0xFFFFFF80

int DevAddr::setNwkId(uint8_t netIdType, uint32_t value)
{
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return setNwkId_1_1(netIdType, value);
#else
    return setNwkId_1_0(netIdType, value);
#endif
}

int DevAddr::setNwkId_1_0(uint8_t netIdType, uint32_t value)
{
    switch (netIdType)
    {
        case 0:
            if (value >= 0x40)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(0, value, 7);
            break;
        case 1:
            if (value >= 0x40)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(1, value, 8);
            break;
        case 2:
            if (value >= 0x200)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(2, value, 12);
            break;
        case 3:
            if (value >= 0x400)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(3, value, 14);
            break;
        case 4:
            if (value >= 0x800)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(4, value, 16);
            break;
        case 5:
            if (value >= 0x2000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(5, value, 19);
            break;
        case 6:
            if (value >= 0x8000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(6, value, 22);
            break;
        case 7:
            if (value >= 0x20000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(7, value, 25);
            break;
        default:
            return ERR_CODE_NWK_OUT_OF_RANGE;
    }
    return 0;
}

int DevAddr::setNwkId_1_1(uint8_t netIdType, uint32_t value)
{
    switch (netIdType)
    {
        case 0:
            if (value >= 0x40)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(0, value, 7);
            break;
        case 1:
            if (value >= 0x40)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(1, value, 8);
            break;
        case 2:
            if (value >= 0x200)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(2, value, 12);
            break;
        case 3:
            if (value >= 0x800)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(3, value, 14);
            break;
        case 4:
            if (value >= 0x1000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(4, value, 16);
            break;
        case 5:
            if (value >= 0x2000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(5, value, 19);
            break;
        case 6:
            if (value >= 0x8000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(6, value, 22);
            break;
        case 7:
            if (value >= 0x20000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(7, value, 25);
            break;
        default:
            return ERR_CODE_NWK_OUT_OF_RANGE;
    }
    return 0;
}

int DevAddr::setNwkAddr(uint8_t netIdType, uint32_t value)
{
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return setNwkAddr_1_1(netIdType, value);
#else
    return setNwkAddr_1_0(netIdType, value);
#endif
}

int DevAddr::setNwkAddr_1_0(uint8_t netIdType, uint32_t value)
{
    switch (netIdType)
    {
        case 0:
            if (value >= 0x2000000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(0, value);
            break;
        case 1:
            if (value >= 0x1000000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(1, value);
            break;
        case 2:
            if (value >= 0x100000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(2, value);
            break;
        case 3:
            if (value >= 0x40000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(3, value);
            break;
        case 4:
            if (value >= 0x10000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(4, value);
            break;
        case 5:
            if (value >= 0x2000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(5, value);
            break;
        case 6:
            if (value >= 0x400)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(6, value);
            break;
        case 7:
            if (value >= 0x80)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(7, value);
            break;
        default:
            return ERR_CODE_ADDR_OUT_OF_RANGE;
    }
    return 0;
}
int DevAddr::setNwkAddr_1_1(uint8_t netIdType, uint32_t value)
{
    switch (netIdType)
    {
        case 0:
            if (value >= 0x2000000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(0, value);
            break;
        case 1:
            if (value >= 0x1000000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(1, value);
            break;
        case 2:
            if (value >= 0x100000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(2, value);
            break;
        case 3:
            if (value >= 0x20000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(3, value);
            break;
        case 4:
            if (value >= 0x8000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(4, value);
            break;
        case 5:
            if (value >= 0x2000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(5, value);
            break;
        case 6:
            if (value >= 0x400)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(6, value);
            break;
        case 7:
            if (value >= 0x80)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(7, value);
            break;
        default:
            return ERR_CODE_ADDR_OUT_OF_RANGE;
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
    r = setNwkId(netTypeId, nwkId);
    if (r)
        return r;
    return setNwkAddr(netTypeId, nwkAddr);
}

// Set address only (w/o nwkId)
int DevAddr::setAddr(
    uint32_t nwkAddr
) {
    return setNwkAddr(getNetIdType(), nwkAddr);
}

int DevAddr::set(
    const NETID &netid,
    uint32_t nwkAddr
)
{
    NetId v(netid);
    return set(v.getType(), v.getNwkId(), nwkAddr);
}

int DevAddr::set(
    const NetId &netid,
    uint32_t nwkAddr
)
{
    return set(netid.getType(), netid.getNwkId(), nwkAddr);
}

bool DevAddr::operator==(const DevAddr &value) const
{
    return memcmp(&value.devaddr, &devaddr, sizeof(DEVADDR)) == 0;
}

bool DevAddr::operator==(const DEVADDR &value) const
{
    return memcmp(&value, &devaddr, sizeof(DEVADDR)) == 0;
}

bool DevAddr::empty() const
{
    return isDEVADDREmpty(devaddr);
}

// prefix increment operator
DevAddr& DevAddr::operator++()
{
    increment();
    return *this;
}

// prefix decrement operator
DevAddr& DevAddr::operator--()
{
    decrement();
    return *this;
}

int DevAddr::increment()
{
    return setAddr(getNwkAddr() + 1);
}

int DevAddr::decrement()
{
    return setAddr(getNwkAddr() - 1);
}

uint32_t DevAddr::getMaxNwkId(uint8_t netTypeId) {
    switch (netTypeId)
    {
        case 0:
            return 0x40 - 1;
        case 1:
            return 0x40 - 1;
        case 2:
            return 0x200 - 1;
        case 3:
            return 0x400 - 1;
        case 4:
            return 0x800 - 1;
        case 5:
            return 0x2000 - 1;
        case 6:
            return 0x8000 - 1;
        case 7:
            return 0x20000 - 1;
        default:
            return ERR_CODE_NWK_OUT_OF_RANGE;
    }
}

uint32_t DevAddr::getMaxNwkAddr(uint8_t netTypeId) {
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return getMaxNwkAddr_1_1(netTypeId);
#else
    return getMaxNwkAddr_1_0(netTypeId);
#endif
}

uint32_t DevAddr::getMaxNwkAddr_1_0(uint8_t netTypeId) {
    switch (netTypeId)
    {
        case 0:
            return 0x2000000 - 1;
        case 1:
            return 0x1000000 - 1;
        case 2:
            return 0x100000 - 1;
        case 3:
            return 0x40000 - 1;
        case 4:
            return 0x10000 - 1;
        case 5:
            return 0x2000 - 1;
        case 6:
            return 0x400 - 1;
        case 7:
            return 0x80 - 1;
        default:
            return ERR_CODE_ADDR_OUT_OF_RANGE;
    }
}

uint32_t DevAddr::getMaxNwkAddr_1_1(uint8_t netTypeId) {
    switch (netTypeId)
    {
        case 0:
            return 0x2000000 - 1;
        case 1:
            return 0x1000000 - 1;
        case 2:
            return 0x100000 - 1;
        case 3:
            return 0x20000 - 1;
        case 4:
            return 0x8000 - 1;
        case 5:
            return 0x2000 - 1;
        case 6:
            return 0x400 - 1;
        case 7:
            return 0x80 - 1;
        default:
            return ERR_CODE_ADDR_OUT_OF_RANGE;
    }
}

uint8_t DevAddr::getTypePrefixBitsCount
(
    uint8_t netTypeId
)
{
    return netTypeId + 1;
}

uint8_t DevAddr::getNwkIdBitsCount(uint8_t typ) {
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return DEVADDR_TYPE_SIZES_1_1[typ].networkIdBits;
#else
    return DEVADDR_TYPE_SIZES_1_0[typ].networkIdBits;
#endif
}

uint8_t DevAddr::getNwkAddrBitsCount(uint8_t typ) {
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return DEVADDR_TYPE_SIZES_1_1[typ].devDddrBits;
#else
    return DEVADDR_TYPE_SIZES_1_0[typ].devDddrBits;
#endif
}
