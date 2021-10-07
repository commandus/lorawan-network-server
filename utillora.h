#ifndef UTIL_LORA_H_
#define UTIL_LORA_H_	1

#include <string>
#include <vector>
#include <map>
#include <inttypes.h>

#include <netinet/in.h>

#include "platform.h"
#include "lora-radio.h"
#include "system/crypto/aes.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

#include "gateway-stat.h"

typedef unsigned char NETID[3];

typedef unsigned char KEY128[16];
typedef unsigned char DEVADDR[4];
typedef unsigned char DEVEUI[8];

typedef unsigned char JOIN_NONCE[3];

typedef uint8_t FREQUENCY[3];
typedef uint8_t JOINNONCE[3];

class IdentityService;

typedef char DEVICENAME[8];

enum ERR_CODE_TX {
	JIT_ERROR_OK,           	// Packet ok to be sent
	JIT_ERROR_TOO_LATE,     	// Too late to send this packet
	JIT_ERROR_TOO_EARLY,    	// Too early to queue this packet
	JIT_ERROR_FULL,         	// Downlink queue is full
	JIT_ERROR_EMPTY,        	// Downlink queue is empty
	JIT_ERROR_COLLISION_PACKET, // A packet is already enqueued for this timeframe
	JIT_ERROR_COLLISION_BEACON, // A beacon is planned for this timeframe
	JIT_ERROR_TX_FREQ,      	// The required frequency for downlink is not supported
	JIT_ERROR_TX_POWER,     	// The required power for downlink is not supported
	JIT_ERROR_GPS_UNLOCKED, 	// GPS timestamp could not be used as GPS is unlocked
	JIT_ERROR_INVALID       	// Packet is invalid
};

void string2DEVADDR(DEVADDR &retval, const std::string &str);
void string2DEVEUI(DEVEUI &retval, const std::string &str);
void string2KEY(KEY128 &retval, const std::string &str);
void string2DEVICENAME(DEVICENAME &retval, const char *str);
std::string DEVICENAME2string(const DEVICENAME &value);

uint64_t str2gatewayId(const char *value);
std::string gatewayId2str(uint64_t value);

class DEVADDRINT
{
	public:
		uint32_t a;
		DEVADDRINT() 
			: a(0)
		{

		}

		DEVADDRINT(const DEVADDR &v) {
			memmove(&a, &v, sizeof(DEVADDR));
		}
};

struct DEVADDRCompare
{
	bool operator() (const DEVADDR& lhs, const DEVADDR& rhs) const {
		return lhs < rhs;
	}
};

struct DEVADDRINTCompare
{
	bool operator() (const DEVADDRINT& lhs, const DEVADDRINT& rhs) const {
		return lhs.a < rhs.a;
	}
};

class TDEVEUI
{
	public:
		DEVEUI eui;
		TDEVEUI() {
		}
		
		TDEVEUI(const std::string &value) {
			string2DEVEUI(eui, value);
		}

		TDEVEUI(const TDEVEUI &v) {
			memmove(&eui, &v.eui, sizeof(DEVEUI));
		}
		TDEVEUI(const DEVEUI &v) {
			memmove(&eui, &v, sizeof(DEVEUI));
		}
		std::string toString() const;
};

struct DEVEUICompare
{
	bool operator() (const DEVEUI& lhs, const DEVEUI& rhs) const {
		return strncmp((const char *) lhs, (const char *) rhs, sizeof(DEVEUI));
	}
};

/**
 * @return ind3ex of the rxpk object 
 */ 
int getMetadataName(
	const char *name
);

