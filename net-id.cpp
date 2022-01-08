#include <cstring>
#include <sstream>
#include "net-id.h"
#include "utillora.h"

NetId::NetId() {
    memset(&netid, 0, sizeof(NETID));
}

NetId::NetId(const NETID &value) {
    memmove(&netid, &value, sizeof(NETID));
}

NetId::NetId(const NetId &value) {
    memmove(&netid, &value.netid, sizeof(NETID));
}

uint8_t NetId::getType() const
{
    return ((NETID_TYPE*) &netid)->networkType;
}

void NetId::get(NETID &retval) const
{
    memmove(&retval, &netid, sizeof(NETID));
}

NETID *NetId::getPtr() const
{
    return (NETID *) &netid;
}

std::string NetId::toString() const
{
    std::stringstream ss;
    ss << std::hex << NETID2int(netid);
    return ss.str();
}

uint32_t NetId::get() const
{
    return NETID2int(netid);
}

static uint8_t MASK_NWK_ID[8] = {
    6, 6, 9, 10, 11, 13, 15, 17
};

uint32_t NetId::getNwkId() const
{
    return NETID2int(netid) & MASK_NWK_ID[((NETID_TYPE*) &netid)->networkType];
}

void NetId::setType(uint8_t value)
{
    ((NETID_TYPE*) &netid)->networkType = value;
}

void NetId::set(const NETID &value) {
    memmove(&netid, &value, sizeof(NETID));
}

void NetId::set(uint32_t value) {
    int2NETID(netid, value);
}

/**
 * Invalidate NetId, set RFU to zeroes
 */
void NetId::applyTypeMask()
{
    set((uint32_t) get() & getTypeMask());
}

/**
 * NETID has 8 types
 * @see LoRaWAN Backend Interfaces 1.0 Specification
 *      Chapter 13 DevAddr Assignment
 * @return
 */
int NetId::getTypeMask() const
{
    switch (((NETID_TYPE*) &netid)->networkType) {
        case 0:
        case 1:
            return (1 << 6) - 1;    // 15 unused bits, 3 bits type, 6 bits- identifier
        case 2:
            return (1 << 9) - 1;    // 12 unused bits, 3 bits type, 9 bits- identifier
        default:    // 3..7
            return (1 << 21) - 1;   // 0 unused bits, 3 bits type, 21 bits- identifier
    }
}
