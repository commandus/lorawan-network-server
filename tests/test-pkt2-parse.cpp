#include <string>
#include <regex>
#include <iostream>

#include "pkt2/str-pkt2.h"

int main(int argc, char **argv) {
	std::string protoPath = "proto";
	
	std::cerr << "Init.." << std::endl;
	void* env = initPkt2(protoPath, 0);
	if (!env) {
		std::cerr << "Init error" << std::endl;
		exit(1);
	}

	std::string hexData = "01004e01001c9a0ba5f633303032333430363032333533343000011900005ab8f59303000b003e68a68143d40000000502001e0810003e01b21200004e812b4e160000390000221400829486247a0d1c09";
	
	std::cerr << "Parse.." << std::endl;
	std::string json = parsePacket(env, 1, 0, hexData, "");

	std::cerr << "Done.." << std::endl;
	std::cout << json << std::endl;
	donePkt2(env);
}
