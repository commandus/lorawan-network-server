#include "utilidentity.h"

IDENTITY_STORAGE string2storageType(
	const std::string &value
) {
	if (value == "txt")
		return IDENTITY_STORAGE_DIR_TEXT;
	if (value == "lmdb")
		return IDENTITY_STORAGE_LMDB;
	return IDENTITY_STORAGE_FILE_JSON;
};

std::string storageType2String
(
	IDENTITY_STORAGE value
) 
{
	switch(value) {
		case IDENTITY_STORAGE_DIR_TEXT:
			return "txt";
		case IDENTITY_STORAGE_LMDB:
			return "lmdb";
		default:
			return "json";
	}
}
