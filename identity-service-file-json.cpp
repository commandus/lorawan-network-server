#include <fstream>
#include <regex>
#include <sstream>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "identity-service-file-json.h"
#include "utilstring.h"
#include "errlist.h"

// end-device identity right flags
#define IDENTITY_FLAG_CAN_CONTROL_SERVICE	1

/**
 * 	JSON attribute names
 */
#define ATTRS_COUNT	14
static const char *ATTR_NAMES[ATTRS_COUNT] = {
    "addr", 		// 0 network address (hex string, 4 bytes)
    "activation",	// 1 ABP or OTAA
    "deveui",   	// 2 device identifier (hex string, 8 bytes)
    "nwkSKey",		// 3 shared session key (hex string, 16 bytes)
    "appSKey",		// 4 private key (hex string, 16 bytes)
    "class", 		// 5 A, B or C
    "version",		// 6 LoraWAN version
    "appeui",   	// 7 OTAA application identifier (JoinEUI) (hex string, 8 bytes)
    "appKey",   	// 8 Application key (hex string, 16 bytes)
    "nwkKey",   	// 9 Network key (hex string, 16 bytes)
    "devNonce",   	// 10 device identifier (hex string, 8 bytes)
    "joinNonce",   	// 11 device identifier (hex string, 8 bytes)
    "name",			// 12 added for search
    // not copied to the storage
    "flags"			// 13 if bit 0 is set it means allow control network service
};

static const char *ACTIVATION_NAMES[2] = {
    "ABP",
    "OTAA"
};

static int getAttrByName(
    const char *name
)
{
    int r = -1;
    for (int i = 0; i < ATTRS_COUNT; i++) {
        if (strcmp(ATTR_NAMES[i], name) == 0)
            return i;
    }
    return r;
}

static ACTIVATION getActivationByName(
    const char *name
)
{
    for (int i = 0; i < 2; i++) {
        if (strcmp(ACTIVATION_NAMES[i], name) == 0)
            return (ACTIVATION) i;
    }
    // default ABP
    return ABP;
}

static std::string getActivationName(
    ACTIVATION value
)
{
    if (value > OTAA)
        value = ABP;
    return ACTIVATION_NAMES[value];
}

JsonFileIdentityService::JsonFileIdentityService()
        : path(""), errcode(0), errmessage(""), maxDevNwkAddr(0)
{

}

JsonFileIdentityService::~JsonFileIdentityService()
{
    done();
}

/**
 * 
 * Loads NetworkIdentities
 *	[
 *		{
 *	 		"addr": "network address (hex string, 4 bytes)"
 * 			"eui": "device identifier (hex string, 8 bytes)",
 * 			"nwkSKey": "shared session key (hex string, 16 bytes)",
 *			"appSKey": "private key (hex string, 16 bytes)"
 *		},
 *		..
 *	]
 */
class RegionBandsJsonEmptyHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, RegionBandsJsonEmptyHandler> {
private:
    JsonFileIdentityService *service;
    bool isNetworkIdentity;
    int idx;
    DevAddr k;
    DEVICEID v;
    uint32_t flags;
public:
    RegionBandsJsonEmptyHandler(JsonFileIdentityService *svc)
            : service(svc), isNetworkIdentity(false), idx(-1)
    {
        memset(&k.devaddr, 0, sizeof(DEVADDR));
        memset(&v, 0, sizeof(DEVICEID));
        if (service) {
            // service->maxDevNwkAddr = 0;
            // service->clear();
        }
        flags = 0;
    }

    void resetEntry() {
        isNetworkIdentity = true;

        memset(&k.devaddr, '\0', sizeof(DEVADDR));
        memset(&v, '\0', sizeof(DEVICEID));
        v.version.major = 1;
        flags = 0;
    }

    bool Uint(unsigned u) {
        switch(idx) {
            case 13:
                flags = u;
                break;
        }
        return true;
    }

