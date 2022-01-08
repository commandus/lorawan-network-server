#ifndef DEV_ADDR_H_
/**
 * DevAddr class
 */
#define DEV_ADDR_H_ 1

#define INVALID_ID 0xffffffff

#define TYPE_OUT_OF_RANGE -1
#define NWK_OUT_OF_RANGE -2
#define ADDR_OUT_OF_RANGE -3

#include <string>

#include "platform.h"
#include "net-id.h"

typedef unsigned char DEVADDR[4];

uint32_t DEVADDR2int(const DEVADDR &value);

class DevAddr {
private:
    int setNetIdType(uint8_t value);
    int setNwkId(uint8_t netIdType, uint32_t value);
    int setNwkAddr(uint8_t netIdType, uint32_t value);
    
    int getTypeMask() const;
public:
    DEVADDR devaddr;
    DevAddr();
    DevAddr(const DEVADDR &value);
    DevAddr(const DevAddr &value);
    
    void get(DEVADDR &retval) const;
    uint32_t get() const;
    std::string toString() const;

    int getNetIdType() const;
    // NwkId is a part of NetId for types 3..7
    uint32_t getNwkId() const;
    uint32_t getNwkAddr() const;

    void set(const std::string &value);
    void set(const DEVADDR &value);
    void set(uint32_t value);

    int set(uint8_t netTypeId, uint32_t nwkId, uint32_t nwkAddr);
    int set(const NETID &netid, uint32_t nwkAddr);
    int set(const NetId &netid, uint32_t nwkAddr);

    /**
     * Invalidate DEVADDR, set RFU to zeroes
     */
    void applyTypeMask();
};

#endif
