#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "platform.h"
#include "utillora.h"
#include "utildate.h"
#include "utilstring.h"
#include "base64/base64.h"

#include "errlist.h"

#if BYTE_ORDER == BIG_ENDIAN
#define ntoh16(x) (x)
#else

// @see https://stackoverflow.com/questions/2182002/convert-big-endian-to-little-endian-in-c-without-using-provided-func
void swapBytes(void *pv, size_t n)
{
    char *p = (char *) pv;
    size_t lo, hi;
    for (lo = 0, hi = n - 1; hi > lo; lo++, hi--) {
        char tmp = p[lo];
        p[lo] = p[hi];
        p[hi] = tmp;
    }
}
#define ntohVar(x) swapBytes(&x, sizeof(x));
#define ntoh16(x) swapBytes(x, 16);
#endif

/**
 * @see https://os.mbed.com/teams/Semtech/code/LoRaWAN-lib//file/2426a05fe29e/LoRaMacCrypto.cpp/
 */
void encryptPayload(
	std::string &payload,
	const unsigned int frameCounter,
	const unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &appSKey
)
{
	uint8_t blockA[16];

    uint16_t i;
    uint16_t ctr = 1;
 
 	blockA[0] = 1;
	blockA[1] = 0;
	blockA[2] = 0;
	blockA[3] = 0;
	blockA[4] = 0;
    blockA[5] = direction;
    blockA[6] = devAddr[3];
	blockA[7] = devAddr[2];
	blockA[8] = devAddr[1];
	blockA[9] = devAddr[0];
	blockA[10] = (frameCounter & 0x00FF);
	blockA[11] = ((frameCounter >> 8) & 0x00FF);
	blockA[12] = 0; // frame counter upper Bytes
	blockA[13] = 0;
	blockA[14] = 0;

	unsigned char size = payload.size();

	std::string encBuffer(payload);
    uint8_t bufferIndex = 0;

    while (size >= 16) {
        blockA[15] = ( ( ctr ) & 0xff );
        ctr++;
 		// calculate S
		aesEncrypt(blockA, appSKey);
        for (i = 0; i < 16; i++) {
            encBuffer[bufferIndex + i] = payload[bufferIndex + i] ^ blockA[i];
        }
        size -= 16;
        bufferIndex += 16;
    }
 	if (size > 0) {
        blockA[15] = ( ( ctr ) & 0xff );
        aesEncrypt(blockA, appSKey);
        for (i = 0; i < size; i++) {
            encBuffer[bufferIndex + i] = payload[bufferIndex + i] ^ blockA[i];
        }
    }
	payload = encBuffer;
}

/*
static void encryptPayload2(
	std::string &payload,
	unsigned int frameCounter,
	unsigned char direction,
	DEVADDR &devAddr,
	KEY128 &appSKey
)
{
	unsigned char *data = (unsigned char *) payload.c_str();
	unsigned char size = payload.size();
	unsigned char blockA[16];

	// calc number of blocks
	unsigned char numberOfBlocks = size / 16;
	unsigned char incompleteBlockSize = size % 16;
	if (incompleteBlockSize != 0)
		numberOfBlocks++;

	for (int i = 1; i <= numberOfBlocks; i++) {
		blockA[0] = 0x01;
		blockA[1] = 0x00;
		blockA[2] = 0x00;
		blockA[3] = 0x00;
		blockA[4] = 0x00;

		blockA[5] = direction;

		blockA[6] = devAddr[3];
		blockA[7] = devAddr[2];
		blockA[8] = devAddr[1];
		blockA[9] = devAddr[0];

		blockA[10] = (frameCounter & 0x00FF);
		blockA[11] = ((frameCounter >> 8) & 0x00FF);

		blockA[12] = 0x00; // frame counter upper Bytes
		blockA[13] = 0x00;

		blockA[14] = 0x00;

		blockA[15] = i;

		// calculate S
		aesEncrypt(blockA, appSKey);

		// check for last block
		if (i != numberOfBlocks) {
			for (int j = 0; j < 16; j++) {
				*data = *data ^ blockA[j];
				data++;
			}
		} else {
			if (incompleteBlockSize == 0)
				incompleteBlockSize = 16;

			for (int j = 0; j < incompleteBlockSize; j++) {
				*data = *data ^ blockA[j];
				data++;
			}
		}
	}
}
*/

