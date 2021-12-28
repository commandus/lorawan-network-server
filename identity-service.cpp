#include <map>
#include "identity-service.h"

int IdentityService::joinAccept(
        JOIN_ACCEPT_FRAME_HEADER &retval,
        const NetworkIdentity &networkIdentity
) {
    // return network identifier
    getNetworkId(retval.netId);
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

void IdentityService::getNetworkId(NETID &retval) {
    retval[0] = 0;
    retval[1] = 0;
    retval[2] = 1;
}
