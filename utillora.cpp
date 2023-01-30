#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iomanip>
#include <cstring>
#include <iostream>
#include <sstream>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "platform.h"
#include "utillora.h"
#include "utildate.h"
#include "utilstring.h"
#include "base64/base64.h"

#include "errlist.h"

#include "system/crypto/aes.h"
#include "system/crypto/cmac.h"

#define KSCH_SIZE   (N_MAX_ROUNDS + 1) * N_BLOCK

#include "identity-service.h"

#include "lora-encrypt.h"
#include "lorawan-mac.h"

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
	}
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

std::string activation2string
(
	ACTIVATION value
)
{
	if (value == OTAA)
		return "OTAA";
	else
		return "ABP";
}

ACTIVATION string2activation
(
	const std::string &value
)
{
	if (value == "OTAA")
		return OTAA;
	else
		return ABP;
}

static uint32_t calculateMICRev103(
	const unsigned char *data,
	const unsigned char size,
	const unsigned int frameCounter,
	const unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &key
)
{
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
	memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(key, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
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

/**
 * Calculate MAC Frame Payload Encryption message integrity code
 * @see 4.3.3 MAC Frame Payload Encryption (FRMPayload)
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
	const unsigned char *data,
	const unsigned char size,
	const unsigned int frameCounter,
	const unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &key
)
{
	return calculateMICRev103(
		data,
		size,
		frameCounter,
		direction,
		devAddr,
		key
	);
}

/**
 * Calculate ReJoin Request MIC
 * @see 6.2.5 Join-request frame
 */
uint32_t calculateMICReJoinRequest(
    const JOIN_REQUEST_HEADER *header,
    const KEY128 &key,
    const uint8_t rejoinType
) {
    aes_context aesContext;
    memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(key, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
    AES_CMAC_SetKey(&aesCmacCtx, key);
    AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) header, 1 + sizeof(JOIN_REQUEST_FRAME));
    uint8_t mic[16];
    AES_CMAC_Final(mic, &aesCmacCtx);
    return (uint32_t) ((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
}

uint32_t calculateMICJoinRequest(
        const JOIN_REQUEST_HEADER *header,
        const KEY128 &key
) {
    return calculateMICReJoinRequest(header, key, 0xff);
}

/**
 * JSEncKey is used to encrypt the Join-Accept triggered by a Rejoin-Request
 * @param retval return derived key
 * @param tag Tag (first byte)
 * @param nwkKey NwkKey network key
 * @param value Device EUI
 */
static void deriveKeyBlock(
        KEY128 &retval,
        const KEY128 &nwkKey,
        const void *value,
        size_t size
)
{
    aes_context aesContext;
    memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(nwkKey, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
    AES_CMAC_SetKey(&aesCmacCtx, nwkKey);
    AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) &value, (std::uint32_t) size);
    AES_CMAC_Final(retval, &aesCmacCtx);
}

// The network session keys are derived from the NwkKey:
// FNwkSIntKey = aes128_encrypt(NwkKey, 0x01 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// SNwkSIntKey = aes128_encrypt(NwkKey, 0x03 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// NwkSEncKey = aes128_encrypt(NwkKey, 0x04 | JoinNonce | JoinEUI | DevNonce | pad 16 )
/**
 * JSEncKey is used to encrypt the Join-Accept triggered by a Rejoin-Request
 * @param retval return derived key
 * @param tag Tag (first byte)
 * @param nwkKey NwkKey network key
 * @param value Device EUI
 */
static void deriveKey(
        KEY128 &retval,
        uint8_t tag,
        const KEY128 &nwkKey,
        const void *value,
        size_t size
)
{
    uint8_t block[16];
    block[0] = tag;
    memmove(&(block[1]), value, size);
    memset(&(block[9]), '\0', 7);
    deriveKeyBlock(retval, nwkKey, &block, sizeof(block));
}

/**
 * Derive JSIntKey used in Accept Join response in MIC calculation
 * @param retval return derived key
 * @param nwkKey NwkKey network key
 * @param devEUI Device EUI
 */
void deriveJSIntKey(
        KEY128 &retval,
        const KEY128 &nwkKey,
        const DEVEUI &devEUI
)
{
    deriveKey(retval, 6, nwkKey, &devEUI, sizeof(DEVEUI));
}

/**
 * JSEncKey is used to encrypt the Join-Accept triggered by a Rejoin-Request
 * @param retval return derived key
 * @param nwkKey NwkKey network key
 * @param devEUI Device EUI
 */
void deriveJSEncKey(
    KEY128 &retval,
    const KEY128 &nwkKey,
    const DEVEUI &devEUI
)
{
    deriveKey(retval, 5, nwkKey, &devEUI, sizeof(DEVEUI));
}

/**
 * DeriveSessionKey
 * @param retval
 * @param tag
 * @param key
 * @param devEUI
 * @param joinNonce
 * @param devNonce
 */
static void deriveDevSessionKey(
        KEY128 &retval,
        uint8_t tag,
        const KEY128 &key,
        const DEVEUI &devEUI,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    uint8_t block[16];
    block[0] = tag;
    memmove(&(block[1]), &joinNonce, sizeof(JOINNONCE));                // + 3 = 4
    memmove(&(block[1 + sizeof(joinNonce)]), &devEUI, sizeof(DEVEUI));  // + 8 = 12
    memmove(&(block[1 + sizeof(joinNonce) + sizeof(devEUI)]), &devNonce, sizeof(DEVNONCE)); // + 2 = 14
    memset(&(block[14]), '\0', 2);
    deriveKeyBlock(retval, key, &block, sizeof(block));
}

/**
 * DeriveSessionKey
 * @param retval
 * @param tag
 * @param key
 * @param netId
 * @param joinNonce
 * @param devNonce
 */
static void deriveNetSessionKey(
        KEY128 &retval,
        uint8_t tag,
        const KEY128 &key,
        const NETID &netId,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    uint8_t block[16];
    block[0] = tag;
    memmove(&(block[1]), &joinNonce, sizeof(JOINNONCE));                // + 3 = 4
    memmove(&(block[1 + sizeof(joinNonce)]), &netId, sizeof(NETID));    // + 3 = 7
    memmove(&(block[1 + sizeof(joinNonce) + sizeof(NETID)]), &devNonce, sizeof(DEVNONCE)); // + 2 = 9
    memset(&(block[9]), '\0', 7);
    deriveKeyBlock(retval, key, &block, sizeof(block));
}

// OptNeg is set
// FNwkSIntKey = aes128_encrypt(NwkKey, 0x01 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// SNwkSIntKey = aes128_encrypt(NwkKey, 0x03 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// NwkSEncKey = aes128_encrypt(NwkKey, 0x04 | JoinNonce | JoinEUI | DevNonce | pad 16 )
void deriveOptNegFNwkSIntKey(
        KEY128 &retval,
        const KEY128 &key,
        const DEVEUI &joinEUI,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    deriveDevSessionKey(retval, 1, key, joinEUI, joinNonce, devNonce);
}

void deriveOptNegSNwkSIntKey(
        KEY128 &retval,
        const KEY128 &key,
        const DEVEUI &joinEUI,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    deriveDevSessionKey(retval, 3, key, joinEUI, joinNonce, devNonce);
}

void deriveOptNegNwkSEncKey(
        KEY128 &retval,
        const KEY128 &key,
        const DEVEUI &joinEUI,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    deriveDevSessionKey(retval, 4, key, joinEUI, joinNonce, devNonce);
}

// AppSKey = aes128_encrypt(NwkKey, 0x02 | JoinNonce | NetID | DevNonce | pad 161 )
// FNwkSIntKey = aes128_encrypt(NwkKey, 0x01 | JoinNonce | NetID | DevNonce | pad 16 )
// SNwkSIntKey = NwkSEncKey = FNwkSIntKey.
/**
 * OptNeg is unset
 * Derive AppSKey
 * @param retval derived key
 * @param key key
 * @param netid Network identifier
 * @param joinNonce Join nonce
 * @param devNonce Device nonce
 */
void deriveAppSKey(
    KEY128 &retval,
    const KEY128 &key,
    const NETID &netId,
    const JOINNONCE &joinNonce,
    const DEVNONCE &devNonce
)
{
    deriveNetSessionKey(retval, 2, key, netId, joinNonce, devNonce);
}

/**
 * OptNeg is unset
 * Derive SNwkSIntKey = NwkSEncKey = FNwkSIntKey.
 * @param retval derived key
 * @param key key
 * @param netid network identifier
 * @param joinNonce Join nonce
 * @param devNonce Device nonce
 */
void deriveFNwkSIntKey(
        KEY128 &retval,
        const KEY128 &key,
        const NETID &netId,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    deriveNetSessionKey(retval, 1, key, netId, joinNonce, devNonce);
}

/**
 * Calculate Join Response MIC OptNeg is unset (version 1.0)
 * MHDR | JoinNonce | NetID | DevAddr | DLSettings |RxDelay | [CFList]
 * @see 6.2.3 Join-accept message
 */
uint32_t calculateMICJoinResponse(
        const JOIN_ACCEPT_FRAME &frame,
        const KEY128 &key
) {
    aes_context aesContext;
    memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(key, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
    AES_CMAC_SetKey(&aesCmacCtx, key);
    AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) &frame, 1 + sizeof(JOIN_ACCEPT_FRAME_HEADER));
    uint8_t mic[16];
    AES_CMAC_Final(mic, &aesCmacCtx);
    return (uint32_t) ((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
}

 /**
  * Calculate Join Response MIC OptNeg is set (version 1.1)
  * JoinReqType | JoinEUI | DevNonce | MHDR | JoinNonce | NetID | DevAddr | DLSettings |RxDelay | [CFList]
  * @see 6.2.3 Join-accept message
  * @param frame Join Accept frame
  * @param joinEUI Join EUI
  * @param devNonce Device nonce
  * @param key Key
  * @return MIC
  */
uint32_t calculateOptNegMICJoinResponse(
    const JOIN_ACCEPT_FRAME &frame,
    const DEVEUI &joinEUI,
    const DEVNONCE &devNonce,
    const KEY128 &key,
    const uint8_t rejoinType
) {
    // JoinReqType- 1, EUI- 8, DevNonce- 2, MHDR- 1, JOIN_ACCEPT_FRAME_HEADER- 12 = 24 bytes
    uint8_t d[24];
    // Join-request    0xFF, Rejoin-request type 0- 0x00,  Rejoin-request type 1- 0x01, Rejoin-request type 2- 0x02
    d[0] = rejoinType;   // JoinReqType 0xFF- join, 0, 1- rejoin
    // JoinEUI
    memmove(&(d[1]), &joinEUI, sizeof(DEVEUI)); // + 8
    // DevNonce
    memmove(&(d[9]), &devNonce, sizeof(DEVNONCE)); // + 2
    // same as OptNeg unset (version 1.0)
    memmove(&(d[10]), &frame, 1 + sizeof(JOIN_ACCEPT_FRAME_HEADER));

    aes_context aesContext;
    memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(key, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
    AES_CMAC_SetKey(&aesCmacCtx, key);
    AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) &d, 1 + sizeof(d));
    uint8_t mic[16];
    AES_CMAC_Final(mic, &aesCmacCtx);
    return (uint32_t) ((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
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
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	memmove(&name, &value.name, sizeof(DEVICENAME));
}

NetworkIdentity::NetworkIdentity
(
        const DEVICEID &value
)
{
    memset(&devaddr, 0, sizeof(DEVADDR));
    memmove(&activation, &value.activation, sizeof(activation));
    memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
    memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
    memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
    memmove(&appSKey, &value.appSKey, sizeof(KEY128));
    memmove(&name, &value.name, sizeof(DEVICENAME));
}

std::string KEY2string(
	const KEY128 &value
)
{
	return hexString(&value, sizeof(value));
}

std::string DEVNONCE2string(
    const DEVNONCE &value
)
{
    return hexString(&value, sizeof(value));
}

std::string JOINNONCE2string(
    const JOINNONCE &value
)
{
    return hexString(&value, sizeof(value));
}

DEVNONCE string2DEVNONCE(
    const std::string &value
)
{
    return (DEVNONCE) strtol(value.c_str(), NULL, 16);
}

void string2JOINNONCE(
    JOINNONCE &retval,
    const std::string &value
)
{
    uint32_t r = ntohl(strtol(value.c_str(), NULL, 16));
    retval[0] = r & 0xff;
    retval[1] = (r >> 8) & 0xff;
    retval[2] = (r >> 16) & 0xff;
}

std::string JOIN_REQUEST_FRAME2string(
    const JOIN_REQUEST_FRAME &value
) {
    return "{\"joinEUI\": \"" + DEVEUI2string(value.joinEUI) + "\", "
       + "\"devEUI\": \"" + DEVEUI2string(value.devEUI) + "\", "
       + "\"devNonce\": \"" + DEVNONCE2string(value.devNonce) + "\"}";
}

std::string CFLIST2string(const CFLIST &value)
{
    std::stringstream ss;

    ss << "{\"cflisttype\": " << (int) value.cflisttype
       << ", \"frequency\": [";
    for (int i = 0; i < 5; i++) {
        ss << FREQUENCY2int(value.frequency[i]);
        if (i < 4)
            ss << ", ";
    }
    ss << "]}";
    return ss.str();
}

std::string JOIN_ACCEPT_FRAME_HEADER2string(
    const JOIN_ACCEPT_FRAME_HEADER &value
) {
    std::stringstream ss;
    ss << "{\"joinNonce\": \"" << JOINNONCE2string(value.joinNonce)
       << "\", \"netId\": \"" << NETID2String(value.netId)
       << "\", \"devAddr\": \"" << DEVADDR2string(value.devAddr)
       << "\", \"dlSettings\": {"
       << "\"RX2DataRate\": " << (int) value.dlSettings.RX2DataRate	    ///< downlink data rate that serves to communicate with the end-device on the second receive window (RX2)
       << ", \"RX1DROffset\": " << (int) value.dlSettings.RX1DROffset         	    ///< offset between the uplink data rate and the downlink data rate used to communicate with the end-device on the first receive window (RX1)
       << ", \"optNeg\": " << (int) value.dlSettings.optNeg     	    ///< 1.0- RFU, 1.1- optNeg
       << "}, \"rxDelay\": \"" << (int) value.rxDelay
       << "\"}";
    return ss.str();
}

std::string JOIN_ACCEPT_FRAME2string(
    const JOIN_ACCEPT_FRAME &value
) {
    std::stringstream ss;
    ss << "{\"mhdr\": " << MHDR2String(value.mhdr)
        << ", \"header\": " << JOIN_ACCEPT_FRAME_HEADER2string(value.hdr)
        << ", \"mic\": \"" << MIC2String(value.mic) << "\"}";
    return ss.str();
}

std::string JOIN_ACCEPT_FRAME_CFLIST2string(
    const JOIN_ACCEPT_FRAME_CFLIST &value
) {
    std::stringstream ss;
    ss << "{\"mhdr\": " << MHDR2String(value.mhdr)
       << ", \"header\": " << JOIN_ACCEPT_FRAME_HEADER2string(value.hdr) << ", "
       << ", \"cflist\": " << CFLIST2string(value.cflist) << ", "
       << ", \"mic\": \"" << MIC2String(value.mic) << "\"}";
    return ss.str();
}

uint32_t NETID2int(
	const NETID &value
) {
	return value[0] + (value[1] << 8) + (value[2] << 16);
}

void int2NETID(
	NETID &retval,
	uint32_t value
)
{
	retval[0] = value & 0xff;
	retval[1] = (value >> 8) & 0xff;
	retval[2] = (value >> 16) & 0xff;
}

std::string NETID2String(
    const NETID &value
)
{
    uint32_t r = NETID2int(value);
    std::stringstream ss;
    ss << std::hex << std::setw(6) << std::setfill('0') << r;
    return ss.str();
}

int FREQUENCY2int(
	const FREQUENCY &frequency
) {
	return frequency[0] + (frequency[1] << 8) + (frequency[2] << 16);
}

uint32_t JOINNONCE2int(
	const JOINNONCE &value
) {
	return value[0] + (value[1] << 8) + (value[2] << 16);
}

std::string NetworkIdentity::toString() const
{
	std::stringstream ss;
	ss << DEVADDR2string(devaddr) 
		<< " " << activation2string(activation)
		<< " " << deviceclass2string(deviceclass)
		<< " " << DEVEUI2string(devEUI)
		<< " " << KEY2string(nwkSKey)
		<< " " << KEY2string(appSKey)
		<< " " << LORAWAN_VERSION2string(version)
        << " " << DEVEUI2string(appEUI)
        << " " << KEY2string(appKey)
        << " " << KEY2string(nwkKey)
        << " " << DEVNONCE2string(devNonce)
        << " " << JOINNONCE2string(joinNonce)
		<< " " << DEVICENAME2string(name);
	return ss.str();
}

std::string NetworkIdentity::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"addr\": \"" << DEVADDR2string(devaddr)
       << "\", \"activation\": \"" << activation2string(activation)
       << "\", \"deviceClass\": \"" << deviceclass2string(deviceclass)
       << "\", \"devEUI\": \"" << DEVEUI2string(devEUI)
       << "\", \"nwkSKey\": \"" << KEY2string(nwkSKey)
       << "\", \"appSKey\": \"" << KEY2string(appSKey)
       << "\", \"version\": \"" << LORAWAN_VERSION2string(version)
       << "\", \"appEUI\": \"" << DEVEUI2string(appEUI)
       << "\", \"appKey\": \"" << KEY2string(appKey)
       << "\", \"nwkKey\": \"" << KEY2string(nwkKey)
       << "\", \"nonce\": \"" << DEVNONCE2string(devNonce)
       << "\", \"joinNonce\": \"" << JOINNONCE2string(joinNonce)
       << "\", \"name\": \"" << DEVICENAME2string(name)
       << "\"}";
    return ss.str();
}

DeviceId::DeviceId() {
	memset(&devEUI, 0, sizeof(DEVEUI));
	memset(&nwkSKey, 0, sizeof(KEY128));
	memset(&appSKey, 0, sizeof(KEY128));
	memset(&name, 0, sizeof(DEVICENAME));
    memset(&appEUI, 0, sizeof(DEVEUI));
    memset(&appKey, 0, sizeof(KEY128));
    memset(&nwkKey, 0, sizeof(KEY128));
    devNonce = 0;
    memset(joinNonce, 0, sizeof(JOINNONCE));
	version.major = 1;
	version.minor = 0;
	version.release = 0;
}

DeviceId::DeviceId(
	const DeviceId &value
) {
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
    memmove(&appEUI, &value.appEUI, sizeof(DEVEUI));
    memmove(&appKey, &value.appKey, sizeof(KEY128));
    memmove(&nwkKey, &value.nwkKey, sizeof(KEY128));
    devNonce = 0;
    memset(joinNonce, 0, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
	version.major = 1;
	version.minor = 0;
	version.release = 0;
}

DeviceId::DeviceId(
	const DEVICEID &value
)
{
	set(value);
}

DeviceId& DeviceId::operator=(const DeviceId& value)
{
	if (this == &value)
		return *this;
	activation = value.activation;	///< activation type: ABP or OTAA
	deviceclass = value.deviceclass;
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	version = value.version;		///< device LoraWAN version
    memmove(&appEUI, &value.appEUI, sizeof(DEVEUI));
    memmove(&appKey, &value.appKey, sizeof(KEY128));
    memmove(&nwkKey, &value.nwkKey, sizeof(KEY128));
    devNonce = value.devNonce;
    memmove(&joinNonce, &value.joinNonce, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
	return *this;
}

DeviceId& DeviceId::operator=(const NetworkIdentity& value)
{
	activation = value.activation;	///< activation type: ABP or OTAA
	deviceclass = value.deviceclass;
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
    memmove(&appEUI, &value.appEUI, sizeof(DEVEUI));
    memmove(&appKey, &value.appKey, sizeof(KEY128));
    memmove(&nwkKey, &value.nwkKey, sizeof(KEY128));
    devNonce = value.devNonce;
    memmove(&joinNonce, &value.joinNonce, sizeof(JOINNONCE));
	version = value.version;		///< device LoraWAN version
	memmove(&name, &value.name, sizeof(DEVICENAME));
	return *this;
}

void DeviceId::set(
	const DEVICEID &value
)
{
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	memmove(&version, &value.version, sizeof(LORAWAN_VERSION));
	memmove(&appEUI, &value.appEUI, sizeof(DEVEUI));
	memmove(&appKey, &value.appKey, sizeof(KEY128));
	memmove(&nwkKey, &value.nwkKey, sizeof(KEY128));
	devNonce = value.devNonce;
	memmove(&joinNonce, &value.joinNonce, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
}

void DeviceId::setEUIString
(
	const std::string &value
)
{
	string2DEVEUI(devEUI, value);
}

void DeviceId::setNwkSKeyString
(
	const std::string &value
)
{
	string2KEY(nwkSKey, value);
}

void DeviceId::setAppSKeyString(
	const std::string &value
)
{
	string2KEY(appSKey, value);
}

void DeviceId::setName(
	const std::string &value
)
{
	string2DEVICENAME(name, value.c_str());
}

void DeviceId::setClass(
	const DEVICECLASS &value
)
{
	deviceclass = value;
}

std::string DeviceId::toJsonString() const
{
	std::stringstream ss;
	ss << "{" 
		<< "\"activation\":\"" << activation2string(activation)
		<< "\",\"class\":\"" << deviceclass2string(deviceclass)
		<< "\",\"deveui\":\"" << DEVEUI2string(devEUI)
		<< "\",\"nwkSKey\":\"" << KEY2string(nwkSKey)
		<< "\",\"appSKey\":\"" << KEY2string(appSKey)
		<< "\",\"version\":\"" << LORAWAN_VERSION2string(version)

        << "\",\"appeui\":\"" << DEVEUI2string(appEUI)
        << "\",\"appKey\":\"" << KEY2string(appKey)
        << "\",\"nwkKey\":\"" << KEY2string(nwkKey)
        << "\",\"devNonce\":\"" << DEVNONCE2string(devNonce)
        << "\",\"joinNonce\":\"" << JOINNONCE2string(joinNonce)

        << "\",\"name\":\"" << std::string(name, sizeof(DEVICENAME))
		<< "\"}";
	return ss.str();
}

void DeviceId::setProperties
(
	std::map<std::string, std::string> &retval
)
{
	retval["activation"] = activation2string(activation);
	retval["class"] = deviceclass2string(deviceclass);
	retval["deveui"] = DEVEUI2string(devEUI);
    retval["appeui"] = DEVEUI2string(appEUI);
    retval["appKey"] = KEY2string(appKey);
    retval["nwkKey"] = KEY2string(nwkKey);
    retval["devNonce"] = DEVNONCE2string(devNonce);
    retval["joinNonce"] = JOINNONCE2string(joinNonce);
	retval["name"] = DEVICENAME2string(name);
	retval["version"] = LORAWAN_VERSION2string(version);
}

void NetworkIdentity::set(
	const DEVADDRINT &addr,
	const DEVICEID &value
)
{
	memmove(&devaddr, &addr.a, sizeof(DEVADDR));
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	memmove(&appEUI, &value.appEUI, sizeof(DEVEUI));
	memmove(&appKey, &value.appKey, sizeof(KEY128));
	memmove(&nwkKey, &value.nwkKey, sizeof(KEY128));
	devNonce = value.devNonce;
	memmove(&joinNonce, &value.joinNonce, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
	memmove(&version, &value.version, sizeof(LORAWAN_VERSION));
}

void NetworkIdentity::set(
	const DEVADDR &addr,
	const DeviceId &value
)
{
	memmove(&devaddr, &addr, sizeof(DEVADDR));
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey, &value.appSKey, sizeof(KEY128));
	memmove(&appEUI, &value.appEUI, sizeof(DEVEUI));
	memmove(&appKey, &value.appKey, sizeof(KEY128));
	memmove(&nwkKey, &value.nwkKey, sizeof(KEY128));
	devNonce = value.devNonce;
	memmove(&joinNonce, &value.joinNonce, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
	memmove(&version, &value.version, sizeof(LORAWAN_VERSION));
}

// const std::string DEF_DATA_RATE = "SF7BW125";
// const std::string DEF_ECCCODE_RATE = "4/6";
#define DEF_BANDWIDTH BANDWIDTH_INDEX_250KHZ
#define DEF_SPREADING_FACTOR DRLORA_SF7
#define DEF_CODING_RATE CRLORA_4_6

#define DEF_RSSI	-35
#define DEF_LSNR	5.1f

rfmMetaData::rfmMetaData() 
	: gatewayId(0), chan(0), rfch(0), freq(868900000), stat(0), 
	modu(LORA), bandwith(DEF_BANDWIDTH), spreadingFactor(DEF_SPREADING_FACTOR),
 	codingRate(DEF_CODING_RATE), bps(0), rssi(DEF_RSSI), lsnr(DEF_LSNR)
{
	time(&t);			// UTC time of pkt RX, us precision, ISO 8601 'compact' format
}

/**
 * @return  LoRa datarate identifier e.g. "SF7BW125"
 */
std::string rfmMetaData::datr() const
{
	int bandwithValue;
	switch (bandwith) {
		case BANDWIDTH_INDEX_7KHZ:
			bandwithValue = 7; // 7.8
			break;
		case BANDWIDTH_INDEX_10KHZ:
			bandwithValue = 10; // 10.4
			break;
		case BANDWIDTH_INDEX_15KHZ:
			bandwithValue = 15; // 15.6
			break;
		case BANDWIDTH_INDEX_20KHZ:
			bandwithValue = 20; // 20.8
			break;
		case BANDWIDTH_INDEX_31KHZ:
			bandwithValue = 31; // 31.2
			break;
		case BANDWIDTH_INDEX_41KHZ:
			bandwithValue = 41; // 41.6
			break;
		case BANDWIDTH_INDEX_62KHZ:
			bandwithValue = 62; // 62.5
			break;
		case BANDWIDTH_INDEX_125KHZ:
			bandwithValue = 125; // 125
			break;
		case BANDWIDTH_INDEX_250KHZ:
			bandwithValue = 250; // 250
			break;
		case BANDWIDTH_INDEX_500KHZ:
			bandwithValue = 500; // 500
			break;
		default:
			bandwithValue = 250;
			break;
	}
	std::stringstream ss;
    // e.g. SF7BW203
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
			bandwith = BANDWIDTH_INDEX_7KHZ; // 7.8
			break;
		case 10:
			bandwith = BANDWIDTH_INDEX_10KHZ; // 10.4
			break;
		case 15:
			bandwith = BANDWIDTH_INDEX_15KHZ; // 15.6
			break;
		case 20:
			bandwith = BANDWIDTH_INDEX_20KHZ; // 20.8
			break;
		case 31:
			bandwith = BANDWIDTH_INDEX_31KHZ; // 31.2
			break;
		case 41:
			bandwith = BANDWIDTH_INDEX_41KHZ; // 41.6
			break;
		case 62:
			bandwith = BANDWIDTH_INDEX_62KHZ; // 62.5
			break;
		case 125:
			bandwith = BANDWIDTH_INDEX_125KHZ; // 125
			break;
		case 250:
			bandwith = BANDWIDTH_INDEX_250KHZ;
			break;
		case 500:
			bandwith = BANDWIDTH_INDEX_500KHZ;
			break;
		default:
			bandwith = BANDWIDTH_INDEX_250KHZ;
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
	: gatewayId(value.gatewayId), t(value.t), tmst(value.tmst), chan(value.chan), rfch(value.rfch), freq(value.freq), stat(value.stat), 
	modu(value.modu), bandwith(value.bandwith),
	spreadingFactor(value.spreadingFactor), codingRate(value.codingRate), 
	bps(value.bps), rssi(value.rssi), lsnr(value.lsnr)
{
}

// copy gateway address from prefix
rfmMetaData::rfmMetaData(
	const SEMTECH_PREFIX_GW *aprefix,
	const rfmMetaData &value
)
	: t(value.t), tmst(value.tmst), chan(value.chan), rfch(value.rfch), freq(value.freq), stat(value.stat), 
	modu(value.modu), bandwith(value.bandwith),
	spreadingFactor(value.spreadingFactor), codingRate(value.codingRate), 
	bps(value.bps), rssi(value.rssi), lsnr(value.lsnr)
{
	if (aprefix) 
		gatewayId = *(uint64_t *) &aprefix->mac;
}

rfmMetaData::rfmMetaData(
    const SEMTECH_PREFIX_GW &aPrefix,
    const SEMTECH_PROTOCOL_METADATA *m
)
{
    gatewayId = deveui2int(aPrefix.mac);
    if (m) {
        t = m->t;
        tmst = m->tmst;
        chan = m->chan;
        rfch = m->rfch;
        freq = m->freq;
        stat = m->stat;
        modu = m->modu;
        bandwith = m->bandwith;
        spreadingFactor = m->spreadingFactor;
        codingRate = m->codingRate;
        bps = m->bps;
        rssi = m->rssi;
        lsnr = m->lsnr;
    }
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
    return MODULATION2String(modu);
}

void rfmMetaData::setModulation(
	const char *value
) {
    modu = string2MODULATION(value);
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
	int n = (int) lsnr;
	int m = (int) (lsnr * 10.0 - n * 10);
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

/**
 * 	Section 6
 */
const char* METADATA_TX_NAMES[16] = {
	"txpk",	// 0 array name
	"imme", // 1 bool     Send packet immediately (will ignore tmst & time)
	"tmst", // 2 number   Send packet on a certain timestamp value (will ignore time)
	"tmms", // 3 number   Send packet at a certain GPS time (GPS synchronization required)
	"freq", // 4 number   TX central frequency in MHz (unsigned float, Hz precision)
	"rfch", // 5 number   Concentrator "RF chain" used for TX (unsigned integer)
	"powe", // 6 number   TX output power in dBm (unsigned integer, dBm precision)
	"modu", // 7 string   Modulation identifier "LORA" or "FSK"
	"datr", // 8 string or number  LoRa data rate identifier (eg. SF12BW500) or FSK datarate (unsigned, in bits per second)
	"codr", // 9 string  LoRa ECC coding rate identifier
	"fdev", // 10 number FSK frequency deviation (unsigned integer, in Hz) 
	"ipol", // 11 bool   Lora modulation polarization inversion
	"prea", // 12 number RF preamble size (unsigned integer)
	"size", // 13 number RF packet payload size in bytes (unsigned integer)
	"data", // 14 string Base64 encoded RF packet payload, padded
	"ncrc", // 15 bool   If true, disable the CRC of the physical layer (optional)
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
	size_t len = str.size();
	if (len > sizeof(DEVADDR))
		len = sizeof(DEVADDR);
	memmove(&retval, str.c_str(), len);
	if (len < sizeof(DEVADDR))
		memset(&retval + len, 0, sizeof(DEVADDR) - len);
	*((uint32_t*) &retval) = NTOH4(*((uint32_t*) &retval));
}

void string2DEVEUI(
	DEVEUI &retval,
	const std::string &value
)
{
	std::string str = hex2string(value);
	size_t len = str.size();
	if (len > sizeof(DEVEUI))
		len = sizeof(DEVEUI);
	memmove(&retval, str.c_str(), len);
	if (len < sizeof(DEVEUI))
		memset(&retval + len, 0, sizeof(DEVEUI) - len);
	*((uint64_t*) &retval) = NTOH8(*((uint64_t*) &retval));
}

void string2KEY(
	KEY128 &retval,
	const std::string &str
)
{
	size_t len = str.size();
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

std::string DEVICENAME2string
(
	const DEVICENAME &value
)
{
	size_t sz = strnlen(value, sizeof(DEVICENAME));
	return std::string(&value[0], sz);
}

void int2DEVADDR(
	DEVADDR &retval,
	uint32_t value
)
{
	// *((uint32_t*) &retval) = NTOH4(value);
	*((uint32_t*) &retval) = value;
}

std::string MHDR2String(const MHDR &value)
{
    // return hexString(&value, 1);
    std::stringstream ss;
    ss << "{\"mtype\": " << (int) value.f.mtype
        << ", \"major\": " << (int) value.f.major
        << ", \"rfu\": " << (int) value.f.rfu
        << "}";
    return ss.str();
}

std::string MIC2String(uint16_t value)
{
    // hex string is MSB first, swap if need it
    value = NTOH2(value);
    return hexString(&value, sizeof(value));
}

std::string DEVADDR2string(
	const DEVADDR &value
)
{
	uint32_t v;
	memmove(&v, &value, sizeof(v));
	// hex string is MSB first, swap if need it
	v = NTOH4(v);
	return hexString(&v, sizeof(v));
}

std::string uint64_t2string(
	const uint64_t &value
) {
	uint64_t v;
	memmove(&v, &value, sizeof(v));
	// hex string is MSB first, swap if need it
	v = NTOH8(v);
	return hexString(&v, sizeof(v));
}

std::string DEVADDRINT2string(
	const DEVADDRINT &value
)
{
	DEVADDRINT v;
	memmove(&v.a, &value.a, sizeof(DEVADDR));
	v.a = NTOH4(*((uint32_t*) &v.a));
	return hexString(&v, sizeof(DEVADDR));
}

std::string DEVEUI2string(
	const DEVEUI &value
)
{
	// EUI stored in memory as 8-bit integer x86 LSB first, ARM MSB first
	uint64_t v;
	memmove(&v, &value, sizeof(DEVEUI));
	// hex string is MSB first, swap if need it
	v = NTOH8(v);
	return hexString(&v, sizeof(v));
}

void rfmMetaData::toJSON(
	rapidjson::Value &value,
	rapidjson::Document::AllocatorType& allocator,
	const std::string &data
) {
	int ms = -1;
	std::string dt = ltimeString(t, ms, "%FT%T") + "Z";	// "2020-12-16T12:17:00.12345Z";
	rapidjson::Value v1(rapidjson::kStringType);
	v1.SetString(dt.c_str(), (rapidjson::SizeType) dt.size());
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
	v8.SetString(s8.c_str(), (rapidjson::SizeType) s8.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[8])), v8, allocator);

	rapidjson::Value v9(rapidjson::kStringType);
	std::string dr = datr();
	v9.SetString(dr.c_str(), (rapidjson::SizeType) dr.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[9])), v9, allocator);

	rapidjson::Value v10(rapidjson::kStringType);
	std::string cr = codr();
	v10.SetString(cr.c_str(), (rapidjson::SizeType) cr.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[10])), v10, allocator);

	rapidjson::Value v11(rssi);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[11])), v11, allocator);

	rapidjson::Value v12(lsnr);
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[12])), v12, allocator);

	rapidjson::Value v13(data.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[13])), v13, allocator);

	rapidjson::Value v14(rapidjson::kStringType);
	std::string d(base64_encode(data));	// base64

	v14.SetString(d.c_str(), (rapidjson::SizeType) d.size());
	value.AddMember(rapidjson::Value(rapidjson::StringRef(METADATA_RX_NAMES[14])), v14, allocator);
}

