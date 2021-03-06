#ifndef LORA_ENCRYPT_H_
#define LORA_ENCRYPT_H_	1

#include <string>

#include "utillora.h"

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
);

void decryptPayload(
	std::string &payload,
	unsigned int frameCounter,
	unsigned char direction,
	DEVADDR &devAddr,
	KEY128 &appSKey
);

/**
 * Encrypt Join Accept LoRaWAN message
 * @see 6.2.3 Join-accept message
 */
std::string encryptJoinAccept(
	const std::string &payload,
	const KEY128 &key
);

/**
 * Decrypt Join Accept LoRaWAN message
 * @see 6.2.3 Join-accept message
 */
std::string decryptJoinAccept(
	const std::string &payload,
	const KEY128 &key
);

#endif
