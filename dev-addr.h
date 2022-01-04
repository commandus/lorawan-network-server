#ifndef DEV_ADDR_H_
/**
 * DevAddr class
 */
#define DEV_ADDR_H_ 1

#define INVALID_ID 0xffffffff

#include <string>

#include "platform.h"

typedef unsigned char DEVADDR[4];

typedef ALIGN struct {
    uint8_t v[3];
    uint8_t typePrefix;  	// MSB network type
} PACKED DEVADDR_TYPE;		// 4 bytes

uint32_t DEVADDR2int(const DEVADDR &value);

class DevAddr {
private:
    DEVADDR devaddr;
    int getNwkIdType() const;
    int getTypeMask() const;
public:
    DevAddr();
    DevAddr(const DEVADDR &value);
    DevAddr(const DevAddr &value);
    uint8_t getType() const;

    void get(DEVADDR &retval) const;
    uint32_t get() const;
    DEVADDR *getPtr() const;
    std::string toString() const;

    // NwkId is a part of NetId for types 3..7
    uint32_t getNwkId() const;
    uint32_t getNwkAddr() const;

    void setType(uint8_t value);
    void set(const std::string &value);
    void set(const DEVADDR &value);
    void set(uint32_t value);

    /**
     * Invalidate DEVADDR, set RFU to zeroes
     */
    void applyTypeMask();
};

#endif
