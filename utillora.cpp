#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#pragma clang diagnostic pop

#include "platform.h"
#include "utillora.h"
#include "utildate.h"
#include "utilstring.h"
#include "base64/base64.h"

#include "errlist.h"

#include "system/crypto/aes.h"
#include "system/crypto/cmac.h"

#include "identity-service-abstract.h"

#include "lora-encrypt.h"

#if BYTE_ORDER == BIG_ENDIAN
#define swap16(x) (x)
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
#define swapVar(x) swapBytes(&x, sizeof(x));
#define swap16(x) swapBytes(x, 16);
#endif

std::string deviceclass2string(
	DEVICECLASS value
) {
	switch(value) {
		case CLASS_A:
			return "A";
		case CLASS_B:
			return "B";
		default:
			return "C";
	};
}

DEVICECLASS string2deviceclass
(
	const std::string &value
)
{
	if (value == "A")
		return CLASS_A;
	else
		if (value == "B")
			return CLASS_B;
		else
			return CLASS_C;
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
uint32_t calculateMIC(
	const std::string &payload,
	const unsigned int frameCounter,
	const unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &key
)
{
	AES_CMAC_CTX aesCmacCtx;
	unsigned char *data = (unsigned char *) payload.c_str();
	unsigned char size = payload.size();
	unsigned char blockB[16];
	// blockB
	blockB[0] = 0x49;
	blockB[1] = 0x00;
	blockB[2] = 0x00;
	blockB[3] = 0x00;
	blockB[4] = 0x00;

	blockB[5] = direction;

	blockB[6] = devAddr[0];
	blockB[7] = devAddr[1];
	blockB[8] = devAddr[2];
	blockB[9] = devAddr[3];

	blockB[10] = (frameCounter & 0x00FF);
	blockB[11] = ((frameCounter >> 8) & 0x00FF);

	blockB[12] = 0x00; // Frame counter upper bytes
	blockB[13] = 0x00;

	blockB[14] = 0x00;
	blockB[15] = size;

	aes_context aesContext;

	memset(aesContext.ksch, '\0', 240);
    aes_set_key(key, sizeof(KEY128), &aesContext);
	AES_CMAC_Init(&aesCmacCtx);
	AES_CMAC_SetKey(&aesCmacCtx, key);
	AES_CMAC_Update(&aesCmacCtx, blockB, sizeof(blockB));
	AES_CMAC_Update(&aesCmacCtx, data, size);
	uint32_t r;
	uint8_t mic[16];
	AES_CMAC_Final(mic, &aesCmacCtx);
    r = (uint32_t) ((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
	return r;
}

NetworkIdentity::NetworkIdentity()
{
}

NetworkIdentity::NetworkIdentity(
	const DEVADDRINT &a,
	const DEVICEID &value
) 
{
	memmove(&devaddr, &a.a, sizeof(DEVADDR));
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&deviceEUI, &value.deviceEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	memmove(&name, &value.name, sizeof(DEVICENAME));
}

std::string KEY2string(
	const KEY128 &value
)
{
	KEY128 v;
	memmove(&v, &value, sizeof(v));
	return hexString(&v, sizeof(v));
}

std::string NetworkIdentity::toString() const
{
	std::stringstream ss;
	ss << DEVADDR2string(devaddr) 
		<< " " << (activation == 1 ? "OTAA" : "ABP")
		<< " " << deviceclass2string(deviceclass)
		<< " " << DEVEUI2string(deviceEUI)
		<< " " << KEY2string(nwkSKey)
		<< " " << KEY2string(appSKey)
		<< " " << std::string(name, sizeof(DEVICENAME));
	return ss.str();
}

DeviceId::DeviceId() {
	memset(&deviceEUI, 0, sizeof(DEVEUI));
	memset(&nwkSKey, 0, sizeof(KEY128));
	memset(&appSKey, 0, sizeof(KEY128));
	memset(&name, 0, sizeof(DEVICENAME));
}

DeviceId::DeviceId(
	const DeviceId &value
) {
	memmove(&deviceEUI, &value.deviceEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	memset(&name, 0, sizeof(DEVICENAME));
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
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&deviceEUI, &value.deviceEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	memmove(&name, &value.name, sizeof(DEVICENAME));
}

void NetworkIdentity::set(
	const DEVADDRINT &addr,
	const DEVICEID &value
)
{
	memmove(&devaddr, &addr.a, sizeof(DEVADDR));
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&deviceEUI, &value.deviceEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	memmove(&name, &value.name, sizeof(DEVICENAME));
}

// const std::string DEF_DATA_RATE = "SF7BW125";
// const std::string DEF_ECCCODE_RATE = "4/6";
#define DEF_BANDWIDTH BW_200KHZ
#define DEF_SPREADING_FACTOR DRLORA_SF7
#define DEF_CODING_RATE CRLORA_4_6

#define DEF_RSSI	-35
#define DEF_LSNR	5.1

rfmMetaData::rfmMetaData() 
	: chan(0), rfch(0), freq(868900000), stat(0), 
	modu(LORA), bandwith(DEF_BANDWIDTH), spreadingFactor(DEF_SPREADING_FACTOR),
 	codingRate(DEF_CODING_RATE), bps(0), rssi(DEF_RSSI), lsnr(DEF_LSNR)
{
	time(&t);			// UTC time of pkt RX, us precision, ISO 8601 'compact' format
}

/*
200:
203:

400:
406:

800:
812:

1600:
1625:

*/
/**
 * @return  LoRa datarate identifier e.g. "SF7BW125"
 */
std::string rfmMetaData::datr() const
{
	int bandwithValue;
	switch (bandwith) {
		case BW_7KHZ:
			bandwithValue = 7; // 7.8
			break;
		case BW_10KHZ:
			bandwithValue = 10; // 10.4
			break;
		case BW_15KHZ:
			bandwithValue = 15; // 15.6
			break;
		case BW_20KHZ:
			bandwithValue = 20; // 20.8
			break;
		case BW_31KHZ:
			bandwithValue = 31; // 31.2
			break;
		case BW_41KHZ:
			bandwithValue = 41; // 41.6
			break;
		case BW_62KHZ:
			bandwithValue = 62; // 62.5
			break;
		case BW_125KHZ:
			bandwithValue = 125; // 125
			break;

		case BW_200KHZ:
			bandwithValue = 203; // 200
			break;
		case BW_400KHZ:
			bandwithValue = 406; // 400
			break;
		case BW_800KHZ:
			bandwithValue = 812; // 800
			break;
		case BW_1600KHZ:
			bandwithValue = 1625; // 1600
			break;
		default:
			bandwithValue = 203;
			break;
	}
	std::stringstream ss;
	ss << "SF" << (int) spreadingFactor 
		<< "BW"  << bandwithValue;
	return ss.str();
}

/**
 * @param value LoRa datarate identifier e.g. "SF7BW125"
 */
void rfmMetaData::setDatr(
	const std::string &value
) 
{
	size_t sz = value.size();
	if (sz < 3)
		return;
	std::size_t p = value.find('B');
	if (p == std::string::npos)
		return;
	std::string s = value.substr(2, p - 2);
	spreadingFactor = static_cast<SPREADING_FACTOR>(atoi(s.c_str()));
	s = value.substr(p + 2);
	int bandwithValue = atoi(s.c_str());
	switch (bandwithValue) {
		case 7:
			bandwith = BW_7KHZ; // 7.8
			break;
		case 10:
			bandwith = BW_10KHZ; // 10.4
			break;
		case 15:
			bandwith = BW_15KHZ; // 15.6
			break;
		case 20:
			bandwith = BW_20KHZ; // 20.8
			break;
		case 31:
			bandwith = BW_31KHZ; // 31.2
			break;
		case 41:
			bandwith = BW_41KHZ; // 41.6
			break;
		case 62:
			bandwith = BW_62KHZ; // 62.5
			break;
		case 125:
			bandwith = BW_125KHZ; // 125
			break;
		case 200:
		case 203:
			bandwith = BW_200KHZ;
			break;
		case 400:
		case 406:
			bandwith = BW_400KHZ;
			break;
		case 800:
		case 812:
			bandwith = BW_800KHZ;
			break;
		case 1600:
		case 1625:
			bandwith = BW_1600KHZ;
			break;
		default:
			bandwith = BW_200KHZ;
			break;
	}
}

/**
 * @return LoRa ECC coding rate identifier e.g. "4/6"
 */
std::string rfmMetaData::codr() const
{
	switch(codingRate) {
		case CRLORA_4_5: // 1
			return "4/5";
		case CRLORA_4_6: // 2
			return "2/3";
		case CRLORA_4_7: // 3
			return "4/7";
		case CRLORA_4_8: // 4
			return "1/2";
		case CRLORA_LI_4_5: // 5
			return "4/5LI";
		case CRLORA_LI_4_6: // 6
			return "4/6LI";
		case CRLORA_LI_4_8: // 7
			return "4/8LI";
		default:
			return "";
	}
}

/**
 * @param LoRa LoRa ECC coding rate identifier e.g. "4/6"
 */
void rfmMetaData::setCodr(
	const std::string &value
)
{
	size_t sz = value.size();
	switch (sz) {
		case 3:
			switch(value[2]) {
				case '5':
					codingRate = CRLORA_4_5;
					break;	
				case '2':
					codingRate = CRLORA_4_6;
					break;	
				case '7':
					codingRate = CRLORA_4_7;
					break;	
				case '1':
					codingRate = CRLORA_4_8;
					break;	
				default:
					codingRate = DEF_CODING_RATE;
			}
			break;
		case 5:
			switch(value[2]) {
				case '5':
					codingRate = CRLORA_LI_4_5;
					break;	
				case '6':
					codingRate = CRLORA_LI_4_6;
					break;	
				case '8':
					codingRate = CRLORA_LI_4_8;
					break;	
				default:
					codingRate = DEF_CODING_RATE;
			}
			break;
		default:
			codingRate = DEF_CODING_RATE;
	}
}

rfmMetaData::rfmMetaData(
	const rfmMetaData &value
)
	: t(value.t), chan(value.chan), rfch(value.rfch), freq(value.freq), stat(value.stat), 
	modu(value.modu), bandwith(value.bandwith),
	spreadingFactor(value.spreadingFactor), codingRate(value.codingRate), 
	bps(value.bps), rssi(value.rssi), lsnr(value.lsnr)
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
	ss << mhz << "." << (freq - (mhz * 1000000));
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
const char* METADATA_RX_NAMES[15] = {
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
		if (strcmp(METADATA_RX_NAMES[i], name) == 0)
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
	std::string v;
	if (len > sizeof(KEY128))
		v = hex2string(str);
	else
		v= str;
	len = v.size();
	if (len > sizeof(KEY128))
		len = sizeof(KEY128);
	memmove(&retval, v.c_str(), len);
	if (len < sizeof(KEY128))
		memset(&retval + len, 0, sizeof(KEY128) - len);
}

void string2DEVICENAME(
	DEVICENAME &retval,
	const char *str
)
{
	strncpy(retval, str, sizeof(DEVICENAME));
}

void int2DEVADDR(
	DEVADDR &retval,
	uint32_t value
)
{
	// *((uint32_t*) &retval) = ntoh4(value);
	*((uint32_t*) &retval) = value;
}

std::string DEVADDR2string(
	const DEVADDR &value
)
{
	uint32_t v;
	memmove(&v, &value, sizeof(v));
	// hex string is MSB first, swap if need it
	v = ntoh4(v);
	return hexString(&v, sizeof(v));
}

std::string uint64_t2string(
	const uint64_t &value
) {
	uint64_t v;
	memmove(&v, &value, sizeof(v));
	// hex string is MSB first, swap if need it
	v = ntoh8(v);
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
	// EUI stored in memory as 8 bit integer x86 LSB first, ARM MSB first
	uint64_t v;
	memmove(&v, &value, sizeof(DEVEUI));
	// hex string is MSB first, swap if need it
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
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[1])), v1, allocator);

	rapidjson::Value v2(tmms());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[2])), v2, allocator);

	rapidjson::Value v3(tmst);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[3])), v3, allocator);

	rapidjson::Value v4(freq);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[4])), v4, allocator);

	rapidjson::Value v5(chan);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[5])), v5, allocator);

	rapidjson::Value v6(rfch);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[6])), v6, allocator);

	rapidjson::Value v7(stat);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[7])), v7, allocator);

	rapidjson::Value v8(rapidjson::kStringType);
	std::string s8(modulation());
	v8.SetString(s8.c_str(), s8.length());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[8])), v8, allocator);

	rapidjson::Value v9(rapidjson::kStringType);
	std::string dr = datr();
	v9.SetString(dr.c_str(), dr.length());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[9])), v9, allocator);

	rapidjson::Value v10(rapidjson::kStringType);
	std::string cr = codr();
	v10.SetString(cr.c_str(), cr.length());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[10])), v10, allocator);

	rapidjson::Value v11(rssi);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[11])), v11, allocator);

	rapidjson::Value v12(lsnr);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[12])), v12, allocator);

	rapidjson::Value v13(data.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[13])), v13, allocator);

	rapidjson::Value v14(rapidjson::kStringType);
	std::string d(base64_encode(data));	// base64

	v14.SetString(d.c_str(), d.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[14])), v14, allocator);
}

