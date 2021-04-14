#include <iostream>
#include <fstream>
#include <regex>

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

static bool onRecord
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

class FindEUIEnv {
	public:
		std::string value;
		std::vector<TDEVEUI> retval;
		bool found;

		FindEUIEnv(
			const std::string &val
		)
			: value(val), found(false) {
		}
};

static bool onFindEUI
(
	void *env,
	DEVADDR *key,
	DEVICEID *data
) {
	FindEUIEnv *r = (FindEUIEnv *) env;
	NetworkIdentity nid(*key, *data);
	if (isHex(r->value)) {
		TDEVEUI v(r->value);
		r->found = memcmp(&v.eui, &nid.deviceEUI, sizeof(DEVEUI)) == 0;
		if (r->found)
			r->retval.push_back(v);
		return r->found;
	} else {
		// can contain regex "*"
		try {
			std::regex rex(r->value, std::regex_constants::grep);
			std::string s2 = DEVEUI2string(nid.deviceEUI);
			if (std::regex_search(s2, rex))
				r->retval.push_back(TDEVEUI(nid.deviceEUI));
		}
		catch (const std::regex_error& e) {
			return false;
		}
	}
	return true;
}

static bool onFindName
(
	void *env,
	DEVADDR *key,
	DEVICEID *data
) {
	FindEUIEnv *r = (FindEUIEnv *) env;
	NetworkIdentity nid(*key, *data);
	// can contain regex "*"
	try {
		std::regex rex(r->value, std::regex_constants::grep);
		std::string s2 = DEVEUI2string(nid.deviceEUI);
		if (std::regex_search(s2, rex))
			r->retval.push_back(TDEVEUI(nid.deviceEUI));
	}
	catch (const std::regex_error& e) {
		return false;
	}
	return true;
}

int LmdbIdentityService::parseIdentifiers(
	std::vector<TDEVEUI> &retval,
	const std::vector<std::string> &list,
	bool useRegex
) {
	for (std::vector<std::string>::const_iterator it(list.begin()); it != list.end(); it++) {
		std::string re;
		if (useRegex)
			re = *it;
		else
			re = replaceAll(replaceAll(*it, "*", ".*"), "?", ".");
		FindEUIEnv findEUIEnv(re);
		lsAddr(env, &onFindEUI, &findEUIEnv);
		if (!findEUIEnv.found)
			return ERR_CODE_INVALID_DEVICE_EUI;
	}
	return 0;
}

int LmdbIdentityService::parseNames(
	std::vector<TDEVEUI> &retval,
	const std::vector<std::string> &list,
	bool useRegex
) {
	for (std::vector<std::string>::const_iterator it(list.begin()); it != list.end(); it++) {
		std::string re;
		if (useRegex)
			re = *it;
		else
			re = replaceAll(replaceAll(*it, "*", ".*"), "?", ".");
		FindEUIEnv findEUIEnv(re);
		lsAddr(env, &onFindName, &findEUIEnv);
		if (!findEUIEnv.found)
			return ERR_CODE_INVALID_DEVICE_EUI;
	}
	return 0;
}