static void decryptPayload(
	std::string &payload,
	unsigned int frameCounter,
	unsigned char direction,
	DEVADDR &devAddr,
	KEY128 &appSKey
)
{
	encryptPayload(payload, frameCounter, direction, devAddr,appSKey);
}

/**
 * 4.3.3 MAC Frame Payload Encryption (FRMPayload)
 * message integrity code
 * B0
 * 1    4       1   4       4(3+1)       1 1
 * 0x49 0 0 0 0 Dir DevAddr frameCounter 0 Len(msg)
 * cmac = aes128_cmac(NwkSKey, B0 | msg)
 * MIC = cmac[0..3]
 * 
 * 401111111100000001a1a46ff045b570
 *   DEV-ADDR  FRAMCN  PAYLOA
 * MH        FC(1.0) FP      MIC
 *           FRAME-CN (1.1)
 * 
 * MH MAC header (40)
 * FC Frame control
 * CN Frame counter
 * FP Frame port
 */ 
static uint32_t calculateMIC(
	const std::string &payload,
	const unsigned int frameCounter,
	const unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &key
)
{
	unsigned char *data = (unsigned char *) payload.c_str();
	unsigned char dataLength = payload.size();
	unsigned char blockB[16];

	unsigned char keyK1[16] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char keyK2[16] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	unsigned char oldData[16] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char newData[16] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	unsigned char blockCounter = 0x01;

	// blockB
	blockB[0] = 0x49;
	blockB[1] = 0x00;
	blockB[2] = 0x00;
	blockB[3] = 0x00;
	blockB[4] = 0x00;

	blockB[5] = direction;

	blockB[6] = devAddr[3];
	blockB[7] = devAddr[2];
	blockB[8] = devAddr[1];
	blockB[9] = devAddr[0];

	blockB[10] = (frameCounter & 0x00FF);
	blockB[11] = ((frameCounter >> 8) & 0x00FF);

	blockB[12] = 0x00; // Frame counter upper bytes
	blockB[13] = 0x00;

	blockB[14] = 0x00;
	blockB[15] = dataLength;

	// calc number of blocks and blocksize of last block
	unsigned char numberOfBlocks = dataLength / 16;
	unsigned char incompleteBlockSize = dataLength % 16;

	if (incompleteBlockSize != 0)
		numberOfBlocks++;

	generateKeys(keyK1, keyK2, key);

	// calc on Block B0

	// AES encryption
	aesEncrypt(blockB, key);

	// copy blockB to oldData
	for (int i = 0; i < 16; i++)
	{
		oldData[i] = blockB[i];
	}

	// full calculating until n-1 messsage blocks
	while (blockCounter < numberOfBlocks) {
		// Copy data into array
		for (int i = 0; i < 16; i++) {
			newData[i] = *data;
			data++;
		}

		// XOR with old data
		XOR(newData, oldData);
		//  AES encryption
		aesEncrypt(newData, key);
		// copy newData to oldData
		for (int i = 0; i < 16; i++) {
			oldData[i] = newData[i];
		}
		// raise Block counter
		blockCounter++;
	}

	// Perform calculation on last block
	// Check if Datalength is a multiple of 16
	if (incompleteBlockSize == 0) {
		// copy last data into array
		for (int i = 0; i < 16; i++) {
			newData[i] = *data;
			data++;
		}

		// XOR with Key 1
		XOR(newData, keyK1);

		// XOR with old data
		XOR(newData, oldData);

		// last AES routine
		aesEncrypt(newData, key);
	} else {
		// copy the remaining data and fill the rest
		for (int i = 0; i < 16; i++) {
			if (i < incompleteBlockSize) {
				newData[i] = *data;
				data++;
			}
			if (i == incompleteBlockSize)
				newData[i] = 0x80;
			if (i > incompleteBlockSize)
				newData[i] = 0x00;
		}
		// XOR with Key 2
		XOR(newData, keyK2);
		// XOR with Old data
		XOR(newData, oldData);
		// last AES routine
		aesEncrypt(newData, key);
	}
	uint32_t r;
	memcpy(&r, &newData, 4);
	return r;
}

NetworkIdentity::NetworkIdentity(
	const DEVADDRINT &a,
	const DEVICEID &value
) 
{
	memmove(&devaddr, &a.a, sizeof(DEVADDR));
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceEUI, &value.deviceEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
}

std::string KEY2string(
	const KEY128 &value
)
{
	KEY128 v;
	memmove(&v, &value, sizeof(v));
	ntoh16(&v);
	return hexString(&v, sizeof(v));
}

