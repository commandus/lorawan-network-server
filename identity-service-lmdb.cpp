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

class ListEnv {
	public:
		std::vector<NetworkIdentity> *list;
		size_t c;
		size_t offset;
		size_t size;

		ListEnv(
			std::vector<NetworkIdentity> *alist,
			size_t aoffset,
			size_t asize
		)
		: list(alist), c(0), offset(aoffset) {
			if (asize <= 0)
				size = UINT64_MAX;
			else
				size = asize;
		}
};

bool onRecord
(
	void *env,
	DEVADDR *key,
	DEVICEID *data
) {
	ListEnv *r = (ListEnv *) env;
	r->c++;	
	if (r->c <= r->offset)
		return false;
	if (r->c > r->offset + r->size)
		return true;
	r->list->push_back(NetworkIdentity(*key, *data));
	return false;
}

void LmdbIdentityService::list(
	std::vector<NetworkIdentity> &retval,
	size_t offset,
	size_t size
)
{
	ListEnv listenv(&retval, offset, size);
	lsAddr(env, &onRecord, &listenv);
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
