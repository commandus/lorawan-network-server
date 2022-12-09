#include "identity-service.h"

IdentityService::IdentityService()
{

}

IdentityService::IdentityService(
    const IdentityService &value
)
    : netid(value.netid)
{

}
 IdentityService::~IdentityService()
{

}

int IdentityService::joinAccept(
        JOIN_ACCEPT_FRAME_HEADER &retval,
        NetworkIdentity &networkIdentity
) {
    if (isDEVADDREmpty(networkIdentity.devaddr)) {
        // No assigned address, generate a new one
        // return network identifier
        netid.get(retval.netId);
        NetworkIdentity newNetworkIdentity(networkIdentity);
        int r = this->next(newNetworkIdentity);
        if (r)
            return r;
        // return network identifier
        netid.get(retval.netId);
        // return address
        memmove(&retval.devAddr, &newNetworkIdentity.devaddr, sizeof(DEVADDR));
        memmove(&retval.joinNonce, &newNetworkIdentity.joinNonce, sizeof(JOINNONCE));
        // return address in the identity too
        memmove(&networkIdentity.devaddr, &newNetworkIdentity.devaddr, sizeof(DEVADDR));
    } else {
        // re-use old address, return network identifier
        netid.get(retval.netId);
        // return address
        memmove(&retval.devAddr, &networkIdentity.devaddr, sizeof(DEVADDR));
        memmove(&retval.joinNonce, &networkIdentity.joinNonce, sizeof(JOINNONCE));
        // return address in the identity too
        memmove(&networkIdentity.devaddr, &networkIdentity.devaddr, sizeof(DEVADDR));
    }
    return 0;
}

NetId *IdentityService::getNetworkId() {
    return &netid;
}

void IdentityService::setNetworkId(const NetId &value) {
    netid = value;
}
