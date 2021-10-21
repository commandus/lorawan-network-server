#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include "utilstring.h"
#include "lora-rejoin.h"

#include "base64/base64.h"

#include <identity-service-dir-txt.h>

void getIdentityTest(
	JsonFileIdentityService &s,
	uint32_t address
)
{
	DeviceId id;
	DEVADDR a;
	int2DEVADDR(a, address);
	if (s.get(a, id) == 0) {
		std::cout 
			<< " Address: " << DEVADDR2string(a) 
			<< " EUI: " << DEVEUI2string(id.deviceEUI) 
			<< " appSKey: " << KEY2string(id.appSKey)
			<< " nwkSKey: " << KEY2string(id.nwkSKey)
			<< std::endl;
	} else {
		std::cerr << "Not found" << std::endl;
	}
}

/**
 * 
 * MH--AD----FC-CN-
 * 400401000280290002e582067cc78ab54e49a538aee94431d5a424
 * 
 * MH 0x40 unconfirmed uplink
 * AD Device address 04010002 
 * FC frame control 80
 * CN frame counter 2900
 * 
 */ 
void decodeTest(
	JsonFileIdentityService &s,
	uint32_t address,
	const std::string &base64Value
)
{
	DeviceId id;
	DEVADDR a;
	int2DEVADDR(a, address);
	if (s.get(a, id) == 0) {
		std::string v = base64_decode(base64Value);
		rfmHeader hdr;
		hdr.parse(v);
		std::cout 
			<< " hex encoded: " << hexString(v) 
			<< " size: " << v.size()
			<< " RFM header size: " << sizeof(RFM_HEADER)
			<< " payload size: " << v.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t) - hdr.header.fctrl.f.foptslen 
			<< " EUI: " << DEVEUI2string(id.deviceEUI) 
			<< " appSKey: " << KEY2string(id.appSKey)
			<< " nwkSKey: " << KEY2string(id.nwkSKey)
			<< std::endl;
		std::cout 
			<< " mtype: " << mtype2string( (MTYPE) hdr.header.macheader.f.mtype)
			<< " mtype int: " << (int) hdr.header.macheader.f.mtype
			<< " major: " << (int) hdr.header.macheader.f.major
			<< " hdr.header.fcnt: " << hdr.header.fcnt 
			<< " hdr.header.fctrl: " << (int) hdr.header.fctrl.i 
			<< " hdr.header.fctrl.f.foptslen: " << (int) hdr.header.fctrl.f.foptslen 
			<< " hdr.header.fctrl.f.ack: " << (int) hdr.header.fctrl.f.ack 
			<< " hdr.header.fctrl.f.adr: " << (int) hdr.header.fctrl.f.adr
			<< " hdr.header.fctrl.f.fpending: " << (int) hdr.header.fctrl.f.fpending
			<< " hdr.header.fctrl.f.rfu: " << (int) hdr.header.fctrl.f.rfu
			<< " hdr.header.devaddr: " << DEVADDR2string(hdr.header.devaddr) 
			<< " hdr.header.macheader: " << (int) hdr.header.macheader.i 
			<< std::endl;

		std::cout 
			<< LoraWANRejoinRequest::toJSONString(v.c_str(), v.size()) << std::endl;
		std::cout 
			<< LoraWANJoinAccept::toJSONString(v.c_str(), v.size()) << std::endl;
	
		uint32_t mico = getMic(v);
		std::cout 
			<< " MIC: " << std::hex << mico
			<< std::dec
			<< std::endl;
				
		int direction = 0;

		int payloadSize = v.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t) - hdr.header.fctrl.f.foptslen;
		std::string p = v.substr(sizeof(RFM_HEADER) + sizeof(uint8_t) + hdr.header.fctrl.f.foptslen, payloadSize);
		std::cout 
			<< " v: " << hexString(p)
			<< " size: " << p.length()
			<< " payloadSize: " << payloadSize
			
			<< std::endl;

		decryptPayload(p, hdr.header.fcnt, direction, hdr.header.devaddr, id.appSKey);
		std::cout 
			<< " hex decoded: " << hexString(p) 
			<< std::endl;

		std::string msg = v.substr(0, v.size() - sizeof(uint32_t));
		uint32_t mic = calculateMIC((const unsigned char*) msg.c_str(), msg.size(), hdr.header.fcnt, direction, hdr.header.devaddr, id.nwkSKey);
		std::cout 
			<< " MIC: " << std::hex << mic
			<< std::endl;
	} else {
		std::cerr << "Not found" << std::endl;
	}
}