/**
 * Parse RXPK
 * @param retSize
 * @param retData
 * @param value
 * @return
 */
int rfmMetaData::parseRX(
	int &retSize,
	std::string &retData,
	rapidjson::Value &value
) {
    if (value.HasMember(METADATA_RX_NAMES[1])) {    // time UTC time of pkt RX, us precision, ISO 8601 'compact' format
		rapidjson::Value &v = value[METADATA_RX_NAMES[1]];
		if (v.IsString()) {
			t = parseDate(v.GetString());
		}
	}
    // skip tmms GPS time of pkt RX, number of milliseconds since 06.Jan.1980
	if (value.HasMember(METADATA_RX_NAMES[3])) {    // tmst Internal timestamp of "RX finished" event (32b unsigned)
		rapidjson::Value &v = value[METADATA_RX_NAMES[3]];
		if (v.IsUint()) {
			tmst = v.GetUint();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[4])) {    // freq RX central frequency in MHz (unsigned float, Hz precision)
        rapidjson::Value &v = value[METADATA_RX_NAMES[4]];
		if (v.IsDouble()) {
			freq = (uint32_t) v.GetDouble() * 1000000;
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[5])) {    // chan Concentrator "IF" channel used for RX (unsigned integer
        rapidjson::Value &v = value[METADATA_RX_NAMES[5]];
		if (v.IsInt()) {
			chan = (uint8_t) v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[6])) {    // rfch Concentrator "RF chain" used for RX (unsigned integer)
        rapidjson::Value &v = value[METADATA_RX_NAMES[6]];
		if (v.IsInt()) {
			rfch = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[7])) {    // stat CRC status: 1 = OK, -1 = fail, 0 = no CRC
        rapidjson::Value &v = value[METADATA_RX_NAMES[7]];
		if (v.IsInt()) {
			stat = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[8])) {    // modu Modulation identifier "LORA" or "FSK"
        rapidjson::Value &v = value[METADATA_RX_NAMES[8]];
		if (v.IsString()) {
			setModulation(v.GetString());
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[9])) {    // datr LoRa data rate identifier (eg. SF12BW500) or FSK datarate (unsigned, in bits per second)
        rapidjson::Value &v = value[METADATA_RX_NAMES[9]];
		if (v.IsString()) {
			setDatr(v.GetString());
		}
	}
	
	if (value.HasMember(METADATA_RX_NAMES[10])) {   // codr LoRa ECC coding rate identifier
        rapidjson::Value &v = value[METADATA_RX_NAMES[10]];
		if (v.IsString()) {
			setCodr(v.GetString());
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[11])) {   // rssi RSSI in dBm (signed integer, 1 dB precision)
        rapidjson::Value &v = value[METADATA_RX_NAMES[11]];
		if (v.IsInt()) {
			rssi = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[12])) {   // lsnr Lora SNR ratio in dB (signed float, 0.1 dB precision)
        rapidjson::Value &v = value[METADATA_RX_NAMES[12]];
		if (v.IsFloat()) {
			lsnr = v.GetFloat();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[13])) {   // size RF packet payload size in bytes (unsigned integer
        rapidjson::Value &v = value[METADATA_RX_NAMES[13]];
		if (v.IsInt()) {
			retSize = v.GetInt();
		}
	}

	if (value.HasMember(METADATA_RX_NAMES[14])) {   // data Base64 encoded RF packet payload, padded
        rapidjson::Value &v = value[METADATA_RX_NAMES[14]];
		if (v.IsString()) {
			try {
				// base64
				retData = base64_decode(std::string(v.GetString()), false);
			} catch (const std::exception&) {
				retData = "";
			}

		}
	}
	return 0;
}