    bool String(const char* str, rapidjson::SizeType length, bool copy) {
        std::string s;
        /*
         * 0- addr 1- activation 2- eui 3- nwkSKey 4- appSKey 5- class 6- version, 7- name
         */
        switch(idx) {
            case 0:
                string2DEVADDR(k.devaddr, str);
                {
                    uint32_t na = k.getNwkAddr();
                    if (na > service->maxDevNwkAddr) {
                        // update helper data to remember last assigned address
                        service->maxDevNwkAddr = na;
                    }
                }
                break;
            case 1:
                v.activation = getActivationByName(str);
                break;
            case 2:
                string2DEVEUI(v.devEUI, str);
                break;
            case 3:
                s = hex2string(str);
                string2KEY(v.nwkSKey, s);
                break;
            case 4:
                s = hex2string(str);
                string2KEY(v.appSKey, s);
                break;
            case 5:
                v.deviceclass = string2deviceclass(str);
                break;
            case 6:
                v.version = string2LORAWAN_VERSION(str);
                break;
            case 7:
                string2DEVEUI(v.appEUI, str);
                break;
            case 8:
                s = hex2string(str);
                string2KEY(v.appKey, s);
                break;
            case 9:
                s = hex2string(str);
                string2KEY(v.nwkKey, s);
                break;
            case 10:
                v.devNonce = string2DEVNONCE(str);
                break;
            case 11:
                string2JOINNONCE(v.joinNonce, str);
                break;
            case 12:
                string2DEVICENAME(v.name, str);
                break;
            default:
                break;
        }
        return true;
    }
    bool StartObject() {
        resetEntry();
        return true;
    }

    bool Key(const char* str, rapidjson::SizeType length, bool copy) {
        idx = getAttrByName(str);
        return true;
    }
    bool EndObject(rapidjson::SizeType memberCount)
    {
        isNetworkIdentity = false;
        if (k.empty()) {
            // OTAA unassigned address, get a new one
            NetworkIdentity identity(v);
            int r = service->next(identity);
            if (r)
                return false;
            memmove(&k.devaddr, &identity.devaddr, sizeof(DEVADDR));
        }
        service->put(k.devaddr, v);
        service->setRightsMask(k.devaddr, flags);
        return true;
    }

    bool StartArray() {
        return true;
    }

    bool EndArray(rapidjson::SizeType elementCount) {
        return true;
    }
};

void JsonFileIdentityService::clear()
{
    storage.clear();
    maxDevNwkAddr = 0;
}

int JsonFileIdentityService::load()
{
    clear();
    RegionBandsJsonEmptyHandler handler(this);
    rapidjson::Reader reader;
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
        return ERR_CODE_INVALID_JSON;
    char readBuffer[4096];
    rapidjson::FileReadStream istrm(fp, readBuffer, sizeof(readBuffer));
    rapidjson::ParseResult r = reader.Parse<rapidjson::kParseCommentsFlag>(istrm, handler);
    if (r.IsError()) {
        errcode = r.Code();
        std::stringstream ss;
        ss << rapidjson::GetParseError_En(r.Code()) << " at " << r.Offset();
        errmessage = ss.str();
    } else {
        errcode = 0;
        errmessage = "";
    }
    fclose(fp);
    return r.IsError() ? ERR_CODE_INVALID_JSON : 0;
}

int JsonFileIdentityService::save()
{
    std::fstream os;
    os.open(path.c_str(), std::ios::out);
    os << "[" << std::endl;
    bool addSeparator(false);
    for (std::map<DEVADDRINT, DEVICEID>::const_iterator it = storage.begin(); it != storage.end(); it++) {
        if (addSeparator)
            os << ",";
        uint32_t addrRightsMask = getRightsMask((DEVADDR &) (it->first.a));
        os << std::endl << "{\""
           << ATTR_NAMES[0] << "\": \"" << DEVADDRINT2string(it->first) << "\",\""
           << ATTR_NAMES[1] << "\": \"" << getActivationName(it->second.activation) << "\",\""
           << ATTR_NAMES[2] << "\": \"" << DEVEUI2string(it->second.devEUI) << "\",\""
           << ATTR_NAMES[3] << "\": \"" << KEY2string(it->second.nwkSKey) << "\",\""
           << ATTR_NAMES[4] << "\": \"" << KEY2string(it->second.appSKey) << "\",\""
           << ATTR_NAMES[5] << "\": \"" << deviceclass2string(it->second.deviceclass) << "\",\""
           << ATTR_NAMES[6] << "\": \"" << LORAWAN_VERSION2string(it->second.version) << "\",\""
           << ATTR_NAMES[7] << "\": \"" << DEVEUI2string(it->second.appEUI) << "\",\""
           << ATTR_NAMES[8] << "\": \"" << KEY2string(it->second.appKey) << "\",\""
           << ATTR_NAMES[9] << "\": \"" << KEY2string(it->second.nwkKey) << "\",\""
           << ATTR_NAMES[10] << "\": \"" << DEVNONCE2string(it->second.devNonce) << "\",\""
           << ATTR_NAMES[11] << "\": \"" << JOINNONCE2string(it->second.joinNonce) << "\",\""
           << ATTR_NAMES[12] << "\": \"" << DEVICENAME2string(it->second.name) << "\"";
        if (addrRightsMask) {
            os << ",\""  << ATTR_NAMES[8] << "\": " << addrRightsMask;
        }
        os << "}";

        addSeparator = true;
    }
    os << "]" << std::endl;
    int r = os.bad() ? ERR_CODE_OPEN_DEVICE : 0;
    os.close();
    return r;
}

