#ifndef NET_ID_H_
/**
 * NetId class
 */
#define NET_ID_H_ 1

#include <string>

#include "platform.h"

#define NETTYPE_OUT_OF_RANGE -1
#define NETID_OUT_OF_RANGE -2

typedef unsigned char NETID[3];

typedef ALIGN struct {
    uint8_t v0;
    uint8_t v1;
    uint8_t v2: 5;
    uint8_t networkType: 3;	// MSB network type 0..7
} PACKED NETID_TYPE;		// 3 bytes

class NetId {
private:
    int getTypeMask() const;
public:
    NETID netid;
    NetId();
    NetId(const NETID &value);
    NetId(const NetId &value);
    uint8_t getType() const;
    void get(NETID &retval)  const;
    uint32_t get() const;
    NETID *getPtr() const;
    uint32_t getNetId() const;
    uint32_t getNwkId() const;
    std::string toString() const;

    void setType(uint8_t value);
    void set(const std::string &value);
    void set(const NETID &value);
    void set(uint32_t value);
    int set(uint8_t netType, uint32_t value);

    /**
     * Invalidate NetId, set RFU to zeroes
     */
    void applyTypeMask();
};

#endif