/**
 * Parse TXPK
 * @param retSize
 * @param retData
 * @param value
 *
 *         "imme", // 1 bool     Send packet immediately (will ignore tmst & time)
            "tmms", // 3 number   Send packet at a certain GPS time (GPS synchronization required)
            "powe", // 6 number   TX output power in dBm (unsigned integer, dBm precision)

            "fdev", // 10 number FSK frequency deviation (unsigned integer, in Hz)
            "ipol", // 11 bool   Lora modulation polarization inversion
            "prea", // 12 number RF preamble size (unsigned integer)
            "ncrc", // 15 bool   If true, disable the CRC of the physical layer (optional)

 * @return
 */
int rfmMetaData::parseTX(
        int &retSize,
        std::string &retData,
        rapidjson::Value &value
) {
    // "tmst" 2 number   Send packet on a certain timestamp value (will ignore time)
    if (value.HasMember(METADATA_TX_NAMES[2])) {
        rapidjson::Value &v = value[METADATA_TX_NAMES[2]];
        if (v.IsUint()) {
            tmst = v.GetUint();
        }
    }

    // "freq" 4 number   TX central frequency in MHz (unsigned float, Hz precision)
    if (value.HasMember(METADATA_TX_NAMES[4])) {
        rapidjson::Value &v = value[METADATA_TX_NAMES[4]];
        if (v.IsDouble()) {
            freq = (uint32_t) (v.GetDouble() * 1000000);
        }
    }

    // "rfch" 5 number   Concentrator "RF chain" used for TX (unsigned integer)
    if (value.HasMember(METADATA_TX_NAMES[6])) {
        rapidjson::Value &v = value[METADATA_TX_NAMES[6]];
        if (v.IsInt()) {
            rfch = v.GetInt();
        }
    }

    // "datr" 8 string or number  LoRa data rate identifier (eg. SF12BW500) or FSK datarate (unsigned, in bits per second)
    if (value.HasMember(METADATA_TX_NAMES[8])) {
        rapidjson::Value &v = value[METADATA_TX_NAMES[8]];
        if (v.IsString()) {
            setDatr(v.GetString());
        }
    }

    // "codr" 9 string  LoRa ECC coding rate identifier
    if (value.HasMember(METADATA_TX_NAMES[9])) {
        rapidjson::Value &v = value[METADATA_TX_NAMES[9]];
        if (v.IsString()) {
            setCodr(v.GetString());
        }
    }

    // "modu" 7 string   Modulation identifier "LORA" or "FSK"
    if (value.HasMember(METADATA_TX_NAMES[7])) {
        rapidjson::Value &v = value[METADATA_TX_NAMES[7]];
        if (v.IsString()) {
            setModulation(v.GetString());
        }
    }

    // "size" 13 number RF packet payload size in bytes (unsigned integer)
    if (value.HasMember(METADATA_TX_NAMES[13])) {
        rapidjson::Value &v = value[METADATA_TX_NAMES[13]];
        if (v.IsInt()) {
            retSize = v.GetInt();
        }
    }

    // "data" 14 string Base64 encoded RF packet payload, padded
    if (value.HasMember(METADATA_TX_NAMES[14])) {
        rapidjson::Value &v = value[METADATA_TX_NAMES[14]];
        if (v.IsString()) {
            try {
                // base64
                retData = base64_decode(std::string(v.GetString()));
            } catch (const std::exception&) {
                retData = "";
            }
        }
    }
    return 0;
}

