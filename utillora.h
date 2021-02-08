#include <string>
#include <vector>
#include <inttypes.h>
#include "platform.h"
#include "aes-128.h"

#include "rapidjson/document.h"

typedef unsigned char DEVADDR[4];
typedef unsigned char DEVEUI[8];

/**
 * PUSH_DATA, PULL_DATA packets
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT section 3.2
 */
typedef ALIGN struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// random token
	uint8_t tag;				// PUSH_DATA 0x00 PULL_DATA 0x02 PUSH_DATA
	DEVEUI mac;					// 4-11	Gateway unique identifier (MAC address). For example : 00:0c:29:19:b2:37
} PACKED SEMTECH_LORA_PREFIX;	// 12 bytes
// After prefix "JSON object", starting with {, ending with }, see section 4

/**
 * PUSH_ACK packet
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT section 3.3
 */
typedef ALIGN struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// same random token as SEMTECH_LORA_PREFIX
	uint8_t tag;				// PUSH_ACK 0x01
} PACKED SEMTECH_ACK;			// 4 bytes

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

class DeviceId {
private:
public:
	DEVEUI deviceEUI;	///< device identifier
	KEY128 nwkSKey;		///< shared session key
	KEY128 appSKey;		///< private key

	DeviceId();
	DeviceId(const DeviceId &value);
};

class rfmMetaData {
public:
	time_t t;					// UTC time of pkt RX, us precision, ISO 8601 'compact' format
	uint32_t tmst;				// Internal timestamp of "RX finished" event (32b unsigned)
	uint8_t chan;				// Concentrator "IF" channel used for RX (unsigned integer)
	uint8_t rfch;				// Concentrator "RF chain" used for RX (unsigned integer)
	double freq;				// RX central frequency in MHz (unsigned float, Hz precision) 868.900000
	int8_t stat;				// CRC status: 1 = OK, -1 = fail, 0 = no CRC
	MODULATION modu;			// LORA, FSK
	std::string datr;			// LoRa datarate identifier e.g. "SF7BW125"
	uint32_t bps;				// FSK bite per second
	std::string codr;			// LoRa ECC coding rate identifier e.g. "4/6"
	int16_t rssi;				// RSSI in dBm (signed integer, 1 dB precision) e.g. -35
	float lsnr; 				// Lora SNR ratio in dB (signed float, 0.1 dB precision) e.g. 5.1
	rfmMetaData();
	rfmMetaData(const rfmMetaData &value);

	uint32_t tmms() const;			// GPS time of pkt RX, number of milliseconds since 06.Jan.1980
	std::string modulation() const;
	void setModulation(const char * value);
	std::string frequency() const;
	std::string snrratio() const;
	void toJSON(rapidjson::Value &value, rapidjson::Document::AllocatorType& allocator, const std::string &data);
	int parse(
		int &retSize,
		std::string &retData,
		rapidjson::Value &value
	);
	std::string toJsonString(const std::string &data) const;
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
	std::vector<rfmMetaData> metadata;	// at least one(from one or many BS)
	rfmHeader header;
	std::string payload;
	void clearPrefix();
	int parseData(const std::string &data);
	// load keys from the authentication service, at least deviceEUI and appSKey. Return 0- success, <0- error code
	int loadCredentialsDevAddr();
public:	
	// parse error code
	int errcode;
	// prefix contains gateway identifier
	SEMTECH_LORA_PREFIX prefix;
	// authentication keys
	DeviceId devId;

	// return array of packets from Basic communication protocol packet
	static int parse(std::vector<semtechUDPPacket> &retPackets, const void *packetForwarderPacket, int size);
	semtechUDPPacket();
	// Called from parse()
	semtechUDPPacket(const SEMTECH_LORA_PREFIX *prefix, const rfmMetaData *metadata, const std::string &data);
	// TODO I dont remember what is it for
	semtechUDPPacket(const std::string &data, const std::string &devaddr, const std::string &appskey);
	
	std::string serialize2RfmPacket();
	std::string toString();
	std::string metadataToJsonString();

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
	void ack(SEMTECH_ACK *retval);	// 4 bytes
};