std::string NetworkIdentity::toString() const
{
	std::stringstream ss;
	ss << DEVADDR2string(devaddr) 
		<< " " << (activation == 1 ? "OTAA" : "ABP")
		<< " " << DEVEUI2string(deviceEUI)
		<< " " << KEY2string(nwkSKey)
		<< " " << KEY2string(appSKey);
	return ss.str();
}

DeviceId::DeviceId() {
	memset(&deviceEUI, 0, sizeof(DEVEUI));
	memset(&nwkSKey, 0, sizeof(KEY128));
	memset(&appSKey, 0, sizeof(KEY128));
}

DeviceId::DeviceId(
	const DeviceId &value
) {
	memmove(&deviceEUI, &value.deviceEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
}

DeviceId::DeviceId(
	const DEVICEID &value
)
{
	set(value);
}

void DeviceId::set(
	const DEVICEID &value
)
{
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceEUI, &value.deviceEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
}

const std::string DEF_DATA_RATE = "SF7BW125";
const std::string DEF_ECCCODE_RATE = "4/6";
#define DEF_RSSI	-35
#define DEF_LSNR	5.1

rfmMetaData::rfmMetaData() 
	: chan(0), rfch(0), freq(868900000), stat(0), modu(LORA), datr(DEF_DATA_RATE),
	bps(0), codr(DEF_ECCCODE_RATE), rssi(DEF_RSSI), lsnr(DEF_LSNR)
{
	time(&t);			// UTC time of pkt RX, us precision, ISO 8601 'compact' format
}

rfmMetaData::rfmMetaData(
	const rfmMetaData &value
)
	: t(value.t), chan(value.chan), rfch(value.rfch), freq(value.freq), stat(value.stat), modu(value.modu), datr(value.datr),
		bps(value.bps), codr(value.codr), rssi(value.rssi), lsnr(value.lsnr)
{
}

/**
 * GPS time of pkt RX, number of milliseconds since 06.Jan.1980
 */ 
uint32_t rfmMetaData::tmms() const
{
	return utc2gps(t);
}

std::string rfmMetaData::modulation() const
{
	switch (modu)
	{
	case FSK:
		return "FSK";
	default:
		return "LORA";
	}
}

void rfmMetaData::setModulation(
	const char *value
) {
	if (strcmp(value, "FSK") == 0)
		modu = FSK;
	else
		modu = LORA;
}

std::string rfmMetaData::frequency() const
{
	std::stringstream ss;
	int mhz = freq / 1000000; 
	ss << mhz << "." << freq - (mhz * 1000000);
	return ss.str();
}

std::string rfmMetaData::snrratio() const
{
	std::stringstream ss;
	int n = lsnr; 
	int m = lsnr * 10.0 - n * 10;
	ss << n << "." << m;
	return ss.str();
}

/**
 * 	Section 3.3	 
 */
const char* METADATA_NAMES[15] = {
	"rxpk",	// 0 array name
	"time", // 1 string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
	"tmms", // 2 number | GPS time of pkt RX, number of milliseconds since 06.Jan.1980
	"tmst", // 3 number | Internal timestamp of "RX finished" event (32b unsigned)
	"freq", // 4 number | RX central frequency in MHz (unsigned float, Hz precision)
	"chan", // 5 number | Concentrator "IF" channel used for RX (unsigned integer)
	"rfch", // 6 number | Concentrator "RF chain" used for RX (unsigned integer)
	"stat", // 7 number | CRC status: 1 = OK, -1 = fail, 0 = no CRC
	"modu", // 8 string | Modulation identifier "LORA" or "FSK"
	"datr", // 9 string or number | LoRa datarate identifier (eg. SF12BW500) or FSK datarate (unsigned, in bits per second)
	"codr", // 10 string | LoRa ECC coding rate identifier
	"rssi", // 11 number | RSSI in dBm (signed integer, 1 dB precision)
	"lsnr", // 12 number | Lora SNR ratio in dB (signed float, 0.1 dB precision)
	"size", // 13 number | RF packet payload size in bytes (unsigned integer)
	"data"  // 14 string | Base64 encoded RF packet payload, padded
};

int getMetadataName(
	const char *name
)
{
	int r = -1;
	for (int i = 0; i < 15; i++) {
		if (strcmp(METADATA_NAMES[i], name) == 0)
			return i;
	}
	return r;
}

void string2DEVADDR(
	DEVADDR &retval,
	const std::string &value
)
{
	std::string str = hex2string(value);
	int len = str.size();
	if (len > sizeof(DEVADDR))
		len = sizeof(DEVADDR);
	memmove(&retval, str.c_str(), len);
	if (len < sizeof(DEVADDR))
		memset(&retval + len, 0, sizeof(DEVADDR) - len);
	*((uint32_t*) &retval) = ntoh4(*((uint32_t*) &retval));
}

void string2DEVEUI(
	DEVEUI &retval,
	const std::string &value
)
{
	std::string str = hex2string(value);
	int len = str.size();
	if (len > sizeof(DEVEUI))
		len = sizeof(DEVEUI);
	memmove(&retval, str.c_str(), len);
	if (len < sizeof(DEVEUI))
		memset(&retval + len, 0, sizeof(DEVEUI) - len);
	*((uint64_t*) &retval) = ntoh8(*((uint64_t*) &retval));
}

void string2KEY(
	KEY128 &retval,
	const std::string &str
)
{
	int len = str.size();
	if (len > sizeof(KEY128))
		len = sizeof(KEY128);
	memmove(&retval, str.c_str(), len);
	if (len < sizeof(KEY128))
		memset(&retval + len, 0, sizeof(KEY128) - len);
	ntoh16(&retval);
}

std::string DEVADDR2string(
	const DEVADDR &value
)
{
	uint32_t v;
	memmove(&v, &value, sizeof(v));
	v = ntoh4(v);
	return hexString(&v, sizeof(v));
}

std::string DEVADDRINT2string(
	const DEVADDRINT &value
)
{
	DEVADDRINT v;
	memmove(&v.a, &value.a, sizeof(DEVADDR));
	v.a = ntoh4(*((uint32_t*) &v.a));
	return hexString(&v, sizeof(DEVADDR));
}

std::string DEVEUI2string(
	const DEVEUI &value
)
{
	uint64_t v;
	memmove(&v, &value, sizeof(DEVEUI));
	v = ntoh8(v);
	return hexString(&v, sizeof(v));
}

void rfmMetaData::toJSON(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator,
	const std::string &data
) {
	int ms;
	std::string dt = ltimeString(t, ms, "%FT%T") + "Z";	// "2020-12-16T12:17:00.12345Z";
	rapidjson::Value v1(rapidjson::kStringType);
	v1.SetString(dt.c_str(), dt.length());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[1])), v1, allocator);

	rapidjson::Value v2(tmms());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[2])), v2, allocator);

	rapidjson::Value v3(tmst);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[3])), v3, allocator);

	rapidjson::Value v4(freq);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[4])), v4, allocator);

	rapidjson::Value v5(chan);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[5])), v5, allocator);

	rapidjson::Value v6(rfch);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[6])), v6, allocator);

	rapidjson::Value v7(stat);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[7])), v7, allocator);

	rapidjson::Value v8(rapidjson::kStringType);
	std::string s8(modulation());
	v8.SetString(s8.c_str(), s8.length());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[8])), v8, allocator);

	rapidjson::Value v9(rapidjson::kStringType);
	v9.SetString(datr.c_str(), datr.length());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[9])), v9, allocator);

	rapidjson::Value v10(rapidjson::kStringType);
	v10.SetString(codr.c_str(), codr.length());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[10])), v10, allocator);

	rapidjson::Value v11(rssi);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[11])), v11, allocator);

	rapidjson::Value v12(lsnr);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[12])), v12, allocator);

	rapidjson::Value v13(data.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[13])), v13, allocator);

	rapidjson::Value v14(rapidjson::kStringType);
	std::string d(base64_encode(data));	// base64
	v14.SetString(d.c_str(), d.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_NAMES[14])), v14, allocator);
}