std::string rfmMetaData::toJsonString(
	const std::string &payload
) const
{
	std::stringstream ss;
	int ms = -1;
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
		<< ",\"" << METADATA_RX_NAMES[13] << "\":" << payload.size()
		<< ",\"" << METADATA_RX_NAMES[14] << "\":\"" << base64_encode(payload) << "\"}";
	return ss.str();
}

RFMHeader::RFMHeader() {
	memset(&header, 0, sizeof(RFM_HEADER));
    memset(&fopts, 0, sizeof(FOPTS));
    fport = 0;
}

RFMHeader::RFMHeader(
        const RFMHeader &value
) {
    memmove(&header, &value.header, sizeof(RFM_HEADER));
    memmove(&fopts, &value.fopts, sizeof(FOPTS));
    fport = value.fport;
}

RFMHeader::RFMHeader(
	const RFM_HEADER &hdr
) {
	memmove(&header, &hdr, sizeof(RFM_HEADER));
    fport = 0;
}

RFMHeader::RFMHeader(
	const DEVADDR &addr
) {
	memset(&header, 0, sizeof(RFM_HEADER));
	memcpy(&header.devaddr, &addr, sizeof(DEVADDR));
	header.macheader.i = 0x40;
    fport = 0;
}

RFMHeader::RFMHeader(
	const DEVADDR &addr,
	uint16_t fcnt
) {
	memcpy(&header.devaddr, &addr, sizeof(DEVADDR));
	header.fcnt = fcnt;
	fport = 0;
	header.fctrl.i = 0;
	header.macheader.i = 0x40;
}

RFMHeader::RFMHeader(
	const DEVADDR &addr,
	uint16_t frameCounter,
	uint8_t framePort
) {
	header.macheader.i = 0x40;
	memcpy(&header.devaddr, &addr, sizeof(DEVADDR));
	header.fcnt = frameCounter;
	fport = framePort;
	header.fctrl.i = 0;
}

RFMHeader::RFMHeader(
	const std::string &value
) {
	parse(value);
}

bool RFMHeader::parse(
	const std::string &value
) 
{
	size_t sz = sizeof(RFM_HEADER);
	bool r = sz <= value.size();
	if (!r)
		sz = value.size();
	if (sz > 0)
		memcpy(&header, value.c_str(), sz);
	// fopts, less than 15 bytes
	uint8_t foptslen = header.fctrl.f.foptslen;
	if (foptslen) {
		if (value.size() >= sizeof(RFM_HEADER) + foptslen) {
			memcpy(&fopts, value.c_str() + sizeof(RFM_HEADER), foptslen);	
		}
	}
	if (value.size() > sizeof(RFM_HEADER) + foptslen)
		memcpy(&fport, value.c_str() + sizeof(RFM_HEADER) + foptslen, 1);
	return r;
}

/**
 * Serialize MHDR + FHDR
 */ 
std::string RFMHeader::toBinary() const {
	RFM_HEADER h(header);
	// *((uint32_t*) &h.devaddr) = NTOH4(*((uint32_t *) &header.devaddr));
	// h.fcnt = NTOH2(header.fcnt);
	return std::string((const char *) &h, sizeof(RFM_HEADER));
}

std::string RFMHeader::toJson() const
{
	std::stringstream ss;
	ss << "{\"fport\": "  << (int) fport
		<< ", \"fopts\": \""  << opts2string(header.fctrl.f.foptslen, fopts)
		<< "\", \"header\": {\"fcnt\": "  << header.fcnt
		<< ", \"fctrl\": {\"foptslen\": " << (int) header.fctrl.f.foptslen
		<< ", \"fpending\": " << (int)  header.fctrl.f.fpending
		<< ", \"ack\": " << (int)  header.fctrl.f.ack
		<< ", \"adr\": " << (int)  header.fctrl.f.adr
		<< "}, \"addr\": \"" << DEVADDR2string(header.devaddr)
		<< "\", \"mac\": {\"major\": " << (int) header.macheader.f.major
		<< ", \"mtype\": \"" << mtype2string((MTYPE) header.macheader.f.mtype)
		<< "\"}}}";
	return ss.str();
}

SemtechUDPPacket::SemtechUDPPacket()
	: errcode(0), downlink(false)
{
	prefix.version = 2;
	prefix.token = 0;
	prefix.tag = 0;

	header.header.macheader.i = 0x40;
	memset(&header.header.devaddr, 0, sizeof(DEVADDR));			// MAC address

	header.header.fctrl.i = 0;
	header.header.fcnt = 0;
	header.fport = 0;

	memset(&header.header.devaddr, 0, sizeof(DEVADDR));
	memset(&prefix.mac, 0, sizeof(prefix.mac));
}

SemtechUDPPacket::SemtechUDPPacket(const SemtechUDPPacket &value)
{
    payload = value.payload;
    header = value.header;
    metadata  = value.metadata;
    memmove(&gatewayAddress, &value.gatewayAddress, sizeof(struct sockaddr_in6));
    errcode = value.errcode;
    downlink = value.downlink;
    memmove(&prefix, &value.prefix, sizeof(SEMTECH_PREFIX_GW));
    devId = value.devId;
}

char *SemtechUDPPacket::getSemtechJSONCharPtr
(
	const void *packet,
	size_t size
)
{
	if (size <= sizeof(SEMTECH_PREFIX_GW))
		return NULL;
    switch (((SEMTECH_PREFIX_GW *) packet)->tag) {
        case SEMTECH_GW_PUSH_DATA:  // 0 network server responds on PUSH_DATA to acknowledge immediately all the PUSH_DATA packets received
            return (char *) packet + sizeof(SEMTECH_PREFIX_GW); // 12 bytes
        case SEMTECH_GW_PUSH_ACK:   // 1 gateway initiate receiving packets from the network server (because of NAT)
            return (char *) packet + sizeof(SEMTECH_PREFIX); // 4 bytes
        case SEMTECH_GW_PULL_DATA:  // 2 network server responds on PULL_DATA
            return (char *) packet + sizeof(SEMTECH_PREFIX_GW); // 12 bytes
        case SEMTECH_GW_PULL_ACK:   // 3 network server send packet to the gateway after PULL_DATA - PULL_ACK sequence
            return (char *) packet + sizeof(SEMTECH_PREFIX); // 4 bytes
        case SEMTECH_GW_PULL_RESP:  // 4
            return (char *) packet + sizeof(SEMTECH_PREFIX); // 4 bytes
        case SEMTECH_GW_TX_ACK:  // 5 gateway inform network server about does PULL_RESP data transmission was successful or not
            return (char *) packet + sizeof(SEMTECH_PREFIX_GW); // 12 bytes
        default:
            return NULL;
    }

}

/**
 * Parse Semtech UDP packet gateway prefix
 * @return 0, ERR_CODE_PACKET_TOO_SHORT, ERR_CODE_INVALID_PROTOCOL_VERSION
 */ 
