#ifndef UTIL_IDENTITY_H_
#define UTIL_IDENTITY_H_	1

#include <string>

typedef enum {
	IDENTITY_STORAGE_FILE_JSON = 1,
	IDENTITY_STORAGE_DIR_TEXT = 2,
	IDENTITY_STORAGE_LMDB = 3
} IDENTITY_STORAGE;

IDENTITY_STORAGE string2storageType
(
	const std::string &value
);

std::string storageType2String
(
	IDENTITY_STORAGE value
);

#endif
