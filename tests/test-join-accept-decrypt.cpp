#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "utilstring.h"
#include "base64/base64.h"
#include "lora-encrypt.h"
#include "lora-rejoin.h"

void decodeTest(
	const std::string &skey,
	const std::string &base64Value
)
{
	std::string v = base64_decode(base64Value);
	//v = v.substr(1, 48);	// remove mhdr
	KEY128 key;
	string2KEY(key, skey);
	std::string p = decryptJoinAccept(v, key);
	std::cout << hexString(p) << ", size " << p.size() << " bytes."<< std::endl;
	std::cout << LoraWANJoinAccept::toJSONString(p.c_str(), p.size()) << std::endl;

	LoraWANJoinAccept a(p.c_str(), p.size());
	std::cout << a.toJSONString() << std::endl;

	// KEY128 key = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
	DEVEUI joinEUI = {0, 0, 0, 0, 0, 0, 0, 0 };
	JOINNONCE devNonce = {0, 0, 0};
	std::cout << std::hex << a.mic(joinEUI, devNonce, key) << std::endl;
};

int main(int argc, char **argv) {
	/*
	Device 	    DevEUI           NwkSKey                          AppSKey                          devAddr
	SI-13-232	3434383566378112 313747123434383535003A0066378888 35003A003434383531374712656B7F47 01450330
	sh-2-1		323934344A386D0C 3338470C32393434170026004A386D0C 17002600323934343338470C65717B40 00550116
	pak811-1	3231323549304C0A 34313235343132353431323534313235 34313235343132353431323534313235 34313235
	*/
	decodeTest("00112233445566778899aabbccddeeff", "IEk+61H7ohFvgQ7bN0KXUUI="); // hex 20493eeb51fba2116f810edb3742975142 line 524
	/*
	decodeTest("313747123434383535003A0066378888", "0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ=="); 
	decodeTest("3338470C32393434170026004A386D0C", "0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ=="); 
	decodeTest("34313235343132353431323534313235", "0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ=="); 
	*/
}
