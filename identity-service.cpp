#include <map>
#include "identity-service.h"

int IdentityService::joinAccept(
        JOIN_ACCEPT_FRAME_HEADER &retval,
        const NetworkIdentity &networkIdentity
) {
    // return network identifier
    netid.get(retval.netId);
    // return address
    if (isDEVADDREmpty(networkIdentity.devaddr)) {
        // a new one
    } else {
        // re-use old address
        memmove(&retval.devAddr, &networkIdentity.devaddr, sizeof(DEVADDR));
        memmove(&retval.joinNonce, &networkIdentity.joinNonce, sizeof(JOINNONCE));
    }
    return 0;
}

NetId *IdentityService::getNetworkId() {
    return &netid;
}

void IdentityService::setNetworkId(const NetId &value) {
    netid = value;
}