int rfmMetaData::parse(
	int &retSize,
	std::string &retData,
	rapidjson::Value &value
) {
	if (value.HasMember(METADATA_RX_NAMES[1])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[1]];
		if (v.IsString()) {
			t = parseDate(v.GetString());
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[3])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[3]];
		if (v.IsInt()) {
			tmst = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[4])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[4]];
		if (v.IsDouble()) {
			freq = v.GetDouble() * 1000000;
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[5])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[5]];
		if (v.IsInt()) {
			chan = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[6])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[6]];
		if (v.IsInt()) {
			rfch = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[7])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[7]];
		if (v.IsInt()) {
			stat = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[8])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[8]];
		if (v.IsString()) {
			setModulation(v.GetString());
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[9])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[9]];
		if (v.IsString()) {
			setDatr(v.GetString());
		}
	}
	
	if (value.HasMember(METADATA_RX_NAMES[10])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[10]];
		if (v.IsString()) {
			setCodr(v.GetString());
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[11])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[11]];
		if (v.IsInt()) {
			rssi = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[12])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[12]];
		if (v.IsInt()) {
			lsnr = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[13])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[13]];
		if (v.IsInt()) {
			retSize = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[14])) {
		rapidjson::Value &v = value[METADATA_RX_NAMES[14]];
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
		<< "\"" << METADATA_RX_NAMES[1] << "\":\"" << dt
		<< "\",\"" << METADATA_RX_NAMES[2] << "\":" << tmms()
		<< ",\"" << METADATA_RX_NAMES[3] << "\":" << tmst
		<< ",\"" << METADATA_RX_NAMES[4] << "\":" << frequency()
		<< ",\"" << METADATA_RX_NAMES[5] << "\":" << (int) chan
		<< ",\"" << METADATA_RX_NAMES[6] << "\":" << (int) rfch
		<< ",\"" << METADATA_RX_NAMES[7] << "\":" << (int) stat
		<< ",\"" << METADATA_RX_NAMES[8] << "\":\"" << modulation()
		<< "\",\"" << METADATA_RX_NAMES[9] << "\":\"" << datr()
		<< "\",\"" << METADATA_RX_NAMES[10] << "\":\"" << codr()
		<< "\",\"" << METADATA_RX_NAMES[11] << "\":" << rssi
		<< ",\"" << METADATA_RX_NAMES[12] << "\":" << lsnr
		<< ",\"" << METADATA_RX_NAMES[13] << "\":" << data.size()
		<< ",\"" << METADATA_RX_NAMES[14] << "\":\"" << base64_encode(data) << "\"}";
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
	uint16_t fcnt
) {
	memcpy(&header.devaddr, &addr, sizeof(DEVADDR));
	header.fcnt = fcnt;
	fport = 0;
	header.fctrl.i = 0;
	header.macheader = 0x40;
}

