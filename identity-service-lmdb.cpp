#include <iostream>
#include <fstream>

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
 
#include "identity-service-lmdb.h"
#include "utilstring.h"
#include "errlist.h"

#define LOG(verbosity) std::cerr

LmdbIdentityService::LmdbIdentityService() 
	: filename("")
{

}

LmdbIdentityService::~LmdbIdentityService() 
{
	done();
}

void LmdbIdentityService::clear()
{
	// TODO not implemented yet
}

int LmdbIdentityService::open()
{
	int flags = 0;
	int mode = 0664;
	env = new dbenv(filename, flags, mode);
	if (!openDb(env))
	{
		LOG(ERROR) << ERR_LMDB_OPEN << filename << std::endl;
		return ERR_CODE_LMDB_OPEN;
	}
	return LORA_OK;
} 

int LmdbIdentityService::close()
{
	if (!closeDb(env))
	{
		LOG(ERROR) << ERR_LMDB_CLOSE << filename << std::endl;
		return ERR_CODE_LMDB_CLOSE;
	}
	return LORA_OK;
}

int LmdbIdentityService::get(
	DEVADDR &devaddr,
	DeviceId &retval
) 
{
	int r = 0;
	return r;
}

void LmdbIdentityService::put(
	DEVADDR &devaddr,
	DEVICEID &id
)
{
}

void LmdbIdentityService::rm(
	DEVADDR &addr
)
{
}

int LmdbIdentityService::init(
	const std::string &option, 
	void *data
)
{
	filename = option;
	open();
}

void LmdbIdentityService::flush()
{
}

void LmdbIdentityService::done()
{
	close();
}