// network server receives SEMTECH_GW_PUSH_DATA, SEMTECH_GW_PULL_DATA, SEMTECH_GW_TX_ACK
// gateway forward the RF packets received, and associated metadata, to the network server
#define SEMTECH_GW_PUSH_DATA	0
// network server responds on PUSH_DATA to acknowledge immediately all the PUSH_DATA packets received
#define SEMTECH_GW_PUSH_ACK		1
// gateway initiate receiving packates from the metwork server (because of NAT)
#define SEMTECH_GW_PULL_DATA	2
// network server responds on PULL_DATA
#define SEMTECH_GW_PULL_ACK		3
// network server send packet to the gateway afrer PULL_DATA - PULL_ACK sequence
#define SEMTECH_GW_PULL_RESP	4
// gateway inform network server about does PULL_RESP data transmission was successful or not
#define SEMTECH_GW_TX_ACK		5

typedef ALIGN struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// random token
	uint8_t tag;				// PUSH_DATA 0x00 PULL_DATA 0x02 PUSH_DATA
} PACKED SEMTECH_PREFIX;		// 4 bytes

/**
 * Semtech PUSH DATA packet described in section 3.2
 * Semtech PULL DATA packet described in section 5.2
 * PUSH_DATA, PULL_DATA packets prefix.
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT sections 3.2, 5,2
 */
typedef ALIGN struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// random token
	uint8_t tag;				// PUSH_DATA 0x00 PULL_DATA 0x02 PUSH_DATA
	DEVEUI mac;					// 4-11	Gateway unique identifier (MAC address). For example : 00:0c:29:19:b2:37
} PACKED SEMTECH_PREFIX_GW;	// 12 bytes
// After prefix "JSON object", starting with {, ending with }, see section 4

// After prefix "JSON object", starting with {, ending with }, see section 4

/**
 * PUSH_ACK packet
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT section 3.3
 */
typedef ALIGN struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// same random token as SEMTECH_PREFIX_GW
	uint8_t tag;				// PUSH_ACK 1 PULL_ACK 4
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
 * 11100000 E0   ProprietaryRADIO
*/

typedef enum {
	MTYPE_JOIN_REQUEST = 0,
 	MTYPE_JOIN_ACCEPT = 1,
 	MTYPE_UNCONFIRMED_DATA_UP = 2,
 	MTYPE_UNCONFIRMED_DATA_DOWN = 3,
 	MTYPE_CONFIRMED_DATA_UP = 4,
 	MTYPE_CONFIRMED_DATA_DOWN = 5,
 	MTYPE_REJOIN_REQUEST = 6,
 	MTYPE_PROPRIETARYRADIO = 7
} MTYPE;

// Join-request types.
typedef enum {
	JOINREQUEST = 0xff,
	REJOINREQUEST0 = 0,
	REJOINREQUEST1 = 1,
	REJOINREQUEST2 = 2
 } JOINREQUESTTYPE;

typedef ALIGN struct {
	union {
		uint8_t i;
		struct {
			uint8_t major: 2;
			uint8_t rfu: 3;
			uint8_t mtype: 3;
		} f;
	};
} PACKED MHDR;			// 1 byte

/**
 * MHDR + FHDR
 */ 
typedef ALIGN struct {
	// MAC heaader byte: message type, RFU, Major
	MHDR macheader;			// 0x40 unconfirmed uplink
	// Frame header (FHDR)
	DEVADDR devaddr;			// MAC address
	union {
		uint8_t i;
		// downlink
		struct {
			uint8_t foptslen: 4;
			uint8_t fpending: 1;
			uint8_t ack: 1;
			uint8_t rfu: 1;
			uint8_t adr: 1;
		} f;
		// uplink
		struct {
			uint8_t foptslen: 4;
			uint8_t classb: 1;
			uint8_t ack: 1;
			uint8_t addrackreq: 1;
			uint8_t adr: 1;
		} fup;
	} fctrl;	// frame control
	uint16_t fcnt;	// frame counter 0..65535
	// FOpts 0..15
} PACKED RFM_HEADER;			// 8 bytes, +1