rfmHeader::rfmHeader(
	const DEVADDR &addr,
	uint16_t frameCounter,
	uint8_t framePort
) {
	header.macheader = 0x40;
	memcpy(&header.devaddr, &addr, sizeof(DEVADDR));
	header.fcnt = frameCounter;
	fport = framePort;
	header.fctrl.i = 0;
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
	h.fcnt = ntoh2(header.fcnt);

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

	header.header.fctrl.i = 0;
	header.header.fcnt = 0;
	header.fport = 0;

	memset(&header.header.devaddr, 0, sizeof(DEVADDR));
	

	memset(&prefix.mac, 0, sizeof(prefix.mac));
}

/**
 * Parse Semtech UDP packet
 * @return 0, ERR_CODE_INVALID_PACKET or ERR_CODE_INVALID_JSON
 */ 
int semtechUDPPacket::parse(
	SEMTECH_DATA_PREFIX &retprefix,
	GatewayStat &retgwstat,
	std::vector<semtechUDPPacket> &retPackets,
	const void *packetForwarderPacket,
	int size,
	IdentityService *identityService
) {
	retgwstat.errcode = ERR_CODE_NO_GATEWAY_STAT;

	if (size < sizeof(SEMTECH_DATA_PREFIX)) {
		return ERR_CODE_PACKET_TOO_SHORT;
	}
	memmove(&retprefix, packetForwarderPacket, sizeof(SEMTECH_DATA_PREFIX));
	// check version

	if (retprefix.version != 2) {
		return ERR_CODE_INVALID_PROTOCOL_VERSION;
	}
	char *json = (char *) packetForwarderPacket + sizeof(SEMTECH_DATA_PREFIX);
	if (size == sizeof(SEMTECH_DATA_PREFIX)) {
		return 0;	// that's ok
	}

	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator(doc.GetAllocator());
	doc.Parse(json);
	if (!doc.IsObject())
		return ERR_CODE_INVALID_JSON;

	int r = 0;

	// rapidjson::StringRef(METADATA_RX_NAMES[1]))

	if (doc.HasMember("stat")) {
		rapidjson::Value &jstat = doc["stat"];
		if (retgwstat.parse(jstat) == 0) {
			// set gateway identifier
			retgwstat.gatewayId = deveui2int(retprefix.mac);
		}
	} else {
		retgwstat.errcode = ERR_CODE_NO_GATEWAY_STAT;
	}

	if (!doc.HasMember(METADATA_RX_NAMES[0]))
		return 0;	// that's ok

	rapidjson::Value &rxpk = doc[METADATA_RX_NAMES[0]];
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
		semtechUDPPacket packet(&retprefix, &m, data, identityService);
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
		swap16(retval);
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

	parseData(data, NULL);
}

