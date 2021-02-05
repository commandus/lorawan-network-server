#include <iostream>
#include "utillora.h"
#include "utilstring.h"

int main(int argc, char **argv) {
	semtechUDPPacket packet;
	packet.setGatewayId("00006cc3743eed46");

	packet.setDeviceEUI("1122334455667788");
	packet.setDeviceAddr("11111111");
	packet.setNetworkSessionKey("11111111111111111111111111111111");
	packet.setApplicationSessionKey("11111111111111111111111111111111");
	packet.setFrameCounter(0);
	packet.setPayload(1, "123");

	std::cerr << "GW:  " << deviceEui2string(packet.prefix.mac) << std::endl;
	std::cerr << "EUI: " << deviceEui2string(packet.deviceEUI) << std::endl;
	std::cerr << "RFM packet: " << hexString(packet.serialize2RfmPacket()) << std::endl;

	std::string s(packet.toString());
	std::cout << s << std::endl;
	semtechUDPPacket packet2(s.c_str(), s.size());
	std::cerr << packet2.toString() << std::endl;
}
