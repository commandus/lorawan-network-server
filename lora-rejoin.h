#ifndef LORA_REJOIN_H_
#define LORA_REJOIN_H_	1

#include "utillora.h"

typedef PACK(struct {					// Bytes
	MHDR macheader;						// 1 MAC header 
	uint8_t rrtype;						// 1 0- reset a device context including all radio parameters, 2- rekey a device or change its DevAdd
	// 0- reset a device context including all radio parameters
	// 2- restore a lost session context (
	NETID netid;						// 3
	DEVEUI deveui;						// 8
	uint16_t rjcount0;					// 2
} ) LORAWAN_REJOIN_REQUEST_0_2;	// 15 bytes

typedef PACK( struct {					// Bytes
	MHDR macheader;						// 1 MAC header message type 6
	uint8_t rrtype;						// 1 1- restore a lost session context (
	DEVEUI joineui;						// 8
	DEVEUI deveui;						// 8
	uint16_t rjcount1;					// 2
} ) LORAWAN_REJOIN_REQUEST_1;			// 20 bytes

typedef PACK( struct {
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
} ) LORAWAN_REJOIN_REQUEST;			// 15 or 20 bytes

class LoraWANRejoinRequest {
private:
	LORAWAN_REJOIN_REQUEST data;
public:
	LoraWANRejoinRequest(const void *buffer, size_t size);
	std::string toJSONString() const;
	static std::string toJSONString(const void *buffer, size_t size);
};

//  MHDR | JoinNonce | NetID | DevAddr | DLSettings | RxDelay | CFList

typedef PACK( struct {
	MHDR macheader;						// 1 MAC header message type 6
	JOINNONCE joinNonce;				// 3
	NETID netid;						// 3
	DEVADDR devaddr;					// 4
										// 1 dlsettings
	uint8_t optneg: 1;					// dlsettings OptNeg bit 7
	uint8_t rx1droffset: 3;				// dlsettings RX1DRoffset 
	uint8_t rx2datarate: 4;				// dlsettings RX2 Data rate bits 0..3

	uint8_t rxdelay;					// 1
} ) LORAWAN_JOIN_ACCEPT_HEADER;	// 13 bytes

typedef PACK( struct {
	LORAWAN_JOIN_ACCEPT_HEADER header;	// 13
	CFLIST cflist;						// 16
	uint32_t mic;						// 4
} ) LORAWAN_JOIN_ACCEPT_LONG;		// 33 bytes

typedef PACK( struct {
	LORAWAN_JOIN_ACCEPT_HEADER header;	// 13
	uint32_t mic;						// 4
} ) LORAWAN_JOIN_ACCEPT_SHORT;		// 17 bytes

typedef PACK( struct {
	union {
		LORAWAN_JOIN_ACCEPT_SHORT s;	// 13
		LORAWAN_JOIN_ACCEPT_LONG l;		// 17
	};
} ) LORAWAN_JOIN_ACCEPT;			// 17 bytes

class LoraWANJoinAccept {
private:
	LORAWAN_JOIN_ACCEPT data;
	bool hasCFList;
public:
	LoraWANJoinAccept(const void *buffer, size_t size);
	std::string toJSONString() const;
	static std::string toJSONString(const void *buffer, size_t size);
	uint32_t mic(
		const DEVEUI &joinEUI,
		const JOINNONCE &devNonce,
		const KEY128 &key
	);
};

#endif