int rfmMetaData::parse(
	int &retSize,
	std::string &retData,
	rapidjson::Value &value
) {
	if (value.HasMember(METADATA_NAMES[1])) {
		rapidjson::Value &v = value[METADATA_NAMES[1]];
		if (v.IsString()) {
			t = parseDate(v.GetString());
		}
	}

	if (value.HasMember(METADATA_NAMES[3])) {
		rapidjson::Value &v = value[METADATA_NAMES[3]];
		if (v.IsInt()) {
			tmst = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_NAMES[4])) {
		rapidjson::Value &v = value[METADATA_NAMES[4]];
		if (v.IsDouble()) {
			freq = v.GetDouble();
		}
	}

	if (value.HasMember(METADATA_NAMES[5])) {
		rapidjson::Value &v = value[METADATA_NAMES[5]];
		if (v.IsInt()) {
			chan = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_NAMES[6])) {
		rapidjson::Value &v = value[METADATA_NAMES[6]];
		if (v.IsInt()) {
			rfch = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_NAMES[7])) {
		rapidjson::Value &v = value[METADATA_NAMES[7]];
		if (v.IsInt()) {
			stat = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_NAMES[8])) {
		rapidjson::Value &v = value[METADATA_NAMES[8]];
		if (v.IsString()) {
			setModulation(v.GetString());
		}
	}

	if (value.HasMember(METADATA_NAMES[9])) {
		rapidjson::Value &v = value[METADATA_NAMES[9]];
		if (v.IsString()) {
			datr = v.GetString();
		}
	}
	
	if (value.HasMember(METADATA_NAMES[10])) {
		rapidjson::Value &v = value[METADATA_NAMES[10]];
		if (v.IsString()) {
			codr = v.GetString();
		}
	}

	if (value.HasMember(METADATA_NAMES[11])) {
		rapidjson::Value &v = value[METADATA_NAMES[11]];
		if (v.IsInt()) {
			rssi = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_NAMES[12])) {
		rapidjson::Value &v = value[METADATA_NAMES[12]];
		if (v.IsInt()) {
			lsnr = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_NAMES[13])) {
		rapidjson::Value &v = value[METADATA_NAMES[13]];
		if (v.IsInt()) {
			retSize = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_NAMES[14])) {
		rapidjson::Value &v = value[METADATA_NAMES[14]];
		if (v.IsString()) {
			retData = base64_decode(v.GetString());
		}
	}
	return 0;
}

