#include <inttypes.h>

typedef unsigned char KEY128[16];

#ifdef __cplusplus
extern "C" {
#endif

void aesEncrypt(
	unsigned char *data,
	unsigned char *key
);

void generateKeys(
	unsigned char *K1,
	unsigned char *K2,
	unsigned char *key
);

void XOR(
	unsigned char *New_Data,
	unsigned char *Old_Data
);

#ifdef __cplusplus
}
#endif