// Channel frequency list
typedef ALIGN struct {
	FREQUENCY frequency[5];	// frequency, 100 * Hz ch 4..8
	uint8_t cflisttype;		// always 0
} PACKED CFLIST;			// 16 bytes

typedef ALIGN struct {
	uint8_t fopts[15];
} PACKED FOPTS;					// 0..15 bytes

typedef enum {
	ABP = 0,
	OTAA = 1
} ACTIVATION;

typedef enum {
	CLASS_A = 0,
	CLASS_B = 1,
	CLASS_C = 2
} DEVICECLASS;

typedef ALIGN struct {
	uint8_t major: 2;		// always 1
	uint8_t minor: 2;		// 0 or 1
	uint8_t release: 4;		// no matter
} PACKED LORAWAN_VERSION;	// 1 byte

std::string LORAWAN_VERSION2string
(
	LORAWAN_VERSION value
);

LORAWAN_VERSION string2LORAWAN_VERSION
(
	const std::string &value
);

typedef struct {
	// value, no key
	ACTIVATION activation;	///< activation type: ABP or OTAA
	DEVICECLASS deviceclass;
	DEVEUI deviceEUI;		///< device identifier 8 bytes (ABP device may not store EUI)
	KEY128 nwkSKey;			///< shared session key 16 bytes
	KEY128 appSKey;			///< private key 16 bytes
	LORAWAN_VERSION version;
	// added for searching
	DEVICENAME name;
} DEVICEID;					// 44 bytes + 8 = 52

/**
 * Section 6.3 Activating an end-device by personalization 
 * - DevAddr ->
 * - FNwkSIntKey
 * - SNwkSIntKey shared session key ->
 * - NwkSEncKey
 * - AppSKey private key ->
 * are directly stored into the end-device
*/
class NetworkIdentity {
private:
public:
	// key
	DEVADDR devaddr;		///< network address
	// value
	ACTIVATION activation;	///< activation type: ABP or OTAA
	DEVICECLASS deviceclass;
	DEVEUI deviceEUI;		///< device identifier
	KEY128 nwkSKey;			///< shared session key
	KEY128 appSKey;			///< private key
	LORAWAN_VERSION version;
	// added for searching
	DEVICENAME name;
	NetworkIdentity();
	NetworkIdentity(const DEVADDRINT &a, const DEVICEID &id);
	void set(
		const DEVADDRINT &addr,
		const DEVICEID &value
	);
	std::string toString() const;
};

class DeviceId {
private:
public:
	ACTIVATION activation;	///< activation type: ABP or OTAA
	DEVICECLASS deviceclass;
	DEVEUI deviceEUI;			///< device identifier
	KEY128 nwkSKey;				///< shared session key
	KEY128 appSKey;				///< private key
	LORAWAN_VERSION version;	///< device LoraWAN version
	// added for searching
	DEVICENAME name;
	
	DeviceId();
	DeviceId(const DeviceId &value);
	DeviceId(const DEVICEID &value);
	void set(const DEVICEID &value);
	void setEUIString(const std::string &value);
	void setNwkSKeyString(const std::string &value);
	void setAppSKeyString(const std::string &value);
	void setName(const std::string &value);
	void setClass(const DEVICECLASS &value);

	std::string toJsonString() const;
	void setProperties(std::map<std::string, std::string> &retval);
};

class rfmMetaData {
public:
	uint64_t gatewayId;
	time_t t;					// UTC time of pkt RX, us precision, ISO 8601 'compact' format
	uint32_t tmst;				// Internal timestamp of "RX finished" event (32b unsigned)
	uint8_t chan;				// Concentrator "IF" channel used for RX (unsigned integer)
	uint8_t rfch;				// Concentrator "RF chain" used for RX (unsigned integer)
	uint32_t freq;				// RX central frequency in Hz, not Mhz. MHz (unsigned float, Hz precision) 868.900000
	int8_t stat;				// CRC status: 1 = OK, -1 = fail, 0 = no CRC
	MODULATION modu;			// LORA, FSK
	BANDWIDTH bandwith;
	SPREADING_FACTOR spreadingFactor;
 	CODING_RATE codingRate;

