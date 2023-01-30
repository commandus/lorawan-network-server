#include <iostream>
#include <fstream>
#include <regex>

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
 
#include "identity-service-lmdb.h"
#include "identity-service-file-json.h"
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
	DeviceId &retval,
	DEVADDR &devaddr
)
{
	DEVICEID v;
	int r = getAddr(env, devaddr, v);
	retval.set(v);
	return r;
}

class FindAddrEnv {
	public:
		DEVEUI value;
		NetworkIdentity retval;
		bool found;

		FindAddrEnv(
			const DEVEUI &eui
		)
			: found(false) 
		{
			memmove(&value, &eui, sizeof(DEVEUI)); 
		}
};

static bool onFindAddr
(
	void *env,
	DEVADDR *key,
	DEVICEID *data
) {
	FindAddrEnv *e = (FindAddrEnv *) env;
	NetworkIdentity nid(*key, *data);
	e->found = memcmp(&e->value, &nid.devEUI, sizeof(DEVEUI)) == 0;
	if (e->found)
		e->retval = nid;
	return e->found;
}

int LmdbIdentityService::getNetworkIdentity(
	NetworkIdentity &retval,
	const DEVEUI &eui
) {
	DEVICEID v;
	FindAddrEnv findAddrEnv(eui);
	lsAddr(env, &onFindAddr, &findAddrEnv);
	if (!findAddrEnv.found)
		return ERR_CODE_INVALID_DEVICE_EUI;
	retval = findAddrEnv.retval;
	return LORA_OK;
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
				size = SIZE_MAX;
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

size_t LmdbIdentityService::size()
{
	return count(env);
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
	return open();
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
		r->found = memcmp(&v.eui, &nid.devEUI, sizeof(DEVEUI)) == 0;
		if (r->found)
			r->retval.push_back(v);
		return r->found;
	} else {
		// can contain regex "*"
		try {
			std::regex rex(r->value, std::regex_constants::grep);
			std::string s2 = DEVEUI2string(nid.devEUI);
			if (std::regex_search(s2, rex))
				r->retval.push_back(TDEVEUI(nid.devEUI));
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
		std::string s2 = std::string(nid.name, sizeof(DEVICENAME));
		if (std::regex_search(s2, rex))
			r->retval.push_back(TDEVEUI(nid.devEUI));
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

bool LmdbIdentityService::canControlService
(
	const DEVADDR &addr
)
{
	return false;
}

/**
 * Return next network address if available
 * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
 */
int LmdbIdentityService::next(NetworkIdentity &retval)
{
	uint32_t maxDevNwkAddr = getMaxDevNwkAddr(env);
    DevAddr nextAddr(netid, maxDevNwkAddr);
    if (nextAddr.increment())   // if reach last address
        return nextBruteForce(retval);  // try harder
    DEVADDRINT dai;
    nextAddr.get(dai);
	DeviceId check;
	if (get(check, (DEVADDR&) dai.a) != 0)
        return nextBruteForce(retval);
    nextAddr.get(retval.devaddr);
    return 0;
}

/**
  * Return next available network address
  * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
  */
int LmdbIdentityService::nextBruteForce(
    NetworkIdentity &retval
)
{
    int r;
    DevAddr nextAddr(netid, false);
    while (true) {
        DEVADDRINT dai;
		nextAddr.get(dai);
		DeviceId di;
		r = get(di, nextAddr.devaddr);
        if (r) {
            // not used. This is first free address. Return it
			retval.set(nextAddr.devaddr, di);
			return LORA_OK;
        }
        // go to next address
        r = nextAddr.increment();
        if (r) {
            // full of space
            return ERR_CODE_ADDR_SPACE_FULL;
        }
    }
    return LORA_OK; // never happens
}
