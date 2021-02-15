#include <iostream>
#include <fstream>

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
 
#include "identity-service-lmdb.h"
#include "utilstring.h"
#include "errlist.h"

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
		return ERR_CODE_LMDB_OPEN;
	return LORA_OK;
} 

int LmdbIdentityService::close()
{
	if (!closeDb(env))
		return ERR_CODE_LMDB_CLOSE;
	return LORA_OK;
}

int LmdbIdentityService::get(
	DEVADDR &devaddr,
	DeviceId &retval
) 
{
	DEVICEID v;
	return getAddr(env, devaddr, v);
	retval.set(v);
}

bool onRecord
(
	void *env,
	DEVADDR *key,
	DEVICEID *data
) {
	std::vector<NetworkIdentity> *r = (std::vector<NetworkIdentity> *) env;
	r->push_back(NetworkIdentity(*key, *data));
}

void LmdbIdentityService::list(
	std::vector<NetworkIdentity> &retval,
	size_t offset,
	size_t size
)
{
	lsAddr(env, &onRecord, &retval);
}

void LmdbIdentityService::put(
	DEVADDR &devaddr,
	DEVICEID &deviceid
)
{
	putAddr(env, devaddr, deviceid);
}

void LmdbIdentityService::rm(
	DEVADDR &devaddr
)
{
	rmAddr(env, devaddr);
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
