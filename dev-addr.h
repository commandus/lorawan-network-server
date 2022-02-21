#ifndef DEV_ADDR_H_
/**
 * DevAddr class
 */
#define DEV_ADDR_H_ 1

#define INVALID_ID 0xffffffff

#include <string>

#include "platform.h"
#include "net-id.h"

typedef unsigned char DEVADDR[4];
class DEVADDRINT
{
public:
    uint32_t a;
    DEVADDRINT();
    DEVADDRINT(const DEVADDR &v);
};

struct DEVADDRCompare
{
    bool operator() (const DEVADDR& lhs, const DEVADDR& rhs) const;
};

struct DEVADDRINTCompare
{
    bool operator() (const DEVADDRINT& lhs, const DEVADDRINT& rhs) const;
};

uint32_t DEVADDR2int(const DEVADDR &value);

class DevAddr {
private:
    int setNetIdType(uint8_t value);

    int setNwkId_1_0(uint8_t netIdType, uint32_t value);
    int setNwkId_1_1(uint8_t netIdType, uint32_t value);
    int setNwkId(uint8_t netIdType, uint32_t value);

    int setNwkAddr_1_0(uint8_t netIdType, uint32_t value);
    int setNwkAddr_1_1(uint8_t netIdType, uint32_t value);
    int setNwkAddr(uint8_t netIdType, uint32_t value);

    int setMaxAddress(const NetId &netId);
    int setMinAddress(const NetId &netId);
    static uint32_t getMaxNwkId(uint8_t netTypeId);
    static uint32_t getMaxNwkAddr_1_0(uint8_t netTypeId);
    static uint32_t getMaxNwkAddr_1_1(uint8_t netTypeId);
    static uint32_t getMaxNwkAddr(uint8_t netTypeId);

    uint32_t getNwkId_1_0() const;
    uint32_t getNwkId_1_1() const;
    uint32_t getNwkAddr_1_0() const;
    uint32_t getNwkAddr_1_1() const;

public:
    DEVADDR devaddr;
    DevAddr();
    DevAddr(const DEVADDR &value);
    DevAddr(const DevAddr &value);
    DevAddr(const NETID &netid, uint32_t nwkAddr);
    DevAddr(const NetId &netid, uint32_t nwkAddr);
    DevAddr(uint8_t netTypeId, uint32_t nwkId, uint32_t nwkAddr);
    // min/max addr
    DevAddr(const NetId &netId, bool retMax);
    
    void get(DEVADDR &retval) const;
    void get(DEVADDRINT &retval) const;
    uint32_t get() const;
    std::string toString() const;

    uint8_t getNetIdType() const;
    // NwkId is a part of NetId for types 3..7
    uint32_t getNwkId() const;
    uint32_t getNwkAddr() const;

    void set(const std::string &value);
    void set(const DEVADDR &value);
    void set(uint32_t value);

    int set(uint8_t netTypeId, uint32_t nwkId, uint32_t nwkAddr);
    // Set address only (w/o nwkId)
    int setAddr(uint32_t nwkAddr);
    int set(const NETID &netid, uint32_t nwkAddr);
    int set(const NetId &netid, uint32_t nwkAddr);

    bool operator==(const DevAddr &value) const;
    bool operator==(const DEVADDR &value) const;

    bool empty() const;

    int increment();
    int decrement();
    DevAddr& operator++();  // prefix increment operator
    DevAddr& operator--();  // prefix decrement operator

    static uint8_t getTypePrefixBitsCount(uint8_t netTypeId);

    static uint8_t getNwkIdBitsCount(uint8_t typ);

    static uint8_t getNwkAddrBitsCount(uint8_t typ);

    // return dev address space size
    size_t size();
};

#endif