/**
 * get device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return LORA_OK- success
 */
int JsonFileIdentityService::get(DeviceId &retval, DEVADDR &devaddr)
{
    mutexMap.lock();
    std::map<DEVADDRINT, DEVICEID>::const_iterator it(storage.find(DEVADDRINT(devaddr)));
    if (it == storage.end()) {
        mutexMap.unlock();
        return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
    }
    retval.set(it->second);
    mutexMap.unlock();
    return 0;
}

/**
* get network identity(with address) by network address. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return LORA_OK- success
*/
int JsonFileIdentityService::getNetworkIdentity(
    NetworkIdentity &retval,
    const DEVEUI &eui
) {
    mutexMap.lock();
    for (std::map<DEVADDRINT, DEVICEID>::const_iterator it(storage.begin()); it != storage.end(); it++) {
        if (memcmp(&it->second.devEUI, &eui, sizeof(DEVEUI)) == 0) {
            retval.set(it->first, it->second);
            mutexMap.unlock();
            return 0;
        }
    }
    mutexMap.unlock();
    return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
}

// List entries
void JsonFileIdentityService::list(
        std::vector<NetworkIdentity> &retval,
        size_t offset,
        size_t size
) {
    int64_t c = -1;
    if (size == 0)
        size = SIZE_MAX;
    for (std::map<DEVADDRINT, DEVICEID>::const_iterator it(storage.begin()); it != storage.end(); it++) {
        c++;
        if (c < offset)
            continue;
        if (c >= size)
            break;
        NetworkIdentity v(it->first, it->second);
        retval.push_back(v);
    }
}

// Entries count
size_t JsonFileIdentityService::size()
{
    return storage.size();
}

void JsonFileIdentityService::put(
    DEVADDR &devaddr,
    DEVICEID &id
)
{
    mutexMap.lock();
    storage[devaddr] = id;
    mutexMap.unlock();

    DevAddr a(devaddr);
    uint32_t na = a.getNwkAddr();
    if (na > maxDevNwkAddr) {
        maxDevNwkAddr = na;
    }
}

void JsonFileIdentityService::rm(
    DEVADDR &addr
)
{
    mutexMap.lock();
    storage.erase(addr);
    mutexMap.unlock();
}

int JsonFileIdentityService::init(
    const std::string &option,
    void *data
)
{
    path = option;
    return load();
}

void JsonFileIdentityService::flush()
{
    save();
}

void JsonFileIdentityService::done()
{

}

int JsonFileIdentityService::parseIdentifiers(
    std::vector<TDEVEUI> &retval,
    const std::vector<std::string> &list,
    bool useRegex
) {
    for (std::vector<std::string>::const_iterator it(list.begin()); it != list.end(); it++) {
        if (isHex(*it)) {
            // identifier itself
            TDEVEUI v(*it);
            for (std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
                if (memcmp(v.eui, dit->second.devEUI, sizeof(DEVEUI)) == 0) {
                    retval.push_back(v);
                    break;
                }
            }
        } else {
            // can contain regex "*"
            try {
                std::string re;
                if (useRegex)
                    re = *it;
                else
                    re = replaceAll(replaceAll(*it, "*", ".*"), "?", ".");
                std::regex rex(re, std::regex_constants::grep);
                for (std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
                    std::string s2 = DEVEUI2string(dit->second.devEUI);
                    if (std::regex_search(s2, rex))
                        retval.push_back(TDEVEUI(dit->second.devEUI));
                }
            }
            catch (const std::regex_error& e) {
                return ERR_CODE_INVALID_REGEX;
            }
        }
    }
    return 0;
}

int JsonFileIdentityService::parseNames(
    std::vector<TDEVEUI> &retval,
    const std::vector<std::string> &list,
    bool useRegex
) {
    for (std::vector<std::string>::const_iterator it(list.begin()); it != list.end(); it++) {
        try {
            std::string re;
            if (useRegex)
                re = *it;
            else
                re = replaceAll(replaceAll(*it, "*", ".*"), "?", ".");
            std::regex rex(re, std::regex_constants::grep);
            for (std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
                std::string s2 = DEVICENAME2string(dit->second.name);
                if (std::regex_search(s2, rex))
                    retval.push_back(TDEVEUI(dit->second.devEUI));
            }
        }
        catch (const std::regex_error& e) {
            return ERR_CODE_INVALID_REGEX;
        }
    }
    return 0;
}