int SemtechUDPPacket::parsePrefixGw
(
	SEMTECH_PREFIX_GW &retprefix,
	const void *packetForwarderPacket,
	int size
)
{
	if (size < sizeof(SEMTECH_PREFIX_GW))
		return ERR_CODE_PACKET_TOO_SHORT;
	memmove(&retprefix, packetForwarderPacket, sizeof(SEMTECH_PREFIX_GW));
	*(uint64_t *) &(retprefix.mac) = NTOH8(*(uint64_t *) &(retprefix.mac));
	// check version
	if (retprefix.version != 2)
		return ERR_CODE_INVALID_PROTOCOL_VERSION;
	return LORA_OK;
}

/**
 * Parse Semtech UDP packet
 * @return 0, ERR_CODE_PACKET_TOO_SHORT, ERR_CODE_INVALID_PROTOCOL_VERSION, ERR_CODE_NO_GATEWAY_STAT, ERR_CODE_INVALID_PACKET or ERR_CODE_INVALID_JSON
 */ 
int SemtechUDPPacket::parse
(
	const struct sockaddr *gwAddress,
	SEMTECH_PREFIX_GW &retPrefix,
	GatewayStat &retGWStat,
	std::vector<SemtechUDPPacket> &retPackets,
	const void *packetForwarderPacket,
	int size,
	IdentityService *identityService
) {
    retGWStat.errcode = ERR_CODE_NO_GATEWAY_STAT;

	int r = parsePrefixGw(retPrefix, packetForwarderPacket, size);
	if (r)
		return r;
	char *json = getSemtechJSONCharPtr(packetForwarderPacket, size);
	if (size == sizeof(SEMTECH_PREFIX_GW)) {
		if (retPrefix.tag == 2)
			return ERR_CODE_PULLOUT;
		else
			return ERR_CODE_PING;	// remove it, nothing to do
	}

	rapidjson::Document doc;
	rapidjson::Document::AllocatorType &allocator(doc.GetAllocator());
	doc.Parse<rapidjson::kParseCommentsFlag>(json);
	if (!doc.IsObject())
		return ERR_CODE_INVALID_JSON;

	// rapidjson::StringRef(METADATA_RX_NAMES[1]))

	if (doc.HasMember(METADATA_RX_NAMES[7])) {	// "stat"
		rapidjson::Value &jstat = doc[METADATA_RX_NAMES[7]];
		if (retGWStat.parse(jstat) == 0) {
			// setValue gateway identifier
			retGWStat.gatewayId = *(uint64_t *) &retPrefix.mac;
            retGWStat.errcode = 0;
		} else {
			return ERR_CODE_INVALID_STAT;
		}
	}

	if (!doc.HasMember(METADATA_RX_NAMES[0])) // check rxpk
    {
        // is it txpk?
        if (doc.HasMember(METADATA_TX_NAMES[0])) {
            rapidjson::Value &jm = doc[METADATA_TX_NAMES[0]];
            if (!jm.IsObject())
                return ERR_CODE_INVALID_JSON;
            rfmMetaData m;
            int sz;
            std::string data;
            // extract txpk.data
            int rr = m.parseTX(sz, data, jm);
            if (rr)
                return rr;
            SemtechUDPPacket packet(gwAddress, &retPrefix, &m, data, identityService);

            if ((packet.errcode == LORA_OK) || (packet.errcode == ERR_CODE_IS_JOIN))
                retPackets.push_back(packet);
            r = packet.errcode;

            return r;
        }
        return 0;    // that's ok, it is a PING
    }

    // rxpk
	rapidjson::Value &rxpk = doc[METADATA_RX_NAMES[0]];
	if (!rxpk.IsArray())
		return ERR_CODE_INVALID_JSON;

	for (rapidjson::SizeType i = 0; i < rxpk.Size(); i++) {
		rapidjson::Value &jm = rxpk[i];
		if (!jm.IsObject())
			return ERR_CODE_INVALID_JSON;
		rfmMetaData m;
		int sz;
		std::string data;
        // extract rxpk.data
		int rr = m.parseRX(sz, data, jm);
		if (rr)
			return rr;
		SemtechUDPPacket packet(gwAddress, &retPrefix, &m, data, identityService);
		if ((packet.errcode == LORA_OK) || (packet.errcode == ERR_CODE_IS_JOIN))
			retPackets.push_back(packet);
        r = packet.errcode;
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
	int r = 0;
	if (!*value)
		return r;
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
		*(uint32_t*) retval = NTOH4(*(uint32_t *) value.c_str());
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
		*(uint64_t*) retval = NTOH8(*(uint64_t *) value.c_str());
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
SemtechUDPPacket::SemtechUDPPacket(
	const struct sockaddr *gwAddress,
	const std::string &data,
	const std::string &devaddr,
	const std::string &appskey
)
	: errcode(0), downlink(false)
{
	clearPrefix();

	// initialize header ?!!
	memset(&header.header, 0, sizeof(RFM_HEADER));
	header.header.macheader.i = 0x40;
	setAddr(header.header.devaddr, devaddr);

	// autentication keys
	setKey(devId.appSKey, appskey);

	// save gateway address
	size_t sz = 0;
	if (gwAddress) {
		if (gwAddress->sa_family == AF_INET6)
			sz = sizeof(struct sockaddr_in6 *);
		if (gwAddress->sa_family == AF_INET)
			sz = sizeof(struct sockaddr_in *);
		memmove(&gatewayAddress, gwAddress, sz);
	}

	parseData(data, nullptr);
}

void SemtechUDPPacket::clearPrefix()
{
	prefix.version = 2;
	prefix.token = 0;
	prefix.tag = 0;
	memset(&prefix.mac, 0, sizeof(prefix.mac));
}

SemtechUDPPacket::SemtechUDPPacket(
    const SEMTECH_PREFIX_GW &aPrefix,
    const SEMTECH_PROTOCOL_METADATA *aMetadata,
    const std::string &payload,
    IdentityService *identityService
)
    : downlink(false)
{
    gatewayAddress.sin6_family = PF_UNSPEC;
    memmove(&prefix, &aPrefix, sizeof(SEMTECH_PREFIX_GW));
    metadata.push_back(rfmMetaData(prefix, aMetadata));
    errcode = parseData(payload, identityService);
}

SemtechUDPPacket::SemtechUDPPacket(
	const struct sockaddr *gwAddress,
	const SEMTECH_PREFIX_GW *aprefix,
	const rfmMetaData *ametadata,
	const std::string &data,
	IdentityService *identityService
)
	: errcode(0), downlink(false)
{
	if (aprefix)
		memmove(&prefix, aprefix, sizeof(SEMTECH_PREFIX_GW));
	else {
		clearPrefix();
	}
	if (ametadata)
		metadata.push_back(rfmMetaData(aprefix, *ametadata));

	// save gateway address
	size_t sz = 0;
	if (gwAddress) {
		if (gwAddress->sa_family == AF_INET6)
			sz = sizeof(struct sockaddr_in6 *);
		if (gwAddress->sa_family == AF_INET)
			sz = sizeof(struct sockaddr_in *);
		memmove(&gatewayAddress, gwAddress, sz);
	}

    errcode = parseData(data, identityService);
}

static std::string getMAC(
	const DEVEUI &value
) {
	return hexString(&value, sizeof(DEVEUI));
}

const RFM_HEADER *SemtechUDPPacket::getRfmHeader() const
{
	return (const RFM_HEADER *) &header.header;
}

RFMHeader *SemtechUDPPacket::getHeader() {
	return &header;
}

void SemtechUDPPacket::setRfmHeader(
	const RFM_HEADER &value
) {
	header = RFMHeader(value);
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
std::string SemtechUDPPacket::serialize2RfmPacket() const
{
	std::stringstream ss;
	std::string p(payload);

	// direction of frame is up
	unsigned char direction = downlink ? 1 : 0;

	// build radio packet, unconfirmed data up macHeader.i = 0x40;
	// RFM header 8 bytes: MHDR + FHDR
	ss << header.toBinary() << header.fport;

	// load data
	// encrypt data
	encryptPayload(p, header.header.fcnt, direction, header.header.devaddr, devId.appSKey);
	ss << p;

	std::string rs = ss.str();
	// calc MIC
	const KEY128 *key = &devId.nwkSKey;
	uint32_t mic = calculateMIC((const unsigned char*) rs.c_str(),
        (unsigned char ) rs.size(), header.header.fcnt,
        direction, header.header.devaddr, *key);
	// load MIC in package
	// mic = NTOH4(mic);
	ss << std::string((char *) &mic, 4);
	return ss.str();
}

std::string SemtechUDPPacket::toString() const
{
	std::stringstream ss;
	// prefix 12 bytes, metadata + payload
	ss << std::string((const char *) &prefix, sizeof(SEMTECH_PREFIX_GW))
		<< metadataToJsonString();
	return ss.str();
}

std::string SemtechUDPPacket::toDebugString() const
{
	std::stringstream ss;
	ss << "device " << DEVICENAME2string(devId.name)
		<< ", addr: " << getDeviceAddrStr()
		<< ", gw: " << DEVEUI2string(prefix.mac);
	if (metadata.size() > 0) {
		ss << ", rssi: " << (int) metadata[0].rssi << "dBm, lsnr: "
			<<  metadata[0].lsnr << "dB";
	}
	return ss.str();
}

std::string SemtechUDPPacket::metadataToJsonString() const
{
	std::string d(serialize2RfmPacket());
	std::stringstream ss;
	ss << "{\"" << METADATA_RX_NAMES[0] << "\":[";  // rxpk
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

void SemtechUDPPacket::setGatewayId(
	const std::string &value
) {
	setMAC(prefix.mac, value);
}

void SemtechUDPPacket::setGatewayId(
    uint64_t value
) {
    uint64_t val = HTON8(value);
    memmove(prefix.mac, &val, sizeof(DEVEUI));
}

std::string SemtechUDPPacket::getDeviceEUI() const
{
	return DEVEUI2string(devId.devEUI);
}

void SemtechUDPPacket::setDeviceEUI(
	const std::string &value
) {
	string2DEVEUI(devId.devEUI, value);
}

/**
 * 0- strongest, -120- nothing
 * @return INT16_MIN if N/A
 */
int16_t SemtechUDPPacket::getStrongesSignalLevel(int &idx) const {
	int16_t r = INT16_MIN;
	idx = -1;
	for (int i = 0; i < metadata.size(); i++) {
		if (metadata[0].rssi > r) {
			r = metadata[0].rssi;
			idx = i;
		}
	}
	return r;
}

std::string SemtechUDPPacket::getDeviceAddrStr() const
{
	return DEVADDR2string(header.header.devaddr);
}

DEVADDRINT SemtechUDPPacket::getDeviceAddr() const {
	DEVADDRINT r = static_cast<DEVADDRINT>(header.header.devaddr);
	return r;
}

void SemtechUDPPacket::getDeviceAddr(
	DEVADDR &retval
) const
{
	memmove(&retval, header.header.devaddr, sizeof(DEVADDR));
}

void SemtechUDPPacket::setDeviceAddr(
	const std::string &value
) {
	setAddr(header.header.devaddr, value);
}

void SemtechUDPPacket::setNetworkSessionKey(
	const std::string &value
) {
	setKey(devId.nwkSKey, value);
}

void SemtechUDPPacket::setApplicationSessionKey(
	const std::string &value
) {
	setKey(devId.appSKey, value);
}

void SemtechUDPPacket::setFrameCounter(
	uint16_t value
)
{
	header.header.fcnt = value;
}

void SemtechUDPPacket::setFOpts
(
	const std::string &value
)
{
	size_t sz = value.size();
	if (sz > 15)
	{
		header.header.fctrl.f.foptslen = 0;
		header.fport = 0;
		payload = value;
	} else {
		header.header.fctrl.f.foptslen = sz;
		memmove(&header.fopts, value.c_str(), sz);
	}
}

std::string SemtechUDPPacket::toJsonString() const
{
	std::stringstream ss;
	ss << "{\"prefix\": "
			<< semtechDataPrefix2JsonString(prefix)
			<< ", \"addr\": \"" << getDeviceAddrStr() << "\""
			<< ", \"id\": " << devId.toJsonString()
			<< ", \"metadata\": " << metadataToJsonString() 
			<< ", \"rfm\": " << header.toJson()
			<< ", \"payload_size\": " << payload.size();

	if (hasMACPayload()) {
		MacPtr macPtr(getMACs());
		ss << ", \"mac\": " << (macPtr.toJSONString());
		if (macPtr.errorcode) {
			ss << ", \"mac_error_code\": " << macPtr.errorcode
				<< ", \"mac_error\": \"" << strerror_lorawan_ns(macPtr.errorcode) << "\"";
		}
	}
	if (hasApplicationPayload())
		ss << ", \"payload\": \"" << hexString(payload) << "\"";
	ss << "}";
	return ss.str();
}

/**
 * addr time name frequency channel datr codr rssi lsnr payloadSize payload
 * @return
 */
std::string SemtechUDPPacket::toCsvString() const
{
    if (metadata.empty())
        return "";
    std::stringstream ss;
    int ms = -1;
    time_t t = metadata[0].t;
    std::string dt = ltimeString(t, ms, "%FT%T") + "Z";	// "2020-12-16T12:17:00.12345Z";
    // time_t tg = gps2utc(metadata[0].tmms());
    // std::string dtg = ltimeString(tg, ms, "%FT%T") + "Z";	// "2020-12-16T12:17:00.12345Z";
    ss
        << getDeviceAddrStr()
        << "," << dt
        << ",\"" << DEVICENAME2string(devId.name) << "\""
        << "," << metadata[0].frequency()
        << "," << (int) metadata[0].chan
        << ",\"" << metadata[0].datr()
        << "\",\"" << metadata[0].codr()
        << "\"," << (int) metadata[0].rssi
        << "," << std::fixed << std::setprecision(2) << metadata[0].lsnr
        << "," << payload.size()
        << ",\"" << base64_encode(payload)
        << "\"";
    return ss.str();
}

void SemtechUDPPacket::setPayload(
	uint8_t port,
	const std::string &value
) {
	header.fport = port;
	header.header.fctrl.i = 0;
	payload = value;
}

void SemtechUDPPacket::ack(SEMTECH_ACK *retval) {	// 4 bytes
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

int SemtechUDPPacket::parseData(
	const std::string &data,
	IdentityService *identityService
) {
    header.header.macheader = * (MHDR *)data.c_str();
    switch (header.header.macheader.f.mtype) {
        case MTYPE_JOIN_REQUEST:
        {
            size_t payloadSize = data.size() - sizeof(MHDR) - sizeof(uint32_t);
            payload = data.substr(sizeof(MHDR), payloadSize);
            errcode = ERR_CODE_IS_JOIN;
            return errcode;
        }
        case MTYPE_JOIN_ACCEPT:
        {
            // with MHDR and MIC, ciphered
            payload = data;
            errcode = ERR_CODE_IS_JOIN;
            return errcode;
        }
    }

    if (!header.parse(data)) {
        return ERR_CODE_INVALID_RFM_HEADER;
    }

	// get identity
	if (identityService) {
		// load keys from the authentication service, at least devEUI and appSKey. Return 0- success, <0- error code
		int rc = identityService->get(devId, header.header.devaddr);
		if (rc == 0) {
			unsigned char direction = downlink ? 1 : 0;
			// calc MIC
			uint32_t mic = getMic(data);
			uint32_t micCalc = calculateMIC((const unsigned char*) data.c_str(),
                (unsigned char ) data.size() - 4, header.header.fcnt,
                direction, header.header.devaddr, devId.nwkSKey);
			errcode = mic == micCalc ? LORA_OK : ERR_CODE_INVALID_MIC; 
			if (errcode == LORA_OK) {
				int payloadSize = data.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t) - header.header.fctrl.f.foptslen;
				if (payloadSize > 0) {
					// FHDR FPort Fopts
					std::string p = data.substr(sizeof(RFM_HEADER) + sizeof(uint8_t) + header.header.fctrl.f.foptslen,
                        (size_t) payloadSize);
					KEY128 *key;
					if (header.fport == 0)
						key = &devId.nwkSKey;
					else
						key = &devId.appSKey;
					decryptPayload(p, header.header.fcnt, direction, header.header.devaddr, *key);
					payload = p; 
				} else {
					payload = "";
				}
			}
		} else {
			errcode = ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
		}
	} else {
        errcode = ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
    }
    if (errcode == ERR_CODE_DEVICE_ADDRESS_NOTFOUND) {
        int payloadSize = data.size() - sizeof(RFM_HEADER) - sizeof(uint32_t) - sizeof(uint8_t) - header.header.fctrl.f.foptslen;
        if (payloadSize > 0) {
            payload = data.substr(sizeof(RFM_HEADER) + sizeof(uint8_t) + header.header.fctrl.f.foptslen,payloadSize);
        }
    }
	return errcode;
}

bool SemtechUDPPacket::hasMACPayload() const
{
	// Packet with payload can contain FOpts up to 15 bytes
	if (header.header.fctrl.f.foptslen)
		return true;
	// Or MAC can be in the payload of type(FPort) 0
	// fport 1..223 - application payload
	// fport 224 - LoRaWAN test protocol
	return (header.fport == 0) && (payload.size() > 0);
}

bool SemtechUDPPacket::hasApplicationPayload() const
{
	// fport 1..223 - application payload
	// fport 224 - LoRaWAN test protocol
	return (header.fport >= 1) && (header.fport <= 223) && (payload.size() > 0);
}

/**
 * Return gateway MAC address as int with best SNR
 * @param retvalLsnr if provided, return SNR
 * @return 0 if not found
 */
uint64_t SemtechUDPPacket::getBestGatewayAddress(
	float *retvalLsnr
) const
{
	float f = -3.402823466E+38f;
	uint64_t r = 0;
	for (std::vector<rfmMetaData>::const_iterator it(metadata.begin()); it != metadata.end(); it++)
	{
		if (it->lsnr > f)
		{
			f = it->lsnr;
			r = it->gatewayId;
			if (retvalLsnr) {
				*retvalLsnr = f;
			}
		}
	}
	return r;
}

/**
 * Return downlink Semtech packet (from server to end-device)
 * @param payloadString RFM header and encrypted data
 * @param receivedTime time when gateway received, microseconds, internal counter
 * @param power transmission power
 * @return JSON serialized metadata and payloadString
 */
std::string SemtechUDPPacket::toTxJsonString
(
	const std::string &payloadString,
	uint32_t receivedTime,
	const int power
) const
{
	if (metadata.size() == 0)
		return "";
	int rfCh = 0;
	int metadataIdx = 0;
	SEMTECH_PREFIX pullPrefix;
	memmove(&pullPrefix, &prefix, sizeof(SEMTECH_PREFIX));
	pullPrefix.tag = SEMTECH_GW_PULL_RESP; // PULL_RESP, after receiving PULL_DATA

	std::stringstream ss;
	ss << std::string((const char *) &pullPrefix, sizeof(SEMTECH_PREFIX))
		<< "{\"" << METADATA_TX_NAMES[0] << "\":{"; // txpk
	if (receivedTime == 0)
		ss << "\"" << METADATA_TX_NAMES[1] << "\":true";    // send immediately
	else {
		uint32_t sendTime = receivedTime + 1000000;
		ss << "\"" << METADATA_TX_NAMES[2] << "\":" << sendTime;
	}

	ss << ",\"" << METADATA_TX_NAMES[4] << "\":" << metadata[metadataIdx].frequency()       // "868.900"
        // "rfch": 0. @see https://github.com/brocaar/chirpstack-network-server/issues/19
        << ",\"" << METADATA_TX_NAMES[5] << "\":" << 0                                      // Concentrator "RF chain" used for TX (unsigned integer)
		<< ",\"" << METADATA_TX_NAMES[6] << "\":" << power									// TX output power in dBm (unsigned integer, dBm precision)
		<< ",\"" << METADATA_TX_NAMES[7] << "\":\"" << metadata[metadataIdx].modulation()	// Modulation identifier "LORA" or "FSK"
		<< "\",\"" << METADATA_TX_NAMES[8] << "\":\"" << metadata[metadataIdx].datr()
        << "\",\"" << METADATA_TX_NAMES[9] << "\":\"" << metadata[metadataIdx].codr()
        << "\",\"" << METADATA_TX_NAMES[11] << "\":true" 									// Lora modulation polarization inversion
		<< ",\"" << METADATA_TX_NAMES[15] << "\":false" 									// Check CRC
		<< ",\"" << METADATA_TX_NAMES[13] << "\":" << payloadString.size()
        << ",\"" << METADATA_TX_NAMES[14] << "\":\"" << base64_encode(payloadString) << "\"}}";
	
	return ss.str();
}

/**
 * Return uplink Semtech packet (from end-device to server)
 * @param payloadString RFM header and encrypted data
 * @param receivedTime time when gateway received, microseconds, internal counter
 * @return JSON serialized metadata and payloadString
 */
std::string SemtechUDPPacket::toRxJsonString
(
    const std::string &payloadString,
    uint32_t receivedTime
) const
{
    if (metadata.size() == 0)
        return "";
    int rfCh = 0;
    int metadataIdx = 0;
    SEMTECH_PREFIX_GW gwPrefix;
    memmove(&gwPrefix, &prefix, sizeof(SEMTECH_PREFIX_GW));
    gwPrefix.tag = SEMTECH_GW_PUSH_DATA; // PUSH_DATA 0x00

    std::stringstream ss;
    ss << std::string((const char *) &gwPrefix, sizeof(SEMTECH_PREFIX_GW))
       << "{\"" << METADATA_RX_NAMES[0] << "\": [{"; // rxpk
    std::string dt = gtimeString(receivedTime, -1, "%FT%T");
    ss << "\"" << METADATA_RX_NAMES[1] << "\": \"" << dt;   // time UTC time of pkt RX, us precision, ISO 8601 'compact' format
    // skip tmms GPS time of pkt RX, number of milliseconds since 06.Jan.1980
    ss << "\", \"" << METADATA_RX_NAMES[3] << "\": " << 0;   // tmst Internal timestamp of "RX finished" event (32b unsigned)
    ss << ", \"" << METADATA_RX_NAMES[4] << "\": " << metadata[metadataIdx].frequency(); // freq RX central frequency in MHz (unsigned float, Hz precision), e.g. 868.900
    // "rfch": 0. @see https://github.com/brocaar/chirpstack-network-server/issues/19
    ss << ", \"" << METADATA_RX_NAMES[5] << "\": " << (int) metadata[metadataIdx].chan; // chan Concentrator "IF" channel used for RX (unsigned integer
    ss << ", \"" << METADATA_RX_NAMES[6] << "\": " << (int) metadata[metadataIdx].rfch;  // rfch Concentrator "RF chain" used for RX (unsigned integer)
    ss << ", \"" << METADATA_RX_NAMES[7] << "\": " << (int) metadata[metadataIdx].stat;	// stat CRC status: 1 = OK, -1 = fail, 0 = no CRC
    ss << ", \"" << METADATA_RX_NAMES[8] << "\": \"" << metadata[metadataIdx].modulation();	// modu Modulation identifier "LORA" or "FSK"
    ss << "\", \"" << METADATA_RX_NAMES[9] << "\": \"" << metadata[metadataIdx].datr();	// datr LoRa data rate identifier (eg. SF12BW500) or FSK datarate (unsigned, in bits per second)
    ss << "\", \"" << METADATA_RX_NAMES[10] << "\": \"" << metadata[metadataIdx].codr();	// codr LoRa ECC coding rate identifier
    ss << "\", \"" << METADATA_RX_NAMES[11] << "\": " << metadata[metadataIdx].rssi;	// rssi RSSI in dBm (signed integer, 1 dB precision)
    ss << ", \"" << METADATA_RX_NAMES[12] << "\": " << metadata[metadataIdx].lsnr;	// lsnr Lora SNR ratio in dB (signed float, 0.1 dB precision)
    ss << ", \"" << METADATA_RX_NAMES[13] << "\": " << payloadString.size();	// size RF packet payload size in bytes (unsigned integer
    ss << ", \"" << METADATA_RX_NAMES[14] << "\": \"" << base64_encode(payloadString);	// data Base64 encoded RF packet payload, padded
    ss << "\"}]}";
    return ss.str();
}

/**
 * Make PULL_RESP Semtech UDP protocol packet repsonse
 * @param data payload
 * @param key key
 * @param power transmission power
 */ 
/*
  15 bytes
  Message Type = Unconfirmed Data Down
  Direction = down
  10 01450330 7f c6d7
  PHYPayload = 60   30034501 83 7803 021901 4371C6B1
               MHDR
                    DevAddr = 01450330
                             FCtrl = 83
                                FCnt = 0378
                                     FOpts = 021901
									        MIC 4
        FCnt = 888 (from packet, 16 bits) 
             = 888 (32 bits, assuming MSB 0x0000)
   FCtrl.ACK = false
   FCtrl.ADR = true


   10014503307fc51700a81ba59df88bc0a30603db696d2a5bfe6e01ec795579ddea1d62e74748dce3d04af776b87f975066c534
*/

std::string SemtechUDPPacket::mkPullResponse(
	const std::string &data,
	const DeviceId &deviceId,
	uint32_t receivedTime,
	const int fcnt,
	const int power
) const
{
	// copy macheader, addr, fcnt form received packet
	RFMHeader rfmHeader(*getRfmHeader());

	// encrypt frame payload
	uint8_t direction = 1;	// downlink
	std::string frmPayload(data);
	size_t payloadSize = frmPayload.size();
	if (deviceId.version.minor == 1) {
		encryptPayload(frmPayload, rfmHeader.header.fcnt, direction, rfmHeader.header.devaddr, deviceId.nwkSKey);	// network key
	}

	std::stringstream sMsg;

	// replace direction: MTYPE_UNCONFIRMED_DATA_UP to MTYPE_UNCONFIRMED_DATA_DOWN
	rfmHeader.header.macheader.f.mtype = MTYPE_UNCONFIRMED_DATA_DOWN;
	rfmHeader.header.fctrl.i = 0;
	rfmHeader.header.fcnt = fcnt;

	if (payloadSize <= 15) {
		// use FOpts
		rfmHeader.header.fctrl.f.foptslen = payloadSize;
		// device controlled by service
		rfmHeader.header.fctrl.f.adr = 1;
		sMsg << rfmHeader.toBinary();
	} else {
		// use FrmPayload
		sMsg << rfmHeader.toBinary();
		sMsg << (uint8_t) 0;	// fport 0- MAC payload
	}
	sMsg << frmPayload;

	std::string msg = sMsg.str();
	// calc mic
	uint32_t mic = calculateMIC((const unsigned char*) msg.c_str(),
        (unsigned char) msg.size(), rfmHeader.header.fcnt,
        direction, rfmHeader.header.devaddr, deviceId.nwkSKey);
	sMsg << std::string((char *) &mic, 4);
	return toTxJsonString(sMsg.str(), receivedTime, power);
}

/**
 * Make Semtech UDP protocol packet request or response
 * @param typ MTYPE_UNCONFIRMED_DATA_DOWN NS->end device, MTYPE_UNCONFIRMED_DATA_UP end-device->NS
 * @param data payload
 * @param networkIdentity device address, keys
 * @param time time
 * @param fCnt fCnt
 * @param gwIdentifier gateway identifier
 * @param power transmission power
 */
std::string SemtechUDPPacket::mkPushDataPacket(
        MTYPE typ,
        const std::string &payload,
        const NetworkIdentity &networkIdentity,
        uint32_t time,
        const int fCnt,
        const  uint64_t gwIdentifier,
        const int power
)
{
    header.fport = 0;
    devId = networkIdentity;
    memmove(header.header.devaddr, networkIdentity.devaddr, sizeof(DEVADDR));
    // set gateway
    setGatewayId(gwIdentifier);

    rfmMetaData rfmMD;
    rfmMD.setDatr("SF7BW125");
    metadata.push_back(rfmMD);

    RFMHeader rfmHeader(*getRfmHeader());
    rfmHeader.header.macheader.f.mtype = typ;
    rfmHeader.header.fctrl.i = 0;
    rfmHeader.header.fcnt = fCnt;

    // encrypt frame payload
    std::string frmPayload(payload);
    size_t payloadSize = frmPayload.size();
    uint8_t direction = rfmHeader.header.macheader.f.mtype & 1;    // MTYPE_UNCONFIRMED_DATA_UP -> 0, MTYPE_CONFIRMED_DATA_UP -> 0, MTYPE_UNCONFIRMED_DATA_DOWN -> 1, MTYPE_CONFIRMED_DATA_DOWN -> 1,..\    // copy macheader, addr, fCnt form received packet
    if (networkIdentity.version.minor == 1) {
        encryptPayload(frmPayload, rfmHeader.header.fcnt, direction, rfmHeader.header.devaddr, networkIdentity.nwkSKey);	// network key
    } else {
        encryptPayload(frmPayload, rfmHeader.header.fcnt, direction, rfmHeader.header.devaddr, networkIdentity.nwkSKey);	// network key
    }

    std::stringstream sMsg;

    bool forcePayload = true;

    if ((!forcePayload) && (payloadSize <= 15)) {
        // use FOpts
        rfmHeader.header.fctrl.f.foptslen = payloadSize;
        // device controlled by service
        rfmHeader.header.fctrl.f.adr = 1;
        sMsg << rfmHeader.toBinary();
    } else {
        // use FrmPayload
        sMsg << rfmHeader.toBinary();
        sMsg << (uint8_t) 1;	// fport >0- payload (no MAC)
    }
    sMsg << frmPayload;

    std::string msg = sMsg.str();
    // calc mic
    uint32_t mic = calculateMIC((const unsigned char*) msg.c_str(),
        (unsigned char) msg.size(), rfmHeader.header.fcnt, direction,
                                rfmHeader.header.devaddr, networkIdentity.nwkSKey);
    sMsg << std::string((char *) &mic, 4);

    if (direction)  // downlink from server to end-device
        return toTxJsonString(sMsg.str(), time, power);
    else
        return toRxJsonString(sMsg.str(), time); // uplink from end-device to server
}

/**
	 * Make Semtech UDP protocol packet Join Accept response
	 * @param frame Join accept payload frame payload
	 * @param receivedTime time
	 * @param power transmission power
	 */
std::string SemtechUDPPacket::mkJoinAcceptResponse(
        const JOIN_ACCEPT_FRAME &frame,
        uint32_t receivedTime,
        const int power
) const
{
    return toTxJsonString(std::string((const char *) &frame, sizeof(JOIN_ACCEPT_FRAME)), receivedTime, power);
}

/**
 * Make MAC request
 * @param data payload
 * @param networkId destination device network identifier
 * @param receivedTime received time
 * @param fCnt counter
 * @param power power value
 * @return MAC request string
 */
std::string SemtechUDPPacket::mkMACRequest(
    const DEVEUI *gwId,
	const std::string &data,
	const NetworkIdentity &networkId,
	uint32_t receivedTime,
	const int fCnt,
	const int power
) const
{
	RFMHeader rfmHeader;

	// encrypt frame payload
	uint8_t direction = 0;	// uplink
	std::string frmPayload(data);
	size_t pSize = frmPayload.size();

	rfmHeader.header.fcnt = fCnt;
	rfmHeader.header.macheader.f.mtype = MTYPE_UNCONFIRMED_DATA_DOWN;
	rfmHeader.header.fctrl.i = 0;

	if (networkId.version.minor == 1) {
		encryptPayload(frmPayload, rfmHeader.header.fcnt, direction, rfmHeader.header.devaddr, networkId.nwkSKey);	// network key
	}

	std::stringstream ssMsg;
	// replace direction: MTYPE_UNCONFIRMED_DATA_UP to MTYPE_UNCONFIRMED_DATA_DOWN

	if (pSize <= 15) {
		// use FOpts
		rfmHeader.header.fctrl.f.foptslen = pSize;
		// device controlled by service
		rfmHeader.header.fctrl.f.adr = 1;
		ssMsg << rfmHeader.toBinary();
	} else {
		// use FrmPayload
		ssMsg << rfmHeader.toBinary();
		ssMsg << (uint8_t) 0;	// FPort 0- MAC payload
	}
	ssMsg << frmPayload;

	std::string msg = ssMsg.str();
	// calc mic
	uint32_t mic = calculateMIC((const unsigned char*) msg.c_str(),
        (unsigned char) msg.size(), (unsigned int) rfmHeader.header.fcnt,
		direction, rfmHeader.header.devaddr, networkId.nwkSKey);
	ssMsg << std::string((char *) &mic, 4);
	return toTxJsonString(ssMsg.str(), 0, power);
}

/**
 * @return received time from interal counter, microsends
 */
uint32_t SemtechUDPPacket::tmst()
{
	std::vector<rfmMetaData>::const_iterator it(metadata.begin());
	if (it == metadata.end())
		return 0;
	return it->tmst;
}

/**
 * @return received GPS time, can be 0
 */
uint32_t SemtechUDPPacket::tmms()
{
	std::vector<rfmMetaData>::const_iterator it(metadata.begin());
	if (it == metadata.end())
		return 0;
	return it->tmms();
}

bool SemtechUDPPacket::isPayloadMAC() const
{
    return (header.fport == 0) && (payload.size() > 0);
}

std::string SemtechUDPPacket::getMACs() const {
	if (header.header.fctrl.f.foptslen)
		return std::string((const char *) &header.fopts, header.header.fctrl.f.foptslen);
	// Or MAC can be in the payload of type(FPort) 0
	// fport 1..223 - application payload
	// fport 224 - LoRaWAN test protocol
	if ((header.fport == 0) && (payload.size() > 0))
	{
		return payload;
	}
	return "";
}

void SemtechUDPPacket::appendMACs(const std::string &macsString) {
    size_t szInsert = macsString.size();
    std::string macs = getMACs();
    size_t szExists = macs.size();
    size_t szNew = szExists + szInsert;
    bool forcePayload = szNew > 15;
    if (forcePayload || isPayloadMAC()) {
        payload += macsString;
        memset(&header.fopts, 0, sizeof(FOPTS));
        header.header.fctrl.f.foptslen = 0;
    } else {
        memmove(&header.fopts.fopts[szExists], macsString.c_str(), szInsert);
        header.header.fctrl.f.foptslen = szNew;
    }
}

JOIN_REQUEST_FRAME *SemtechUDPPacket::getJoinRequestFrame() const
{
    // if (errCode != ERR_CODE_IS_JOIN)  return NULL; // after parseRX
    size_t sz = payload.size();
    if (sz == sizeof(JOIN_REQUEST_FRAME))
        return (JOIN_REQUEST_FRAME *) payload.c_str();
    return NULL;
}

JOIN_ACCEPT_FRAME *SemtechUDPPacket::getJoinAcceptFrame() const
{
    // if (errCode != ERR_CODE_IS_JOIN) return NULL; // after parseRX
    size_t sz = payload.size();
    if (sz == sizeof(JOIN_ACCEPT_FRAME))
        return (JOIN_ACCEPT_FRAME *) payload.c_str();
    return NULL;
}

JOIN_ACCEPT_FRAME_CFLIST *SemtechUDPPacket::getJoinAcceptCFListFrame() const
{
    // if (errCode != ERR_CODE_IS_JOIN) return NULL; // after parseRX
    size_t sz = payload.size();
    if (sz == sizeof(JOIN_ACCEPT_FRAME_CFLIST))
        return (JOIN_ACCEPT_FRAME_CFLIST *) payload.c_str();
    return NULL;
}

uint64_t deveui2int(
	const DEVEUI &value
)
{
	uint64_t v;
	memmove(&v, &value, sizeof(DEVEUI));
	return NTOH8(v);
}

void int2deveui(
    DEVEUI &retval,
    const uint64_t value
)
{
    *(uint64_t *) &retval = (uint64_t) NTOH8(value);
}

uint32_t getMic(const std::string &v)
{
	uint32_t r = *((uint32_t *) (v.c_str() + v.size() - 4));
	return r; // NTOH4(r);
}

uint64_t str2gatewayId(const char *value) {
	return strtoull(value, NULL, 16);
}

std::string gatewayId2str(uint64_t value) {
	std::stringstream ss;
	ss << std::hex << value;
	return ss.str();
}

std::string TDEVEUI::toString() const {
	return DEVEUI2string(eui);
}

std::string opts2string
(
	uint8_t len,
	const FOPTS &value
)
{
	if (len > sizeof(FOPTS))
		len = sizeof(FOPTS);
	return hexString(&value, len);
}

std::string mtype2string
(
	MTYPE value
) {
	switch (value) {
		case MTYPE_JOIN_REQUEST:
			return "join-request";
		case MTYPE_JOIN_ACCEPT:
			return "join-accept";
		case MTYPE_UNCONFIRMED_DATA_UP:
			return "unconfirmed-data-up";
		case MTYPE_UNCONFIRMED_DATA_DOWN:
			return "unconfirmed-data-down";
		case MTYPE_CONFIRMED_DATA_UP:
			return "confirmed-data-up";
		case MTYPE_CONFIRMED_DATA_DOWN:
			return "confirmed-data-up";
		case MTYPE_REJOIN_REQUEST:
			return "rejoin-request";
		case MTYPE_PROPRIETARYRADIO:
			return "proprietary-radio";
		default:
			return "";
	}
}

MTYPE string2mtype(
	const std::string &value
) {
	if (value == "join-request")
		return MTYPE_JOIN_REQUEST;
 	if (value == "join-accept")
	 	return MTYPE_JOIN_ACCEPT;
 	if (value == "unconfirmed-data-up")
	 	return MTYPE_UNCONFIRMED_DATA_UP;
 	if (value == "unconfirmed-data-down")
	 	return MTYPE_UNCONFIRMED_DATA_DOWN;
	if (value == "confirmed-data-up")
		return MTYPE_CONFIRMED_DATA_UP;
 	if (value == "confirmed-data-up")
	 	return MTYPE_CONFIRMED_DATA_DOWN;
 	if (value == "rejoin-request")
		return MTYPE_REJOIN_REQUEST;
 	if (value == "proprietary-radio")
	 	return MTYPE_PROPRIETARYRADIO;
	return MTYPE_JOIN_REQUEST;	//?!!
}

void string2NETID(
	NETID &retval,
	const char *value
) {
	std::string str = hex2string(value);
	size_t len = str.size();
	if (len > sizeof(NETID))
		len = sizeof(NETID);
	memmove(&retval, str.c_str(), len);
	if (len < sizeof(NETID))
		memset(&retval + len, 0, sizeof(NETID) - len);
}

void string2FREQUENCY(
	FREQUENCY &retval,
	const char *value
) {
	std::string str = hex2string(value);
	size_t len = str.size();
	if (len > sizeof(FREQUENCY))
		len = sizeof(FREQUENCY);
	memmove(&retval, str.c_str(), len);
	if (len < sizeof(FREQUENCY))
		memset(&retval + len, 0, sizeof(FREQUENCY) - len);
}

void string2JOINNONCE(
	JOINNONCE &retval,
	const char *value
) {
	std::string str = hex2string(value);
	size_t len = str.size();
	if (len > sizeof(JOINNONCE))
		len = sizeof(JOINNONCE);
	memmove(&retval, str.c_str(), len);
	if (len < sizeof(JOINNONCE))
		memset(&retval + len, 0, sizeof(JOINNONCE) - len);
}

std::string semtechDataPrefix2JsonString(
	const SEMTECH_PREFIX_GW &prefix
)
{
	std::stringstream ss;
	ss << "{"
		<< "\"version\":" << (int) prefix.version			// protocol version = 2
		<< ", \"token\":" << (int) prefix.token
		<< ", \"tag\":" << (int) prefix.tag
		<< ", \"mac\": \"" << DEVEUI2string(prefix.mac)		 /// 4-11	Gateway unique identifier (MAC address). For example : 00:0c:29:19:b2:37
		<< "\"}";
	return ss.str();
}

/**
 * SpreadFactorToRequiredSNRTable contains the required SNR to demodulate a LoRa frame for the given spreadfactor.
 * Reference: SX1276 datasheet
 * SpreadingFactor 6..12
 * @see https://semtech.my.salesforce.com/sfc/p/#E0000000JelG/a/2R0000001Rbr/6EfVZUorrpoKFfvaF_Fkpgp5kzjiNyiAbqcpqh9qSjE
 * 
*/
static float SpreadFactorToRequiredSNR[13] = {
	-5,	// 0
	-5,
	-5,
	-5,
	-5,
	-5,	// 5
	-5,
	-7.5,
	-10,
	-12.5,
	-15,
	-17.5,
	-20
};

/**
 * link margin, dB, range of 0..254
 * “0” - the frame was received at the demodulation floor
 * “20” - frame reached the gateway 20 dB above demodulation floor
 * @param spreadingFactor 6,.12, use always 6
 * @param loraSNR
 * @return 0..254
 */
uint8_t loraMargin(
	uint8_t spreadingFactor,
	float loraSNR
)
{
	if (spreadingFactor >= 12)
		spreadingFactor = 11;
	int r = (int) (loraSNR - SpreadFactorToRequiredSNR[spreadingFactor] + 0.5);	// round
	if (r < 0)
		r = 0;
	if (r > 254)
		r = 254;
	return r;
}

#define TRACK_CODE_SSIZE	11
static const char* TxAckCodes [TRACK_CODE_SSIZE] {
	"",
	"TOO_LATE",
	"TOO_EARLY",
	"FULL",		// n/a	Downlink queue is full
	"EMPTY",	// n/a
	"COLLISION_PACKET",
	"COLLISION_BEACON",
	"TX_FREQ",
	"TX_POWER",
	"GPS_UNLOCKED",
	"UNKNOWN"	// packet is invalid
};

ERR_CODE_TX extractTXAckCode(
	const void *buffer,
	size_t sz
)
{
	if ((sz < sizeof(SEMTECH_PREFIX_GW)) || ((const char *) buffer)[3] != 5) /// 5- PKT_TX_ACK
		return JIT_TX_OK;	// it is not transmission ACK packet
	std::string s(((const char *) buffer ) + sizeof(SEMTECH_PREFIX_GW), sz);
	std::size_t p = s.find("txpk_ack");
	if (p == std::string::npos)
		return JIT_TX_OK;
	p = s.find("error", p);
	if (p == std::string::npos)
		return JIT_TX_OK;
	for (int i = 1; i < TRACK_CODE_SSIZE; i++) {
		std::size_t f = s.find(TxAckCodes[i], p);
		if (f != std::string::npos)
			return (ERR_CODE_TX) i;
	}
	return JIT_TX_OK;
}

const char *getTXAckCodeName
(
	ERR_CODE_TX code
)
{
	return TxAckCodes[(int) code];
}

std::string LORAWAN_VERSION2string
(
	LORAWAN_VERSION value
)
{
	std::stringstream ss;
	ss 
		<< (int) value.major 
		<< "." << (int) value.minor
		<< "." << (int) value.release;
	return ss.str();
}

LORAWAN_VERSION string2LORAWAN_VERSION
(
	const std::string &value
)
{
	std::stringstream ss(value);
	int ma = 1, mi = 0, re = 0;
    char dot;
	if (!ss.eof ())
		ss >> ma;
	if (!ss.eof ())
		ss >> dot;
	if (!ss.eof ())
		ss >> mi;
	if (!ss.eof ())
		ss >> dot;
	if (!ss.eof ())
		ss >> re;
	LORAWAN_VERSION r = { (uint8_t) (ma & 3), (uint8_t) (mi & 3), (uint8_t) (re & 0xf) };
	return r;
  }

std::string REGIONAL_PARAMETERS_VERSION2string(
    REGIONAL_PARAMETERS_VERSION value
) {
    return LORAWAN_VERSION2string(*(LORAWAN_VERSION*) &value);
}

REGIONAL_PARAMETERS_VERSION string2REGIONAL_PARAMETERS_VERSION(
    const std::string &value
) {
    std::stringstream ss(value);
    int ma = 1, mi = 0, re = 0;
    char dot;
    if (!ss.eof ())
        ss >> ma;
    if (!ss.eof ())
        ss >> dot;
    if (!ss.eof ())
        ss >> mi;
    if (!ss.eof ())
        ss >> dot;
    if (!ss.eof ())
        ss >> re;
    REGIONAL_PARAMETERS_VERSION r = { (uint8_t) (ma & 3), (uint8_t) (mi & 3), (uint8_t) (re & 0xf) };
    return r;
}

std::string MODULATION2String(MODULATION value)
    {
        switch (value)
        {
            case FSK:
                return "FSK";
            default:
                return "LORA";
        }
    }

    MODULATION string2MODULATION(const char *value)
    {
        if (strcmp(value, "FSK") == 0)
            return FSK;
        else
            return LORA;
    }

std::string BANDWIDTH2String(BANDWIDTH value) {
    switch (value) {
        case BANDWIDTH_INDEX_7KHZ:
            return "7.8";
        case BANDWIDTH_INDEX_10KHZ:
            return "10.4";
        case BANDWIDTH_INDEX_15KHZ:
            return "15.6";
        case BANDWIDTH_INDEX_20KHZ:
            return "20.8";
        case BANDWIDTH_INDEX_31KHZ:
            return "31.2";
        case BANDWIDTH_INDEX_41KHZ:
            return "41.6";
        case BANDWIDTH_INDEX_62KHZ:
            return "62.5";
        case BANDWIDTH_INDEX_125KHZ:
            return "125";
        case BANDWIDTH_INDEX_250KHZ:
            return "250";
        case BANDWIDTH_INDEX_500KHZ:
            return "500";
    }
    return "7.8";
}

BANDWIDTH string2BANDWIDTH(const char *value)
{
    if (strcmp(value, "7.8") == 0)
        return BANDWIDTH_INDEX_7KHZ;
    if (strcmp(value, "10.4") == 0)
        return BANDWIDTH_INDEX_10KHZ;
    if (strcmp(value, "15.6") == 0)
        return BANDWIDTH_INDEX_15KHZ;
    if (strcmp(value, "20.8") == 0)
        return BANDWIDTH_INDEX_20KHZ;
    if (strcmp(value, "31.2") == 0)
        return BANDWIDTH_INDEX_31KHZ;
    if (strcmp(value, "41.6") == 0)
        return BANDWIDTH_INDEX_41KHZ;
    if (strcmp(value, "62.5") == 0)
        return BANDWIDTH_INDEX_62KHZ;
    if (strcmp(value, "125") == 0)
        return BANDWIDTH_INDEX_125KHZ;
    if (strcmp(value, "250") == 0)
        return BANDWIDTH_INDEX_250KHZ;
    if (strcmp(value, "500") == 0)
        return BANDWIDTH_INDEX_500KHZ;
    return BANDWIDTH_INDEX_7KHZ;
}

BANDWIDTH int2BANDWIDTH(int value)
{
    if (value == 7)
        return BANDWIDTH_INDEX_7KHZ;
    if (value == 10)
        return BANDWIDTH_INDEX_10KHZ;
    if (value == 15)
        return BANDWIDTH_INDEX_15KHZ;
    if (value == 20)
        return BANDWIDTH_INDEX_20KHZ;
    if (value == 31)
        return BANDWIDTH_INDEX_31KHZ;
    if (value == 41)
        return BANDWIDTH_INDEX_41KHZ;
    if (value == 62)
        return BANDWIDTH_INDEX_62KHZ;
    if (value == 125)
        return BANDWIDTH_INDEX_125KHZ;
    if (value == 250)
        return BANDWIDTH_INDEX_250KHZ;
    if (value == 500)
        return BANDWIDTH_INDEX_500KHZ;
    return BANDWIDTH_INDEX_7KHZ;
}

/**
 * @param value 7..500 or 7000..
 */
BANDWIDTH double2BANDWIDTH(double value)
{
    if (value <= 500)
        return int2BANDWIDTH((int) value);
    else {
        if (value >= 7000)
            return BANDWIDTH_INDEX_7KHZ;
        if (value >= 10000)
            return BANDWIDTH_INDEX_10KHZ;
        if (value >= 15000)
            return BANDWIDTH_INDEX_15KHZ;
        if (value >= 20000)
            return BANDWIDTH_INDEX_20KHZ;
        if (value >= 31000)
            return BANDWIDTH_INDEX_31KHZ;
        if (value >= 41000)
            return BANDWIDTH_INDEX_41KHZ;
        if (value >= 62000)
            return BANDWIDTH_INDEX_62KHZ;
        if (value >= 125000)
            return BANDWIDTH_INDEX_125KHZ;
        if (value >= 250000)
            return BANDWIDTH_INDEX_250KHZ;
        if (value >= 500000)
            return BANDWIDTH_INDEX_500KHZ;
        return BANDWIDTH_INDEX_7KHZ;

    }
}

bool isDEVADDREmpty(const DEVADDR &addr)
{
    return *((uint32_t *) &addr) == 0;
}

bool isDEVEUIEmpty(const DEVEUI &eui)
{
    return *((uint64_t *) &eui) == 0;
}