void onIdentitiesUpdate
(
	DirTxtIdentityService *service,
	const std::string &path,
	const filewatch::Event &event
) {
	std::cout << path << ": ";
	switch (event)
	{
		case filewatch::Event::added:
			std::cout << "The file was added to the directory." << '\n';
			break;
		case filewatch::Event::removed:
			std::cout << "The file was removed from the directory." << '\n';
			break;
		case filewatch::Event::modified:
			std::cout << "The file was modified. This can be a change in the time stamp or attributes." << '\n';
			break;
		case filewatch::Event::renamed_old:
			std::cout << "The file was renamed and this is the old name." << '\n';
			break;
		case filewatch::Event::renamed_new:
			std::cout << "The file was renamed and this is the new name." << '\n';
			break;
	};
	std::cerr << service->toJsonString() << std::endl;
}

void checkGrep() {
	// std::string s = file2string("doc/lora_devices.txt");
	std::string res = "^([0-9A-Fa-f]{8})\\s([0-9A-Fa-f]{32})\\s([0-9A-Fa-f]{32})\\s([0-9A-Fa-f]{16})\\s([0-9A-Fa-f]{32})\\s([0-9A-Fa-f]{16})\\s([0-9A-Fa-f]{64})\\s([0-9]{1,2})\\s([0-9]{1,2})\\s([0-9]{1,2})";
	std::regex re(res, std::regex_constants::ECMAScript);
	std::string s;
	std::ifstream f("doc/lora_devices.txt", std::ios::in);
	std::smatch matches;
	while (std::getline(f, s)) {
		if (std::regex_search(s, matches, re)) {
			std::cerr << "" << matches[1].str() << std::endl;
			std::cerr << "" << matches[2].str() << std::endl;
			std::cerr << "" << matches[3].str() << std::endl;
			std::cerr << "" << matches[4].str() << std::endl;
			std::cerr << "" << matches[5].str() << std::endl;
			std::cerr << "" << matches[6].str() << std::endl;
			std::cerr << "" << matches[7].str() << std::endl;
			std::cerr << "" << matches[8].str() << std::endl;
			std::cerr << "" << matches[9].str() << std::endl;
			std::cerr << "" << matches[10].str() << std::endl;
			std::stringstream ss(s);
			//ss >> std::hex >> 
		}
	}
	f.close();
}

int main(int argc, char **argv) {
	DirTxtIdentityService s;
	std::cerr << "Init.." << std::endl;
	s.init("doc", NULL);
	std::cerr << "Listen.." << std::endl;
	s.startListen(onIdentitiesUpdate);
	std::cerr << s.toJsonString() << std::endl;
	// putTest(s);
	std::cerr << "Check regex.." << std::endl;
	checkGrep();
	std::cerr << "Identity check.." << std::endl;
	getIdentityTest(s, 0x00000A15);
	std::cerr << "Decoding.." << std::endl;
	// "QDADRQGAvQYCr/WbeIJ/+r95BvZus+xszlkT4Lrr6d91/KnQ5Q==" "ADMxaXNhZzIwEoE3ZjU4NDSS9yoDlRQ="
	decodeTest(s, 0x00000A15, "0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ=="); 
	decodeTest(s, 0x34313235, "0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ=="); 
	decodeTest(s, 0x01450330, "0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ=="); 
	decodeTest(s, 0x00550116, "0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ=="); 
	// must 01002124f13e601e000000004a0000000000000000000000
	//      01002124f13e601e000000004a0000008582e9b18f284ecf

	// decodeTest(s, 0x01450330, "YAFFAzAABVwAj7fiZ+K9xg==");
	
	// decodeTest2();
}