std::string rfmMetaData::toJsonString(
	const std::string &data
) const
{
	std::stringstream ss;
	int ms;
	std::string dt = ltimeString(t, ms, "%FT%T") + "Z";	// "2020-12-16T12:17:00.12345Z";
	ss << "{" 
		<< "\"" << METADATA_NAMES[1] << "\":\"" << dt << "\""
		<< "\"" << METADATA_NAMES[2] << "\":" << tmms()
		<< "\"" << METADATA_NAMES[3] << "\":" << tmst
		<< "\"" << METADATA_NAMES[4] << "\":" << frequency()
		<< "\"" << METADATA_NAMES[5] << "\":" << chan
		<< "\"" << METADATA_NAMES[6] << "\":" << rfch
		<< "\"" << METADATA_NAMES[7] << "\":" << stat
		<< "\"" << METADATA_NAMES[8] << "\":\"" << modulation() << "\""
		<< "\"" << METADATA_NAMES[9] << "\":\"" << datr << "\""
		<< "\"" << METADATA_NAMES[10] << "\":\"" << codr << "\""
		<< "\"" << METADATA_NAMES[11] << "\":" << rssi
		<< "\"" << METADATA_NAMES[12] << "\":" << lsnr
		<< "\"" << METADATA_NAMES[13] << "\":" << data.size()
		<< "\"" << METADATA_NAMES[14] << "\":\"" << base64_encode(data) << "\"}";
	return ss.str();
}

rfmHeader::rfmHeader() {
	memset(&header, 0, sizeof(RFM_HEADER));
}

rfmHeader::rfmHeader(
	const RFM_HEADER &hdr
) {
	memmove(&header, &hdr, sizeof(RFM_HEADER));
	header.macheader = 0x40;
}

rfmHeader::rfmHeader(
	const DEVADDR &addr
) {
	memset(&header, 0, sizeof(RFM_HEADER));
	memcpy(&header.devaddr, &addr, sizeof(DEVADDR));
	header.macheader = 0x40;
}

rfmHeader::rfmHeader(
	const DEVADDR &addr,
	uint16_t frameCounter
) {
	memcpy(&header.devaddr, &addr, sizeof(DEVADDR));
	header.framecountertx = frameCounter;
	fport = 0;
	header.framecontrol = 0;
	header.macheader = 0x40;
}

rfmHeader::rfmHeader(
	const DEVADDR &addr,
	uint16_t frameCounter,
	uint8_t framePort
) {
	header.macheader = 0x40;
	memcpy(&header.devaddr, &addr, sizeof(DEVADDR));
	header.framecountertx = frameCounter;
	fport = framePort;
	header.framecontrol = 0;
}