	uint32_t bps;				// FSK bite per second
	int16_t rssi;				// RSSI in dBm (signed integer, 1 dB precision) e.g. -35
	float lsnr; 				// Lora SNR ratio in dB (signed float, 0.1 dB precision) e.g. 5.1
	rfmMetaData();
	// copy gateway address from value
	rfmMetaData(const rfmMetaData &value);
	// copy gateway address from prefix
	rfmMetaData(const SEMTECH_PREFIX_GW *aprefix, const rfmMetaData &value);

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
	std::string toJsonString(const std::string &payload) const;
	std::string datr() const;			// LoRa datarate identifier e.g. "SF7BW125"
	void setDatr(const std::string &value);			// LoRa datarate identifier e.g. "SF7BW125"
	std::string codr() const;			// LoRa ECC coding rate identifier e.g. "4/6"
	void setCodr(const std::string &value);			// LoRa datarate identifier e.g. "SF7BW125"
};

class rfmHeader {
public:	
	RFM_HEADER header;
	FOPTS fopts;
	uint8_t fport;

	rfmHeader();
	rfmHeader(
		const RFM_HEADER &headerRADIO
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

	std::string toBinary() const;
	std::string toJson() const;
	bool parse(const std::string &value);
};

class semtechUDPPacket {
private:
	void clearPrefix();
	int parseData(const std::string &data, IdentityService *identityService);
	std::string toTxImmediatelyJsonString(
		const std::string &payload,
		uint32_t recievedTime,
		const int power = 14
	) const;
protected:
public:
	std::string payload;
	rfmHeader header;
	std::vector<rfmMetaData> metadata;	// at least one(from one or many BS)
	struct sockaddr_in6 gatewayAddress;
	// parse error code
	int errcode;
	// downlink direction 01, uplink direction 00
	bool downlink;
	// prefix contains gateway identifier
	SEMTECH_PREFIX_GW prefix;
	// authentication keys
	DeviceId devId;

	/**
	 * Parse Semtech UDP packet gateway prefix
	 * @return 0, ERR_CODE_PACKET_TOO_SHORT, ERR_CODE_INVALID_PROTOCOL_VERSION
	 */ 
	static int parsePrefixGw
	(
		SEMTECH_PREFIX_GW &retprefix,
		const void *packetForwarderPacket,
		int size
	);

	// return array of packets from Basic communication protocol packet
	static int parse(
		const struct sockaddr *gatewayAddress,
		SEMTECH_PREFIX_GW &retprefix,
		GatewayStat &retgwstat,
		std::vector<semtechUDPPacket> &retPackets, 
		const void *packetForwarderPacket, 
		int size,
		IdentityService *identityService
	);
	semtechUDPPacket();
	// Called from parse()
	semtechUDPPacket(const struct sockaddr *gatewayAddress, const SEMTECH_PREFIX_GW *prefix, const rfmMetaData *metadata, const std::string &data, IdentityService *identityService);
	// TODO I dont remember what is it for
	semtechUDPPacket(const struct sockaddr *gatewayAddress, const std::string &data, const std::string &devaddr, const std::string &appskey);
	
	std::string serialize2RfmPacket() const;
	std::string toString() const;
	std::string toDebugString() const;
	std::string metadataToJsonString() const;
	std::string toJsonString() const;

	const RFM_HEADER *getRfmHeader() const;
	rfmHeader *getHeader();
	void setRfmHeader(const RFM_HEADER &value);

	std::string getDeviceEUI() const;
	void setDeviceEUI(const std::string &value);

