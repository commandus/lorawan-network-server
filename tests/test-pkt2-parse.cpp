#include <string>
#include <iostream>

#include <google/protobuf/message.h>

#include "pkt2/str-pkt2.h"

int main(int argc, char **argv) {
	std::string protoPath = "proto";
	void* env = initPkt2(protoPath, 0);
	if (!env) {
		std::cerr << "Init error" << std::endl;
		exit(1);
	}

	std::string hexData = "01004e01001c9a0ba5f633303032333430363032333533343000011900005ab8f59303000b003e68a68143d40000000502001e0810003e01b21200004e812b4e160000390000221400829486247a0d1c09";
	std::string mt = "iridium.IEPacket"; // iridium.IE_Packet

	for (int i = OUTPUT_FORMAT_JSON; i < OUTPUT_FORMAT_BIN; i++) {
		std::string s = parsePacket(env, INPUT_FORMAT_HEX, i, hexData, mt);
		std::cout << i << ": " << s << std::endl;
	}

	google::protobuf::Message *msg;

	parsePacket2ProtobufMessage((void **) &msg, env, INPUT_FORMAT_HEX, hexData, mt);

	if (msg) {
		std::cout << msg->DebugString() << std::endl;
		delete msg;
	}

	/*
	std::string flds = headerFields(env, mt, ", ");
	std::cout << flds << std::endl;
	*/
	donePkt2(env);

}