rfmHeader::rfmHeader(
	const std::string &value
) {
	parse(value);
}

bool rfmHeader::parse(
	const std::string &value
) 
{
	size_t sz = sizeof(RFM_HEADER);
	bool r = sz <= value.size();
	if (!r)
		sz = value.size();
	if (sz > 0)
		memcpy(&header, value.c_str(), sz);
	return r;
}

std::string rfmHeader::toString() const {
	RFM_HEADER h;
	*((uint32_t*) &h.devaddr) = ntoh4(*((uint32_t *) &header.devaddr));
	*((uint16_t*) &h.framecountertx) = ntoh2(*((uint16_t *) &header.framecountertx));
	std::string r((const char *) &h, sizeof(RFM_HEADER));
	return r;
}

semtechUDPPacket::semtechUDPPacket() 
	: errcode(0)
{
	prefix.version = 2;
	prefix.token = 0;
	prefix.tag = 0;

	header.header.macheader = 0x40;
	memset(&header.header.devaddr, 0, sizeof(DEVADDR));			// MAC address

	header.header.framecontrol = 0;
	header.header.framecountertx = 0;
	header.fport = 0;
	header.header.framecontrol = 0;

	memset(&header.header.devaddr, 0, sizeof(DEVADDR));
	

	memset(&prefix.mac, 0, sizeof(prefix.mac));
}

int semtechUDPPacket::parse(
	std::vector<semtechUDPPacket> &retPackets,
	const void *packetForwarderPacket,
	int size
) {
	if (size < sizeof(SEMTECH_LORA_PREFIX)) {
		return ERR_CODE_INVALID_PACKET;
	}
	SEMTECH_LORA_PREFIX rprefix;
	memmove(&rprefix, packetForwarderPacket, sizeof(SEMTECH_LORA_PREFIX));
	// check version

	if (rprefix.version != 2) {
		return ERR_CODE_INVALID_PACKET;
	}
	char *json = (char *) packetForwarderPacket + sizeof(SEMTECH_LORA_PREFIX);
	if (!json)
		return ERR_CODE_INVALID_JSON;

	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator(doc.GetAllocator());
	doc.Parse(json);
	if (!doc.IsObject())
		return ERR_CODE_INVALID_JSON;

	int r = 0;

	// rapidjson::StringRef(METADATA_NAMES[1]))
	if (!doc.HasMember(METADATA_NAMES[0]))
		return ERR_CODE_INVALID_JSON;
	rapidjson::Value &rxpk = doc[METADATA_NAMES[0]];
	if (!rxpk.IsArray())
		return ERR_CODE_INVALID_JSON;
	for (int i = 0; i < rxpk.Size(); i++) {
		rapidjson::Value &jm = rxpk[i];
		if (!jm.IsObject())
			return ERR_CODE_INVALID_JSON;
		rfmMetaData m;
		int sz;
		std::string data;
		int rr = m.parse(sz, data, jm);
		if (rr)
			return rr;
		semtechUDPPacket packet(&rprefix, &m, data);
		retPackets.push_back(packet);
	}
	return r;
}

/**
 * @see https://stackoverflow.com/questions/10324/convert-a-hexadecimal-string-to-an-integer-efficiently-in-c/11068850
 */

/** 
 * @brief convert a hexidecimal string to a signed long
 * will not produce or process negative numbers except 
 * to signal error.
 * 
 * @param hex without decoration, case insensitive. 
 * 
 * @return -1 on error, or result (max (sizeof(long)*8)-1 bits)
 */
static int hexdec(unsigned char *value) {
	int r;
	if (!*value)
		return 0;
	if (*value >= '0' && *value <= '9') 
		r = *value - '0';
    else
		if (*value >= 'a' && *value <='f')
			r = *value - 'a' + 10;
    	else
			if (*value >= 'A' && *value <='F')
				r = *value - 'A' + 10; 
	r <<= 4;
	value++;
	if (*value) {
		if (*value >= '0' && *value <= '9') 
			r += *value - '0';
		else
			if (*value >= 'a' && *value <='f')
				r += *value - 'a' + 10;
			else
				if (*value >= 'A' && *value <='F')
					r += *value - 'A' + 10; 
	}
    return r;
}

