#include <string>
#include <inttypes.h>
#include "platform.h"
#include "aes-128.h"

typedef unsigned char DEVADDR[4];
typedef unsigned char DEVUEI[8];

typedef ALIGN struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// random token
	uint8_t tag;				// PUSH_DATA 0x00 PULL_DATA 0x02 PUSH_DATA
	DEVUEI mac;					// 4-11	Gateway unique identifier (MAC address). For example : 00:0c:29:19:b2:37
} PACKED SEMTECH_LORA_PREFIX;	// 12 bytes

/**
 * 
 * MAC message types(7..5) RFU(4..2) Major(1..0)
 * Hex   Bin Hex
 * 00000000 0    Join-request
 * 00100000 20   Join-accept
 * 01000000 40   Unconfirmed Data Up
 * 01100000 60   Unconfirmed Data Down
 * 10000000 80   Confirmed Data Up
 * 10100000 A0   Confirmed Data Down
 * 11000000 C0   Rejoin-request
 * 11100000 E0   Proprietary
*/
typedef ALIGN struct {
	// MAC heaader byte: message type, RFU, Major
	uint8_t macheader;			// 0x40 unconfirmed uplink
	// Frame header (FHDR)
	DEVADDR devaddr;			// MAC address
	uint8_t framecontrol;		// 
	uint16_t framecountertx;	// frame counter
	// FOpts 0..15
} PACKED RFM_HEADER;			// 8 bytes, +1

typedef ALIGN struct {
	uint8_t fopts[15];
} PACKED FOPTS;					// 0..15 bytes

typedef enum {
	LORA = 0,
	FSK = 1
} MODULATION;

class rfmMetaData {
public:
	time_t t;					// UTC time of pkt RX, us precision, ISO 8601 'compact' format
	uint8_t chan;				// Concentrator "IF" channel used for RX (unsigned integer)
	uint8_t rfch;				// Concentrator "RF chain" used for RX (unsigned integer)
	uint32_t freq;				// RX central frequency in MHz (unsigned float, Hz precision) 868.900000
	int8_t stat;				// CRC status: 1 = OK, -1 = fail, 0 = no CRC
	MODULATION modu;			// LORA, FSK
	std::string datr;			// LoRa datarate identifier e.g. "SF7BW125"
	uint32_t bps;				// FSK bite per second
	std::string codr;			// LoRa ECC coding rate identifier e.g. "4/6"
	int16_t rssi;				// RSSI in dBm (signed integer, 1 dB precision) e.g. -35
	float lsnr; 				// Lora SNR ratio in dB (signed float, 0.1 dB precision) e.g. 5.1
	rfmMetaData();
	uint32_t tmst();			// GPS time of pkt RX, number of milliseconds since 06.Jan.1980
	std::string modulation();
	std::string frequency();
	std::string snrratio();
};

class rfmHeader {
private:
	void ntoh();
public:	
	RFM_HEADER header;
	FOPTS fopts;
	uint8_t fport;

	rfmHeader();
	rfmHeader(
		const RFM_HEADER &header
	);
	rfmHeader(
		const DEVADDR &addr
	);
	rfmHeader(
		const DEVADDR &addr,
		uint16_t frameCounter
	);
	rfmHeader(
		const DEVADDR &addr,
		uint16_t frameCounter,
		uint8_t framePort
	);
	
	rfmHeader(
		const std::string &value
	);

	std::string toString();
	bool parse(const std::string &value);
};

class semtechUDPPacket {
private:
	rfmMetaData metadata;
	rfmHeader header;
	std::string payload;
	int parse(const std::string &packet);
public:	
	SEMTECH_LORA_PREFIX prefix;
	DEVUEI deviceEUI;
	KEY128 nwkSKey;
	KEY128 appSKey;

	semtechUDPPacket();
	semtechUDPPacket(const std::string &packet, const std::string &devaddr, const std::string &appskey);
	std::string serialize2RfmPacket();
	std::string toString();

	RFM_HEADER *getRfmHeader();
	rfmHeader *getHeader();
	void setRfmHeader(const RFM_HEADER &value);

	std::string getDeviceEUI();
	void setDeviceEUI(const std::string &value);

	std::string getDeviceAddr();
	void setDeviceAddr(const std::string &value);
	void setGatewayId(const std::string &value);
	void setNetworkSessionKey(const std::string &value);
	void setApplicationSessionKey(const std::string &value);
	void setFrameCounter(uint16_t value);

	std::string getPayload();
	int setPayload(uint8_t port, const std::string &payload);
	int setPayload(const std::string &value);
};

/**
 * @brief constructs a LoRaWAN package and sends it
 * @param data pointer to the array of data that will be transmitted
 * @param dataLength bytes to be transmitted
 * @param frameCounterUp  frame counter of upstream frames
 * @param devAddr 4 bytes long device address
 * @param nwkSkey 128 bits network key
 * @param appSkey 128 bits application key
 */
std::string loraDataJson(
	unsigned char *data, 
	unsigned char dataLength,
	unsigned int frameCounterTx,
	DEVADDR &devAddr,
	KEY128 &nwkSkey,
	KEY128 &appSkey
);

void setKey(KEY128 &value, const std::string &strvalue);
void setMAC(DEVUEI &value, const std::string &strvalue);
void setAddr(DEVADDR &value, const std::string &strvalue);

std::string key2string(const KEY128 &value);
std::string deviceEui2string(const DEVUEI &value);

void encryptPayload(
	std::string &payload,
	unsigned int frameCounter,
	unsigned char direction,
	DEVADDR &devAddr,
	KEY128 &appSKey
);
