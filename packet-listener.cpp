#include "packet-listener.h"
#include <iostream>
#include <sstream>

#include "utilstring.h"
#include "errlist.h"

#define DEF_BUFFER_SIZE     4096

PacketListener::PacketListener() :
    sysSignalPtr(nullptr), verbosity(0), stopped(false), onLog(nullptr), onGatewayStatDump(nullptr), gwStatEnv(nullptr),
    onDeviceStatDump(nullptr), deviceStatEnv(nullptr),
    handler(nullptr), identityService(nullptr), gatewayList(nullptr), deviceHistoryService(nullptr)
{
}

PacketListener::~PacketListener() {
	clear();
}

void PacketListener::setLogger(
    int aVerbosity,
    LogIntf *value
)
{
	verbosity = aVerbosity;
	onLog = value;
}

void PacketListener::setGatewayStatDumper(
	void *gwStatEnvVal,
	std::function<void(
		void *env,
		GatewayStat *gwStat
)> value)
{
	gwStatEnv = gwStatEnvVal;
	onGatewayStatDump = value;
}

void PacketListener::setDeviceStatDumper(
	void *deviceStatEnvVal,
	std::function<void(
		void *env,
		const SemtechUDPPacket &packet
)> value)
{
	deviceStatEnv = deviceStatEnvVal;
	onDeviceStatDump = value;
}

void PacketListener::setHandler(
	LoraPacketHandler *value
) {
	handler = value;
}

void PacketListener::setGatewayList(
	GatewayList *value
)
{
	gatewayList = value;
}

void PacketListener::setIdentityService
(
	IdentityService* value
)
{
	identityService = value;
}

void PacketListener::setDeviceHistoryService
(
	DeviceHistoryService *value
)
{
    deviceHistoryService = value;
}

/**
 * Set system signal last value pointer
 */
void PacketListener::setSysSignalPtr
(
	int *value
)
{
	sysSignalPtr = value;
}

void PacketListener::clear()
{
}

std::string PacketListener::toString() const {
    return "";
}

int PacketListener::add(
    const std::vector<std::string> &values,
    int hint
)
{
    int r = 0;
    for (std::vector<std::string>::const_iterator it(values.begin()); it != values.end(); it++) {
        if (!add(*it, hint)) {
            std::stringstream ss;
            ss << ERR_MESSAGE << ERR_CODE_SOCKET_BIND << ": " <<  ERR_SOCKET_BIND << *it << std::endl;
            onLog->logMessage(this, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_SOCKET_BIND, ss.str());
            exit(ERR_CODE_SOCKET_BIND);
        }
        r++;
    }
    return r;
}