	float getSignalLevel() const;
	std::string getDeviceAddrStr() const;
	DEVADDRINT getDeviceAddr() const;
	void getDeviceAddr(DEVADDR &retval) const;
	void setDeviceAddr(const std::string &value);
	void setGatewayId(const std::string &value);
	void setNetworkSessionKey(const std::string &value);
	void setApplicationSessionKey(const std::string &value);
	void setFrameCounter(uint16_t value);

	void setPayload(uint8_t port, const std::string &payload);
	// Create ACK response to be send to the BS 
	void ack(SEMTECH_ACK *retval);	// 4 bytes
	int16_t getStrongesSignalLevel(int &idx) const;

	static char *getSemtechJSONCharPtr(const void *packet, size_t size);

	// @return true- has MAC payload
	bool hasMACPayload() const ;
	bool hasApplicationPayload() const;
	/**
	 * Return gateway MAC address as int with best SNR
	 * @param retvalLsnr if provided, return SNR
	 * @return 0 if not found
	 */
	uint64_t getBestGatewayAddress(float *retvalLsnr = NULL) const;
	
	/**
	 * Make PULL_RESP Semtech UDP protocol packet repsonse
	 * @param data payload
	 * @param version LoraWAN version
	 * @param key key
	 * @param power transmission power
	 */ 
	std::string mkPullResponse(
		const std::string &payload,
		const DeviceId &deviceid,
		uint32_t recievedTime,
		const int power = 14
	) const;

	uint32_t tmst();	// return received time in microsends
	uint32_t tmms();	// return received GPS time, can be 0

};

uint64_t deveui2int(const DEVEUI &value);
void int2DEVADDR(DEVADDR &retval, uint32_t value);

std::string DEVADDR2string(const DEVADDR &value);
std::string uint64_t2string(const uint64_t &value);
std::string DEVADDRINT2string(const DEVADDRINT &value);
std::string DEVEUI2string(const DEVEUI &value);
std::string KEY2string(const KEY128 &value);
uint32_t NETID2int(const NETID &value);
uint32_t JOINNONCE2int(const JOINNONCE &value);
int FREQUENCY2int(const FREQUENCY &frequency);

// Debug onlyRADIO

void decryptPayload(
	std::string &payload,
	unsigned int frameCounter,
	unsigned char direction,
	DEVADDR &devAddr,
	KEY128 &appSKey
);

uint32_t calculateMIC(
	const unsigned char *data,
	const unsigned char size,
	const unsigned int frameCounter,
	const unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &key
);

uint32_t getMic(const std::string &v);

std::string deviceclass2string(
	DEVICECLASS value
);

DEVICECLASS string2deviceclass
(
	const std::string &value
);

std::string mtype2string(
	MTYPE value
);

std::string opts2string
(
	uint8_t len,
	const FOPTS &value
);

MTYPE string2mtype(
	const std::string &value
);

void string2NETID(NETID &retval, const char *str);
void string2FREQUENCY(FREQUENCY &retval, const char *value);
void string2JOINNONCE(JOINNONCE &retval, const char *value);

std::string activation2string(ACTIVATION value);
ACTIVATION string2activation(const std::string &value);

std::string semtechDataPrefix2JsonString(
	const SEMTECH_PREFIX_GW &prefix
);

/**
 * link margin, dB, range of 0..254
 * “0” - the frame was received at the demodulation floor
 * “20” - frame reached the gateway 20 dB above demodulation floor
 * @param spreadingFactor 6,.12
 * @param loraSNR
 * @return 0..254
 */
uint8_t loraMargin(
	uint8_t spreadingFactor,
	float loraSNR
);

/**
 * Check is packet is transmission ACK packet, if it does, look for error code and return it if it found. If it is not, 
 * ACK indicates that tramsmission was successful.
 * @param buffer Semtech's gateway TX ACK packet buffer
 * @param size recieved bytes (packet size)
 */
ERR_CODE_TX extractTXAckCode(
	const void *buffer,
	size_t sz
);

const char *getTXAckCodeName
(
	ERR_CODE_TX code
);

#endif
