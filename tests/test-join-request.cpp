#include <string>
#include <iostream>
#include <cassert>

#include "utilstring.h"
#include "base64/base64.h"

#include "errlist.h"
#include "utillora.h"
#include "utilstring.h"

void testJoinRequest2(const std::string &data)
{
	KEY128 key;
    SEMTECH_PREFIX_GW retPrefix;
    rfmMetaData m;
    const struct sockaddr *gwAddress = nullptr;
    SemtechUDPPacket packet(gwAddress, &retPrefix, &m, data, nullptr);
    if (packet.errcode == ERR_CODE_IS_JOIN) {
        std::cerr << "Join request: "
                  << JOIN_REQUEST_FRAME2string(packet.getJoinRequestFrame())
          << std::endl;
    }
}

void testJoinRequest1(const std::string &data)
{
    if (((MHDR *)data.c_str())->f.mtype <= MTYPE_JOIN_ACCEPT) {
        int payloadSize = data.size() - sizeof(MHDR) - sizeof(uint32_t);
        std::string payload = data.substr(sizeof(MHDR), payloadSize);
        // calc MIC
        uint32_t mic = getMic(data);
        KEY128 APPSKEY = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
        uint32_t micCalc = calculateMICJoinRequest((JOIN_REQUEST_HEADER *) data.c_str(), APPSKEY);

        assert(mic == micCalc);

        std::cerr << "*** JOIN request *** parseData MIC: " << std::hex << mic << ", calculated MIC: " << micCalc
                  << ", payload: " << hexString(payload)
                  << std::endl;
    }
}

int main(int argc, char **argv)
{
    std::string data = hex2string("00111213141516171801020304050607088aaacbeb32a2");
    testJoinRequest1(data);
    // testJoinRequest2(data);
}
