#ifndef LORA_REJOIN_H_
#define LORA_REJOIN_H_	1

#include "utillora.h"

typedef ALIGN struct {
	MHDR macheader;				// MAC header message type 6
	uint8_t rrtype;				// 0- reset a device context including all radio parameters, 2- rekey a device or change its DevAdd
	// 0- reset a device context including all radio parameters
	// 2- restore a lost session context (
	NETID netid;		// 3
	DEVEUI deveui;		// 8
	uint16_t rjcount0;	// 2
} PACKED LORAWAN_REJOIN_REQUEST_0_2;			// 15 bytes

typedef ALIGN struct {
	MHDR macheader;		// MAC header message type 6
	uint8_t rrtype;		// 1- restore a lost session context (
	DEVEUI joineui;		// 8
	DEVEUI deveui;		// 8
	uint16_t rjcount1;	// 2
} PACKED LORAWAN_REJOIN_REQUEST_1;			// 20 bytes

typedef ALIGN struct {
	MHDR macheader;				// MAC header message type 6
	// rejoin 
	uint8_t rejointype;			// 0- reset a device context including all radio parameters,  1- same, 2- rekey a device or change its DevAdd
	union {
		// 0- reset a device context including all radio parameters
		// 2- restore a lost session context (
		struct {
			NETID netid;		// 3
			DEVEUI deveui;		// 8
			uint16_t rjcount0;	// 2
		} reset;
		// 1- restore a lost session context (
		struct {
			DEVEUI joineui;		// 8
			DEVEUI deveui;		// 8
			uint16_t rjcount1;	// 2
		} restore;
	} request;
} PACKED LORAWAN_REJOIN_REQUEST;			// 15 or 20 bytes

class LoraWANRejoinRequest {
private:
	LORAWAN_REJOIN_REQUEST data;
public:
	LoraWANRejoinRequest(const void *buffer, size_t size);
	std::string toJSONString() const;
	static std::string toJSONString(const void *buffer, size_t size);
};

typedef ALIGN struct {
	MHDR macheader;				// 1 MAC header message type 6
	JOIN_NONCE joinNonce;		// 3
	NETID netid;				// 3
	DEVADDR devaddr;			// 4
								// 1 dlsettings
	uint8_t optneg: 1;			// dlsettings OptNeg bit 7
	uint8_t rx1droffset: 3;		// dlsettings RX1DRoffset 
	uint8_t rx2datarate: 4;		// dlsettings RX2 Data rate bits 0..3

	uint8_t rxdelay;			// 1
	// optional list of network parameters (CFList)
	CFLIST cflist;				// 16
} PACKED LORAWAN_JOIN_ACCEPT;	// 13 or 29 bytes

class LoraWANJoinAccept {
private:
	LORAWAN_JOIN_ACCEPT data;
public:
	LoraWANJoinAccept(const void *buffer, size_t size);
	std::string toJSONString() const;
	static std::string toJSONString(const void *buffer, size_t size);
};

#endif
