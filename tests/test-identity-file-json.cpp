#include <iostream>
#include "identity-service-file-json.h"
#include "utilstring.h"
#include "base64/base64.h"

void putTest(
	JsonFileIdentityService &s
)
{
	DEVADDR a;

	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	a[3] = 4;

	DEVICEID id;
	id.deviceEUI[0] = 1;
	id.deviceEUI[1] = 2;
	id.deviceEUI[2] = 3;
	id.deviceEUI[3] = 4;
	id.deviceEUI[4] = 5;
	id.deviceEUI[5] = 6;
	id.deviceEUI[6] = 7;
	id.deviceEUI[7] = 8;

	id.nwkSKey[0] = 1;
	id.nwkSKey[1] = 2;
	id.nwkSKey[2] = 3;
	id.nwkSKey[3] = 4;
	id.nwkSKey[4] = 5;
	id.nwkSKey[5] = 6;
	id.nwkSKey[6] = 7;
	id.nwkSKey[7] = 8;
	id.nwkSKey[8] = 9;
	id.nwkSKey[9] = 10;
	id.nwkSKey[10] = 11;
	id.nwkSKey[11] = 12;
	id.nwkSKey[12] = 13;
	id.nwkSKey[13] = 14;
	id.nwkSKey[14] = 15;
	id.nwkSKey[15] = 16;

	id.appSKey[0] = 1;
	id.appSKey[1] = 2;
	id.appSKey[2] = 3;
	id.appSKey[3] = 4;
	id.appSKey[4] = 5;
	id.appSKey[5] = 6;
	id.appSKey[6] = 7;
	id.appSKey[7] = 8;
	id.appSKey[8] = 9;
	id.appSKey[9] = 10;
	id.appSKey[10] = 11;
	id.appSKey[11] = 12;
	id.appSKey[12] = 13;
	id.appSKey[13] = 14;
	id.appSKey[14] = 15;
	id.appSKey[15] = 16;

	s.put(a, id);

	a[0] = 2;
	a[1] = 3;
	a[2] = 4;
	a[3] = 5;
	s.put(a, id);

	a[0] = 3;
	a[1] = 4;
	a[2] = 5;
	a[3] = 6;
	s.put(a, id);

	std::cerr << "--" << std::endl;
	std::vector<NetworkIdentity> l;
	s.list(l, 0, 100);
	for (std::vector<NetworkIdentity>::const_iterator it(l.begin()); it != l.end(); it++) {
		std::cout << it->toString() << std::endl;
	}

	a[0] = 2;
	a[1] = 3;
	a[2] = 4;
	a[3] = 5;
	s.rm(a);

	std::cerr << "--" << std::endl;
	l.clear();
	s.list(l, 0, 100);
	for (std::vector<NetworkIdentity>::const_iterator it(l.begin()); it != l.end(); it++) {
		std::cout << it->toString() << std::endl;
	}

	s.flush();
}

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
		RFMHeader hdr;
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

void decodeTest2()
{
	KEY128 nwkSKey;
	string2KEY(nwkSKey, "99D58493D1205B43EFF938F0F66C339E");

	KEY128 appSKey;
	string2KEY(appSKey, "0A501524F8EA5FCBF9BDB5AD7D126F75");
	// string2KEY(appSKey, "756f127dadb5bdf9cb5feaf82415500a");
	std::string v = base64_decode("QDADRQGAvQYCr/WbeIJ/+r95BvZus+xszlkT4Lrr6d91/KnQ5Q==");	// QK4TBCaAAAABb4ldmIEHFOMmgpU=
	RFMHeader hdr;
	hdr.parse(v);
	std::cout 
		<< " hex encoded: " << hexString(v) 
		<< " size: " << v.size()
		<< " RFM header size: " << sizeof(RFM_HEADER)
		<< " payload size: " << v.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t) - hdr.header.fctrl.f.foptslen 
		<< " appSKey: " << KEY2string(appSKey)
		<< " nwkSKey: " << KEY2string(nwkSKey)
		<< std::endl;
	std::cout 
		<< " FRMPayload: " << hexString(v.substr(sizeof(RFM_HEADER) + sizeof(uint8_t),
			v.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t)))
		<< std::endl;
	uint32_t mico = getMic(v);
	std::cout 
		<< " MIC: " << std::hex << mico
		<< std::endl;
	
	std::cout 
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
	int direction = 0;

	std::string p = v.substr(sizeof(RFM_HEADER) + sizeof(uint8_t),
			v.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t));
	std::cout 
		<< " v: " << hexString(p)
		<< " size: " << p.size()
		<< std::endl;

	decryptPayload(p, hdr.header.fcnt, direction, hdr.header.devaddr, appSKey);
	std::cout 
		<< " hex decoded: " << hexString(p) << " " << p 
		<< std::endl;

	std::string msg = v.substr(0, v.size() - sizeof(uint32_t));
	std::cout 
		<< " message to calc MIC: " << hexString(msg)
		<< " hdr.header.fcnt: " << hdr.header.fcnt
		
		<< std::endl;
	uint32_t mic = calculateMIC((const unsigned char*) msg.c_str(), msg.size(), hdr.header.fcnt, direction, hdr.header.devaddr, nwkSKey);
	std::cout 
		<< " MIC: " << std::hex << mic
		<< std::endl;
}

void decodeTest3(
	JsonFileIdentityService &s,
	uint32_t address,
	const std::string &base64Value
)
{
	DeviceId id;
	DEVADDR a;
	int2DEVADDR(a, address);
	if (s.get(a, id) == 0) {
		std::string v = hex2string(base64Value);
		RFMHeader hdr;
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

int main(int argc, char **argv) {
	JsonFileIdentityService s;
	s.init("identity.json", NULL);
	// putTest(s);
	
	// getIdentityTest(s, 0x01450330);
	decodeTest(s, 0x01450330, "QDADRQGAvQYCr/WbeIJ/+r95BvZus+xszlkT4Lrr6d91/KnQ5Q=="); // "ADMxaXNhZzIwEoE3ZjU4NDSS9yoDlRQ="
	// must 01002124f13e601e000000004a0000000000000000000000
	//      01002124f13e601e000000004a0000008582e9b18f284ecf

	// decodeTest(s, 0x01450330, "YAFFAzAABVwAj7fiZ+K9xg==");
	
	// decodeTest2();
}

/*
QDADRQGAvQYCr/WbeIJ/+r95BvZus+xszlkT4Lrr6d91/KnQ5Q==
*/