void semtechUDPPacket::clearPrefix()
{
	prefix.version = 2;
	prefix.token = 0;
	prefix.tag = 0;
	memset(&prefix.mac, 0, sizeof(prefix.mac));
}

semtechUDPPacket::semtechUDPPacket(
	const SEMTECH_DATA_PREFIX *aprefix,
	const rfmMetaData *ametadata,
	const std::string &data,
	IdentityService *identityService
)
	: errcode(0)
{
	if (aprefix)
		memmove(&prefix, aprefix, sizeof(SEMTECH_DATA_PREFIX));
	else {
		clearPrefix();
	}
	if (ametadata)
		metadata.push_back(rfmMetaData(*ametadata));
	parseData(data, identityService);
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
	encryptPayload(p, header.header.fcnt, direction, header.header.devaddr, devId.appSKey);
	ss << p;

	std::string rs = ss.str();
	// calc MIC
	uint32_t mic = calculateMIC(rs, header.header.fcnt, direction, header.header.devaddr, devId.nwkSKey);	// nwkSKey
	// load MIC in package
	// mic = ntoh4(mic);
	ss << std::string((char *) &mic, 4);
	return ss.str();
}

std::string semtechUDPPacket::toString() const
{
	std::stringstream ss;
	// prefix 12 bytes, metadata + payload
	ss << std::string((const char *) &prefix, sizeof(SEMTECH_DATA_PREFIX))
		<< metadataToJsonString();
	return ss.str();
}

