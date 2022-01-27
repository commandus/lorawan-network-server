#include <string>
#include <iomanip>
#include <utillora.h>
#include <iostream>

#include "lora-encrypt.h"
#include "errlist.h"
#include "utilstring.h"

int main(int argc, char **argv)
{
    JOIN_ACCEPT_FRAME frame;
    frame.mhdr.i = 0;
    frame.hdr.dlSettings.RX1DROffset = 1;
    frame.hdr.dlSettings.optNeg = 0;
    frame.hdr.dlSettings.RX2DataRate = 1;
    frame.hdr.joinNonce[0] = 0x11;
    frame.hdr.joinNonce[1] = 0x12;
    frame.hdr.joinNonce[2] = 0x13;
    string2DEVADDR(frame.hdr.devAddr, "1A2B3C4D");
    string2NETID(frame.hdr.netId, "000001");
    frame.hdr.rxDelay = 0x12;
    frame.mic = 0x1234;

    KEY128 key = {0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5};
    std::cerr << hexString(&frame, sizeof(JOIN_ACCEPT_FRAME)) << std::endl;
    encryptJoinAcceptResponse(frame, key);
    std::cerr << hexString(&frame, sizeof(JOIN_ACCEPT_FRAME)) << std::endl;
    encryptJoinAcceptResponse(frame, key);
    std::cerr << hexString(&frame, sizeof(JOIN_ACCEPT_FRAME)) << std::endl;
    std::cout << JOIN_ACCEPT_FRAME2string(frame) << std::endl;
}
