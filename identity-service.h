#ifndef IDENTITY_SERVICE_H_
#define IDENTITY_SERVICE_H_ 1

#include <map>
#include "utillora.h"

/**
 * Identity service interface
 * Get device identifier and keys by the network address
 */ 
class IdentityService {
protected:
    // LoraWAN network identifier
    NetId netid;
public:
    IdentityService();
    IdentityService(const IdentityService &value);
    virtual ~IdentityService();
    /**
    * get device identifier(w/o address) by network address. Return 0 if success, retval = EUI and keys
    * @param retval device identifier
    * @param devaddr network address
    * @return LORA_OK- success
    */
    virtual int get(DeviceId &retval, DEVADDR &devaddr) = 0;

    /**
    * get network identity(with address) by network address. Return 0 if success, retval = EUI and keys
    * @param retval network identity(with address)
    * @param eui device EUI
    * @return LORA_OK- success
    */
    virtual int getNetworkIdentity(NetworkIdentity &retval, const DEVEUI &eui) = 0;

    // Add or replace Address = EUI and keys pair
    virtual void put(DEVADDR &devaddr, DEVICEID &id) = 0;

    // Remove entry
    virtual void rm(DEVADDR &addr) = 0;

    /**
     * List entries
     * @param retval return values
     * @param offset 0..
     * @param size 0- all
     */

    virtual void list(std::vector<NetworkIdentity> &retval, size_t offset, size_t size) = 0;

    // Entries count
    virtual size_t size() = 0;

    // force save
    virtual void flush() = 0;

    // reload
    virtual int init(const std::string &option, void *data) = 0;

    // close resources
    virtual void done() = 0;

    // parseRX list of identifiers, wildcards or regexes and copy found EUI into retval
    virtual int parseIdentifiers(
            std::vector<TDEVEUI> &retval,
            const std::vector<std::string> &list,
            bool useRegex
    ) = 0;

    // parseRX list of names, wildcards or regexes and copy found EUI into retval
    virtual int parseNames(
            std::vector<TDEVEUI> &retval,
            const std::vector<std::string> &list,
            bool useRegex
    ) = 0;

    virtual bool canControlService(
            const DEVADDR &addr
    ) = 0;

    int joinAccept(
            JOIN_ACCEPT_FRAME_HEADER &retval,
            NetworkIdentity &networkIdentity
    );

    NetId * getNetworkId();
    void setNetworkId(const NetId &value);

    /**
     * Return next network address if available
     * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
     */
    virtual int next(NetworkIdentity &retval) = 0;
};

#endif
