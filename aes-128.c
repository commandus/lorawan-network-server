#include <string.h>
#include "aes-128.h"

static const unsigned char STable[16][16] = {
  { 0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76 },
  { 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0 },
  { 0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15 },
  { 0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75 },
  { 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84 },
  { 0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF },
  { 0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8 },
  { 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2 },
  { 0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73 },
  { 0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB },
  { 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79 },
  { 0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08 },
  { 0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A },
  { 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E },
  { 0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF },
  { 0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16 }
};

static void aesAddRoundKey(
	unsigned char *roundKey,
	unsigned char (*state)[4]
)
{
	unsigned char row, collum;

	for (collum = 0; collum < 4; collum++) {
		for (row = 0; row < 4; row++) {
			state[row][collum] ^= roundKey[row + (collum << 2)];
		}
	}
}

static void aesShiftRows(
	unsigned char (*state)[4]
)
{
	unsigned char buffer;

	//Store firt byte in buffer
	buffer = state[1][0];
	//Shift all bytes
	state[1][0] = state[1][1];
	state[1][1] = state[1][2];
	state[1][2] = state[1][3];
	state[1][3] = buffer;

	buffer = state[2][0];
	state[2][0] = state[2][2];
	state[2][2] = buffer;
	buffer = state[2][1];
	state[2][1] = state[2][3];
	state[2][3] = buffer;

	buffer = state[3][3];
	state[3][3] = state[3][2];
	state[3][2] = state[3][1];
	state[3][1] = state[3][0];
	state[3][0] = buffer;
}

static void aesMixCollums(
	unsigned char (*state)[4]
)
{
	unsigned char row, collum;
	unsigned char a[4], b[4];

	for (collum = 0; collum < 4; collum++) {
		for (row = 0; row < 4; row++) {
			a[row] = state[row][collum];
			b[row] = (state[row][collum] << 1);

			if ((state[row][collum] & 0x80) == 0x80) {
				b[row] ^= 0x1B;
			}
		}

		state[0][collum] = b[0] ^ a[1] ^ b[1] ^ a[2] ^ a[3];
		state[1][collum] = a[0] ^ b[1] ^ a[2] ^ b[2] ^ a[3];
		state[2][collum] = a[0] ^ a[1] ^ b[2] ^ a[3] ^ b[3];
		state[3][collum] = a[0] ^ b[0] ^ a[1] ^ a[2] ^ b[3];
	}
}

/**
 * unsigned char S_Row,S_Collum;
 * unsigned char S_Byte;
 * S_Row    = ((byt >> 4) & 0x0F);
 * S_Collum = ((byt >> 0) & 0x0F);
 * S_Byte   = STable [S_Row][S_Collum];
 */
static unsigned char aesSubByte(
	unsigned char byt
)
{
	// return pgm_read_byte(&(STable[((byt >> 4) & 0x0F)][((byt >> 0) & 0x0F)]));
	return STable [ ((byt >> 4) & 0x0F) ] [ ((byt >> 0) & 0x0F) ];
}

static void aesCalculateRoundKey(
	unsigned char round,
	unsigned char *roundKey
)
{
	unsigned char i, j, b, rcon;
	unsigned char tmp[4];
	// Calculate rcon
	rcon = 0x01;
	while (round != 1) {
		b = rcon & 0x80;
		rcon = rcon << 1;

		if (b == 0x80)
			rcon ^= 0x1b;
		round--;
	}

	//  Calculate first tmp
	//  Copy laste byte from previous key and subsitute the byte, but shift the array contents around by 1.
	tmp[0] = aesSubByte(roundKey[12 + 1]);
	tmp[1] = aesSubByte(roundKey[12 + 2]);
	tmp[2] = aesSubByte(roundKey[12 + 3]);
	tmp[3] = aesSubByte(roundKey[12 + 0]);

	// XOR with rcon
	tmp[0] ^= rcon;

	//  Calculate new key
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			roundKey[j + (i << 2)] ^= tmp[j];
			tmp[j] = roundKey[j + (i << 2)];
		}
	}
}

void aesEncrypt(
	unsigned char *data,
	unsigned char *key
)
{
	unsigned char row, column, round = 0;
	unsigned char roundKey[16];
	unsigned char state[4][4];

	// Copy input to state arry
	for (column = 0; column < 4; column++) {
		for (row = 0; row < 4; row++) {
			state[row][column] = data[row + (column << 2)];
		}
	}

	// Copy key to round key
	memcpy(&roundKey[0], &key[0], 16);

	// Add round key
	aesAddRoundKey(roundKey, state);

	// Perform 9 full rounds with mixed collums
	for (round = 1; round < 10; round++) {
		// Perform byt substitution with S table
		for (column = 0; column < 4; column++) {
			for (row = 0; row < 4; row++) {
				state[row][column] = aesSubByte(state[row][column]);
			}
		}

		// Perform row Shift
		aesShiftRows(state);

		// Mix Collums
		aesMixCollums(state);

		// Calculate new round key
		aesCalculateRoundKey(round, roundKey);

		// Add the round key to the roundKey
		aesAddRoundKey(roundKey, state);
	}

	// Perform byt substitution with S table whitout mix collums
	for (column = 0; column < 4; column++) {
		for (row = 0; row < 4; row++) {
			state[row][column] = aesSubByte(state[row][column]);
		}
	}

	// Shift rows
	aesShiftRows(state);

	// Calculate new round key
	aesCalculateRoundKey(round, roundKey);

	// Add round key
	aesAddRoundKey(roundKey, state);

	// Copy the state into the data array
	for (column = 0; column < 4; column++) {
		for (row = 0; row < 4; row++) {
			data[row + (column << 2)] = state[row][column];
		}
	}
}

static void shiftLeft(
	unsigned char *data
)
{
	unsigned char i;
	unsigned char overflow = 0;
	for (i = 0; i < 16; i++) {
		// Check for overflow on next byte except for the last byte
		if (i < 15) {
			// Check if upper bit is one
			if ((data[i + 1] & 0x80) == 0x80)
				overflow = 1;
			else
				overflow = 0;
		} else
			overflow = 0;
		// Shift one left
		data[i] = (data[i] << 1) + overflow;
	}
}

void generateKeys(
	unsigned char *K1,
	unsigned char *K2,
	unsigned char *key
)
{
	unsigned char i;
	unsigned char msbKey;

	// Encrypt the zeros in K1 with the NwkSkey
	aesEncrypt(K1, key);

	// Create K1
	// Check if MSB is 1
	if ((K1[0] & 0x80) == 0x80)
		msbKey = 1;
	else
		msbKey = 0;

	// Shift K1 one bit left
	shiftLeft(K1);

	// if MSB was 1
	if (msbKey == 1)
		K1[15] = K1[15] ^ 0x87;

	// Copy K1 to K2
	for (i = 0; i < 16; i++)
		K2[i] = K1[i];

	// Check if MSB is 1
	if ((K2[0] & 0x80) == 0x80)
		msbKey = 1;
	else
		msbKey = 0;

	// Shift K2 one bit left
	shiftLeft(K2);

	// Check if MSB was 1
	if (msbKey == 1)
		K2[15] = K2[15] ^ 0x87;
}

void XOR(
	unsigned char *newData,
	unsigned char *oldData
)
{
	unsigned char i;
	for (i = 0; i < 16; i++) {
		newData[i] = newData[i] ^ oldData[i];
	}
}