std::string JsonFileIdentityService::toJsonString()
{
    std::stringstream ss;
    ss << "[";
    bool needComma = false;
    for (std::map<DEVADDRINT, DEVICEID, DEVADDRINTCompare>::const_iterator dit(storage.begin()); dit != storage.end(); dit++) {
        if (needComma)
            ss << ", "  << std::endl;
        else
            needComma = true;
        ss << "{"
           << "\"" << ATTR_NAMES[0] << "\":\"" << DEVADDRINT2string(dit->first) << "\", "
           << "\"" << ATTR_NAMES[1] << "\":\"" << getActivationName(dit->second.activation) << "\", "
           << "\"" << ATTR_NAMES[2] << "\":\"" << DEVEUI2string(dit->second.devEUI) << "\", "
           << "\"" << ATTR_NAMES[3] << "\":\"" << KEY2string(dit->second.nwkSKey) << "\", "
           << "\"" << ATTR_NAMES[4] << "\":\"" << KEY2string(dit->second.appSKey) << "\", "
           << "\"" << ATTR_NAMES[5] << "\":\"" << deviceclass2string(dit->second.deviceclass) << "\", "
           << "\"" << ATTR_NAMES[6] << "\":\"" << LORAWAN_VERSION2string(dit->second.version) << "\", "
           << "\"" << ATTR_NAMES[7] << "\":\"" << DEVEUI2string(dit->second.appEUI) << "\", "
           << "\"" << ATTR_NAMES[8] << "\":\"" << KEY2string(dit->second.appKey) << "\", "
           << "\"" << ATTR_NAMES[9] << "\":\"" << KEY2string(dit->second.nwkKey) << "\", "
           << "\"" << ATTR_NAMES[10] << "\":\"" << DEVNONCE2string(dit->second.devNonce) << "\", "
           << "\"" << ATTR_NAMES[11] << "\":\"" << JOINNONCE2string(dit->second.joinNonce) << "\", "
           << "\"" << ATTR_NAMES[12] << "\":\"" << DEVICENAME2string(dit->second.name) << "\"";
        uint32_t rightsMask = getRightsMask((DEVADDR &) (dit->first.a));
        if (rightsMask)
            ss << ",\""  << ATTR_NAMES[8] << "\": " << rightsMask;
        ss << "}";
    }
    ss << "]";
    return ss.str();
}

uint32_t JsonFileIdentityService::getRightsMask
(
    const DEVADDR &addr
)
{
    std::map<DEVADDRINT, uint32_t>::const_iterator it(rightsMask.find(DEVADDRINT(addr)));
    if (it == rightsMask.end())
        return 0;
    uint32_t r = it->second;
    return r;
}

void JsonFileIdentityService::setRightsMask
(
    const DEVADDR &addr,
    uint32_t value
)
{
    if (value)
        rightsMask[DEVADDRINT(addr)] = value;
    else {
        // remove if exists
        std::map<DEVADDRINT, uint32_t>::iterator it(rightsMask.find(DEVADDRINT(addr)));
        if (it != rightsMask.end())
            rightsMask.erase(it);
    }

}

bool JsonFileIdentityService::canControlService
(
    const DEVADDR &addr
)
{
    return getRightsMask(addr) & IDENTITY_FLAG_CAN_CONTROL_SERVICE;
}

/**
  * Return next network address if available
  * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
  */
int JsonFileIdentityService::next(NetworkIdentity &retval)
{
    DevAddr nextAddr(netid, maxDevNwkAddr);
    if (nextAddr.increment())   // if reach last address
        return nextBruteForce(retval);  // try harder
    DEVADDRINT dai;
    nextAddr.get(dai);
    std::map<DEVADDRINT, DEVICEID>::const_iterator it = storage.find(dai);
    if (it != storage.end())
        return nextBruteForce(retval);
    nextAddr.get(retval.devaddr);
    return 0;
}

/**
  * Return next available network address
  * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
  */
int JsonFileIdentityService::nextBruteForce
(
    NetworkIdentity &retval
)
{
    int r = ERR_CODE_ADDR_SPACE_FULL;
    DevAddr nextAddr(netid, false);
    while(true) {
        DEVADDRINT dai;
        nextAddr.get(dai);
        std::map<DEVADDRINT, DEVICEID>::const_iterator it = storage.find(dai);
        if (it == storage.end()) {
            // not used. This is first free address. Return it
            nextAddr.get(retval.devaddr);
            r = LORA_OK;
            break;
        }
        // go to next address
        r = nextAddr.increment();
        if (r) {
            // full of space
            return r;
        }
    }
    return r;
}