std::string semtechUDPPacket::toDebugString() const
{
	std::stringstream ss;
	ss << "device " << getDeviceAddrStr()
		<< ": "
		<< hexString(payload);
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

/**
 * 0- strongest, -120- nothing
 * @return INT16_MIN if N/A
 */
int16_t semtechUDPPacket::getStrongesSignalLevel(int &idx) const {
	int16_t r = INT16_MIN;;
	idx = -1;
	for (int i = 0; i < metadata.size(); i++) {
		if (metadata[0].rssi > r) {
			r = metadata[0].rssi;
			idx = i;
		}
	}
	return r;
}

std::string semtechUDPPacket::getDeviceAddrStr() const 
{
	return DEVADDR2string(header.header.devaddr);
}

DEVADDRINT semtechUDPPacket::getDeviceAddr() const {
	DEVADDRINT r = static_cast<DEVADDRINT>(header.header.devaddr);
	return r;
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
	header.header.fcnt = value;
}

std::string semtechUDPPacket::getPayload() {
	return payload;
}

void semtechUDPPacket::setPayload(
	uint8_t port,
	const std::string &value
) {
	header.fport = port;
	header.header.fctrl.i = 0;
	payload = value;
}

void semtechUDPPacket::setPayload(
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
	const std::string &data,
	IdentityService *identityService
) {
	if (!header.parse(data)) {
		return ERR_CODE_INVALID_RFM_HEADER;
	}
	char direction = 0;
	int payloadSize = data.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t) - header.header.fctrl.f.foptslen;
	std::string p = data.substr(sizeof(RFM_HEADER) + sizeof(uint8_t) + header.header.fctrl.f.foptslen, payloadSize);
	// get identity
	if (identityService) {
		// load keys from the authentication service, at least deviceEUI and appSKey. Return 0- success, <0- error code
		int rc = identityService->get(header.header.devaddr, devId);
		if (rc == 0) {
			decryptPayload(p, header.header.fcnt, direction, header.header.devaddr, devId.appSKey);
		} else {
			// return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
		}
		setPayload(p); 
	} else {
		// put cipher data
		setPayload(p); 
	}
	return LORA_OK;
}

uint64_t deveui2int(
	const DEVEUI &value
)
{
	uint64_t v;
	memmove(&v, &value, sizeof(DEVEUI));
	return ntoh8(v);
}

uint32_t getMic(const std::string &v)
{
	uint32_t r = *((uint32_t *) (v.c_str() + v.size() - 4));
	return r; //ntoh4(r);
}

uint64_t str2gatewayId(const char *value) {
	return strtoull(value, NULL, 16);
}

std::string TDEVEUI::toString() {
	return DEVEUI2string(eui);
}