static void setAddr(
	DEVADDR &retval,
	const std::string &value
) {
	if (value.size() == sizeof(DEVADDR)) {
		*(uint32_t*) retval = ntoh4(*(uint32_t *) value.c_str());
		return;
	}
	memset(retval, 0, sizeof(DEVADDR));
	if (value.size() < sizeof(DEVADDR) * 2)
		return;
	unsigned char *s = (unsigned char *) value.c_str();
#if BYTE_ORDER == BIG_ENDIAN
	for (int i = 0; i < sizeof(DEVADDR); i++) {
		retval[i] = hexdec(s);
		s += 2;
	}
#else
	for (int i = sizeof(DEVADDR) - 1; i >= 0; i--) {
		retval[i] = hexdec(s);
		s += 2;
	}
#endif	
}

static void setMAC(
	DEVEUI &retval,
	const std::string &value
) {
	if (value.size() == sizeof(DEVEUI)) {
		*(uint64_t*) retval = ntoh8(*(uint64_t *) value.c_str());
		return;
	}
	if (value.size() < sizeof(DEVEUI) * 2)
		return;
	memset(retval, 0, sizeof(DEVEUI));
	unsigned char *s = (unsigned char *) value.c_str();
#if BYTE_ORDER == BIG_ENDIAN
	for (int i = 0; i < sizeof(DEVEUI); i++) {
		retval[i] = hexdec(s);
		s += 2;
	}
#else
	for (int i = sizeof(DEVEUI) - 1; i >= 0; i--) {
		retval[i] = hexdec(s);
		s += 2;
	}
#endif	
}

void setKey(
	KEY128 &retval,
	const std::string &value
) {
	if (value.size() == sizeof(KEY128)) {
		memcpy(retval, value.c_str(), sizeof(KEY128));
		ntoh16(retval);
		return;
	}
	if (value.size() < sizeof(KEY128) * 2)
		return;
	unsigned char *s = (unsigned char *) value.c_str();
#if BYTE_ORDER == BIG_ENDIAN
	for (int i = 0; i < sizeof(KEY128); i++) {
		retval[i] = hexdec(s);
		s += 2;
	}
#else
	for (int i = sizeof(KEY128) - 1; i >= 0; i--) {
		retval[i] = hexdec(s);
		s += 2;
	}
#endif	
}

/**
 * format = 0 hex
 */ 
semtechUDPPacket::semtechUDPPacket(
	const std::string &data,
	const std::string &devaddr,
	const std::string &appskey
)
	: errcode(0)
{
	clearPrefix();

	// initialize header ?!!
	memset(&header.header, 0, sizeof(RFM_HEADER));
	header.header.macheader = 0x40;
	setAddr(header.header.devaddr, devaddr);

	// autentication keys
	setKey(devId.appSKey, appskey);

	parseData(data);
}

void semtechUDPPacket::clearPrefix()
{
	prefix.version = 2;
	prefix.token = 0;
	prefix.tag = 0;
	memset(&prefix.mac, 0, sizeof(prefix.mac));
}

semtechUDPPacket::semtechUDPPacket(
	const SEMTECH_LORA_PREFIX *aprefix,
	const rfmMetaData *ametadata,
	const std::string &data
)
	: errcode(0)
{
	if (aprefix)
		memmove(&prefix, aprefix, sizeof(SEMTECH_LORA_PREFIX));
	else {
		clearPrefix();
	}
	if (ametadata)
		metadata.push_back(rfmMetaData(*ametadata));
	parseData(data);
}

static std::string getMAC(
	const DEVEUI &value
) {
	return hexString(&value, sizeof(DEVEUI));
}

RFM_HEADER *semtechUDPPacket::getRfmHeader() {
	
	return &header.header;
}

rfmHeader *semtechUDPPacket::getHeader() {
	return &header;
}

void semtechUDPPacket::setRfmHeader(
	const RFM_HEADER &value
) {
	header = rfmHeader(value);
}

/**
 * prefix  Gateway-MAC-addr RFM-header
 * 0 1 2 3 4 5 6 7 8 9 A B  C                 DAT mic  
 * 0200000000006cc3743eed46 401111111100000000a0ccfb8e
 *                          401111111100000000
 * VEttttTTaaaaaaaaaaaaaaaa   DEVICEAD
 * Version                  40- unconfirmed uplink 
 *   token                            FC    PO
 *       Tag                            Coun
 * 
 * 401111111100000001a1a46ff045b570
 *   DEV-ADDR  FRAMCN  PAYLOA
 * MH        FC(1.0) FP      MIC
 *           FRAME-CN (1.1)
 * packet.getDeviceAddr();
 * FP Frame port
 
 * 
 */
