#include "lora-encrypt.h"

/**
 * @see 4.3.3 MAC Frame Payload Encryption (FRMPayload)
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
	aes_context aesContext;
	memset(aesContext.ksch, '\0', 240);
    aes_set_key(appSKey, sizeof(KEY128), &aesContext);
 
 	uint8_t a[16];
 	a[0] = 1;	// 1- uplink, 0- downlink
	a[1] = 0;
	a[2] = 0;
	a[3] = 0;
	a[4] = 0;
    a[5] = direction;
    a[6] = devAddr[0];
	a[7] = devAddr[1];
	a[8] = devAddr[2];
	a[9] = devAddr[3];
	a[10] = (frameCounter & 0x00FF);
	a[11] = ((frameCounter >> 8) & 0x00FF);
	a[12] = 0; // frame counter upper Bytes
	a[13] = 0;
	a[14] = 0;

	uint8_t s[16];
	memset(s, 0, sizeof(s));

	int size = payload.size();
	uint16_t ctr = 1;
	uint8_t bufferIndex = 0;
	std::string encBuffer(payload);
	while( size >= 16 ) {
        a[15] = ctr & 0xff;
        ctr++;
        aes_encrypt(a, s, &aesContext);
        for(int i = 0; i < 16; i++) {
            encBuffer[bufferIndex + i] = payload[bufferIndex + i] ^ s[i];
        }
        size -= 16;
        bufferIndex += 16;
    }
 
    if( size > 0 )
    {
        a[15] = ctr & 0xff;
        aes_encrypt(a, s, &aesContext);
        for(int i = 0; i < size; i++) {
            encBuffer[bufferIndex + i] = payload[bufferIndex + i] ^ s[i];
        }
    }

	/*
	std::string app(16 - (payload.size() % 16), '\0');
	std::string encBuffer(payload + app);
	
	int sz1 = size + 16 - (payload.size() % 16);
payload8_t bufferIndex = 0;

	for (int i = 0; i < sz1 / 16; i++)
	{
		a[15] = i + 1;
		aes_encrypt(a, appSKey);
		for (int j = 0; j < 16; j++) {
			encBuffer[ i * 16 + j] = encBuffer[i * 16 + j] ^ s[j];
		}
	}
	*/
	/*
    while (sz1 >= 16) {
        a[15] = ( ( ctr ) & 0xff );
        ctr++;
 		// calculate S
		aesEncrypt(a, appSKey);
        for (i = 0; i < 16; i++) {
            encBuffer[bufferIndex + i] = payload[bufferIndex + i] ^ a[i];
        }
        sz1 -= 16;
        bufferIndex += 16;
    }
	*/
	/*
 	if (size > 0) {
        a[15] = ( ( ctr ) & 0xff );
        aesEncrypt(a, appSKey);
        for (i = 0; i < size; i++) {
            encBuffer[bufferIndex + i] = payload[bufferIndex + i] ^ a[i];
        }
    }
	*/
	payload = encBuffer;
}

/*
static void encryptPayload2(
	std::string &payload,
	const unsigned int frameCounter,
	const unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &appSKey
)
{
	unsigned char *data = (unsigned char *) payload.c_str();
	unsigned char size = payload.size();
	unsigned char a[16];

	// calc number of blocks
	unsigned char numberOfBlocks = size / 16;
	unsigned char incompleteBlockSize = size % 16;
	if (incompleteBlockSize != 0)
		numberOfBlocks++;

	for (int i = 1; i <= numberOfBlocks; i++) {
		a[0] = 0x01;
		a[1] = 0x00;
		a[2] = 0x00;
		a[3] = 0x00;
		a[4] = 0x00;

		a[5] = direction;

		a[6] = devAddr[3];
		a[7] = devAddr[2];
		a[8] = devAddr[1];
		a[9] = devAddr[0];

		a[10] = (frameCounter & 0x00FF);
		a[11] = ((frameCounter >> 8) & 0x00FF);

		a[12] = 0x00; // frame counter upper Bytes
		a[13] = 0x00;

		a[14] = 0x00;

		a[15] = i;
 + app

		// check for last block
		if (i != numberOfBlocks) {
			for (int j = 0; j < 16; j++) {
				*data = *data ^ a[j];
				data++;
			}
		} else {
			if (incompleteBlockSize == 0)
				incompleteBlockSize = 16;

			for (int j = 0; j < incompleteBlockSize; j++) {
				*data = *data ^ a[j];
				data++;
			}
		}
	}
}
*/

void decryptPayload(
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
 * Encrypt Join Accept LoRaWAN message
 * @see 6.2.3 Join-accept message
 */
std::string encryptJoinAccept(
	const std::string &payload,
	const KEY128 &key
) {
	/*
	aes_context aesContext;
	memset(aesContext.ksch, '\0', 240);
    aes_set_key(key, sizeof(KEY128), &aesContext);
 
 	uint8_t a[16];
	memset(a, 0, sizeof(s));
	uint8_t s[16];
	memset(s, 0, sizeof(s));

	int size = payload.size();
	uint8_t bufferIndex = 0;
	std::string encBuffer(payload);
	while (size >= 16) {
        aes_encrypt(a, s, &aesContext);
        for(int i = 0; i < 16; i++) {
            encBuffer[bufferIndex + i] = payload[bufferIndex + i] ^ s[i];
        }
        size -= 16;
        bufferIndex += 16;
    }
 
    if (size > 0)
    {
        aes_encrypt(a, s, &aesContext);
        for(int i = 0; i < size; i++) {
            encBuffer[bufferIndex + i] = payload[bufferIndex + i] ^ s[i];
        }
    }
	payload = encBuffer;
	*/
	return "";
}

/**
 * Decrypt Join Accept LoRaWAN message
 * @see 6.2.3 Join-accept message
 */
std::string decryptJoinAccept(
	const std::string &payload,
	const KEY128 &key
) {
	aes_context aesContext;
	memset(aesContext.ksch, '\0', 240);
    aes_set_key(key, sizeof(KEY128), &aesContext);
 
 	uint8_t a[16];
	memset(a, 0, sizeof(a));
	uint8_t s[16];
	memset(s, 0, sizeof(s));

	int size = payload.size() - 1;
	uint8_t bufferIndex = 1;
	std::string encBuffer(payload);
	
	while (size >= 16) {
		aes_encrypt((const uint8_t*) payload.c_str() + bufferIndex,
			(uint8_t*) encBuffer.c_str() + bufferIndex, &aesContext);
	    size -= 16;
        bufferIndex += 16;
    }
	
 	return encBuffer;
}
