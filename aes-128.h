#include <inttypes.h>

typedef unsigned char KEY128[16];

#ifdef __cplusplus
extern "C" {
#endif

void aesEncrypt(
	unsigned char *data,
	const unsigned char *key
);

void generateKeys(
	unsigned char *K1,
	unsigned char *K2,
	const unsigned char *key
);

void XOR(
	unsigned char *New_Data,
	unsigned char *Old_Data
);

#ifdef __cplusplus
}
#endif