std::string semtechUDPPacket::serialize2RfmPacket() const
{
	std::stringstream ss;
	std::string p(payload);

	// direction of frame is up
	unsigned char direction = 0x00;

	// build radio packet, unconfirmed data up macHeader = 0x40;
	// RFM header 8 bytes
	ss << header.toString() << header.fport;

	// load data
	// encrypt data
	encryptPayload(p, header.header.framecountertx, direction, header.header.devaddr, devId.appSKey);
	ss << p;

	std::string rs = ss.str();
	// calc MIC
	uint32_t mic = calculateMIC(rs, header.header.framecountertx, direction, header.header.devaddr, devId.nwkSKey);	// nwkSKey
	// load MIC in package
	// mic = ntoh4(mic);
	ss << std::string((char *) &mic, 4);
	return ss.str();
}

std::string semtechUDPPacket::toString() const
{
	std::stringstream ss;
	std::string p(payload);

	// prefix 12 bytes, metadata + payload
	ss << std::string((const char *) &prefix, sizeof(SEMTECH_LORA_PREFIX))
		<< metadataToJsonString();
	return ss.str();
}

std::string semtechUDPPacket::metadataToJsonString() const
{
	std::string d(serialize2RfmPacket());
	std::stringstream ss;
	ss << "{\"rxpk\":[";
	bool needColon = false;
	for (std::vector<rfmMetaData>::const_iterator it(metadata.begin()); it != metadata.end(); it++) {
		if (needColon)
			ss << ",";
		ss << it->toJsonString(d);
		needColon = true;
	}
	ss << "]}";
	return ss.str();
}

void semtechUDPPacket::setGatewayId(
	const std::string &value
) {
	setMAC(prefix.mac, value);
}

std::string semtechUDPPacket::getDeviceEUI() {
	return getMAC(devId.deviceEUI);
}

void semtechUDPPacket::setDeviceEUI(
	const std::string &value
) {
	setMAC(devId.deviceEUI, value);
}

std::string semtechUDPPacket::getDeviceAddrStr() const {
	return DEVADDR2string(header.header.devaddr);
}

DEVADDRINT semtechUDPPacket::getDeviceAddr() const {
	return *(DEVADDRINT *) &header.header.devaddr;
}

void semtechUDPPacket::setDeviceAddr(
	const std::string &value
) {
	setAddr(header.header.devaddr, value);
}

void semtechUDPPacket::setNetworkSessionKey(
	const std::string &value
) {
	setKey(devId.nwkSKey, value);
}

void semtechUDPPacket::setApplicationSessionKey(
	const std::string &value
) {
	setKey(devId.appSKey, value);
}

void semtechUDPPacket::setFrameCounter(
	uint16_t value
) {
	header.header.framecountertx = value;
}

std::string semtechUDPPacket::getPayload() {
	return payload;
}

int semtechUDPPacket::setPayload(
	uint8_t port,
	const std::string &value
) {
	header.fport = port;
	header.header.framecontrol = 0;
	payload = value;
}

int semtechUDPPacket::setPayload(
	const std::string &value
) {
	payload = value;
}

void semtechUDPPacket::ack(SEMTECH_ACK *retval) {	// 4 bytes
	retval->version = 2;
	retval->token = prefix.token;
	retval->tag = 1;	// PUSH_ACK
}

std::string key2string(
	const KEY128 &value
) {
	std::stringstream ss;
	ss << std::hex << std::setw(2) << std::setprecision(2) << std::setfill('0');
	for (int i = 0; i < sizeof(KEY128); i++) {
		ss << (unsigned int)  value[i];
	}
	return ss.str();
}

int semtechUDPPacket::parseData(
	const std::string &data
) {
	if (!header.parse(data)) {
		return ERR_CODE_INVALID_RFM_HEADER;
	}
	if (loadCredentialsDevAddr()) {
		return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
	}
	char direction = 0;
	std::string p = data.substr(sizeof(RFM_HEADER) + sizeof(uint8_t) , data.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t));
	decryptPayload(p, header.header.framecountertx, direction, header.header.devaddr, devId.appSKey);
	setPayload(p); 
	return LORA_OK;
}

int semtechUDPPacket::loadCredentialsDevAddr() 
{
	const DEVADDR &devaddr = header.header.devaddr;
}