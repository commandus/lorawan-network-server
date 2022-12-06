#include "lorawan-mac.h"
#include <cstring>
#include <sstream>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <sys/time.h>
#endif


#include <iostream>
#include <iomanip>

#include "utildate.h"
#include "utilstring.h"
#include "lora-encrypt.h"

#include "errlist.h"

#define DEF_FREQUENCY_100 868900

/**
 * MAC command names
 */
static const std::string MAC_NAME[] =
{
	// Class-A
	"",
	"Reset",			// 1
	"LinkCheck",		// 2
	"LinkADR",			// 3
	"DutyCycle",		// 4
	"RXParamSetup",		// 5
	"DevStatus",		// 6
	"NewChannel",		// 7
	"RXTimingSetup",	// 8
	"TXParamSetup",		// 9
	"DLChannel",		// 0x0a
	"Rekey",			// 0x0b
	"ADRParamSetup",	// 0x0c
	"DeviceTime",		// 0x0d
	"ForceRejoin",		// 0x0e
	"RejoinParamSetup",	// 0x0f
	// Class-B Section 14
	"PingSlotInfo",		// 0x10
	"PingSlotChannel",	// 0x11
	// 0x12 has been deprecated in 1.1
	"BeaconTiming",		// 0x12,	//  Deprecated
	"BeaconFreq",		// 0x13,
};
/**
 * Extra MAC class C command name
 * "DeviceMode" 0x20
 */
static const std::string MAC_NAME_DEVICEMODE = "DeviceMode";
/**
 * Extended MAC command name and invalid MAC command names
 * 0x80 to 0xFF reserved for proprietary network command extensions
 */
static const std::string MAC_NAME_INVALID = "invalid";
static const std::string MAC_NAME_EXTENSION = "extension";

/**
 * @return MAC command name. If MAC command is extended, return 'extension'. Otherwise return 'invalid'
 */
const std::string& getMACCommandName(uint8_t command)
{
	if (command <= 0x13)
		return MAC_NAME[command];
	if (command <= 0x20)
		return MAC_NAME_DEVICEMODE;
	if (command >= 0x80)
		return MAC_NAME_EXTENSION;
	return MAC_NAME_INVALID;
}

#define SET_FREQUENCY(arr, value) \
	std::cerr << "set frequency: " << value << std::endl; \
	arr[0] = value & 0xff; \
	arr[1] = (value >> 8) & 0xff; \
	arr[2] = (value >> 16) & 0xff;

/**
 * @param cmd MAC command code
 * @return 0- known, 1- proprietary network command extensions, 2- invalid
 */
MAC_COMMAND_TYPE isMACCommand(uint8_t cmd)
{
	if (cmd >= 0x80)
		return MCT_PROPRIETARY;

	if ((cmd == 0) || ((cmd >= 0x14) && (cmd <= 0x1f)) || ((cmd >= 0x21) && (cmd <= 0x7f)))
		return MCT_INVALID;
	return MCT_KNOWN;
}

#define COPY_MAC(cn) \
		if (sz < MAC_##cn##_SIZE) \
			return ERR_CODE_MAC_TOO_SHORT; \
		r = MAC_##cn##_SIZE; \
		memmove(&retval, value, MAC_##cn##_SIZE);

#define PTR_MAC(cn) \
		if (sz < MAC_##cn##_SIZE) \
			return ERR_CODE_MAC_TOO_SHORT; \
		r = MAC_##cn##_SIZE; \
		if (retval) \
			*retval = (MAC_COMMAND*) value;


/**
 * Parse transmitted by the server client side
 * @param retval returns MAC command
 * @param value packet to parseRX
 * @return size of first MAC command, if error, return <0
 */
int parseClientSide(
	MAC_COMMAND &retval,
	const char* value,
	size_t sz
)
{
	if (sz < 1)
		return ERR_CODE_MAC_TOO_SHORT;
	int r = 0;
	switch ((uint8_t) value[0]) {
	case Reset:	// req
		COPY_MAC(RESET)	// same request and response
		break;
	case LinkCheck:
		COPY_MAC(LINK_CHECK)
		break;
	case LinkADR:
		COPY_MAC(LINK_ADR_REQ)
		break;
	case DutyCycle:
		COPY_MAC(DUTY_CYCLE)	// same request and response
		break;
	case RXParamSetup:
		COPY_MAC(RXRARAMSETUP_REQ)
		break;
	case DevStatus:
		COPY_MAC(EMPTY)
		break;
	case NewChannel:
		COPY_MAC(NEWCHANNEL_REQ)
		break;
	case RXTimingSetup:
		COPY_MAC(TIMINGSETUP)
		break;
	case TXParamSetup:
		COPY_MAC(TXPARAMSETUP)
		break;
	case DLChannel:
		COPY_MAC(DLCHANNEL_REQ)
		break;
	case Rekey:
		COPY_MAC(REKEY_RESP)
		break;
	case ADRParamSetup:
		COPY_MAC(ADRPARAMSETUP) 
		break;
	case DeviceTime:
		COPY_MAC(DEVICETIME)
		break;
	case ForceRejoin:
		COPY_MAC(FORCEREJOIN)
		break;
	case RejoinParamSetup:
		COPY_MAC(REJOINPARAMSETUP_REQ)
		break;
	// Class-B Section 14
	case PingSlotInfo:
		COPY_MAC(EMPTY)
		break;
	case PingSlotChannel:
		COPY_MAC(PINGSLOTCHANNEL_REQ)
		break;
	// 0x12 has been deprecated in 1.1
	case BeaconTiming:
		COPY_MAC(BEACONTIMING)
		break;
	case BeaconFreq:
		COPY_MAC(BEACONFREQUENCY_REQ)
		break;
	// Class-C
	case DeviceMode:
		COPY_MAC(DEVICEMODE)	// same
		break;
	default:
		return ERR_CODE_MAC_INVALID;
	}
	return r;
}

/**
 * Parse MAC command transmitted by end-device (server side)
 * @param retval returns MAC command
 * @param value packet to parseRX
 * @param sz packet size
 * @return size of first MAC command, if error, return <0
 */
int parseServerSide(
	MAC_COMMAND &retval,
	const char* value,
	size_t sz
)
{
	if (sz < 1)
		return ERR_CODE_MAC_TOO_SHORT;
	int r = 0;
	switch ((uint8_t) value[0]) {
	case Reset:	// req
		COPY_MAC(RESET)	// same request and response
		break;
	case LinkCheck:
		COPY_MAC(EMPTY)
		break;
	case LinkADR:
		COPY_MAC(LINK_ADR_RESP)
		break;
	case DutyCycle:
		COPY_MAC(EMPTY)
		break;
	case RXParamSetup:
		COPY_MAC(RXRARAMSETUP_RESP)
		break;
	case DevStatus:
		COPY_MAC(DEVSTATUS)
		break;
	case NewChannel:
		COPY_MAC(NEWCHANNEL_RESP)
		break;
	case RXTimingSetup:
		COPY_MAC(EMPTY)
		break;
	case TXParamSetup:
		COPY_MAC(EMPTY)
		break;
	case DLChannel:
		COPY_MAC(DLCHANNEL_RESP)
		break;
	case Rekey:
		COPY_MAC(REKEY_REQ)
		break;
	case ADRParamSetup:
		COPY_MAC(EMPTY) 
		break;
	case DeviceTime:
		COPY_MAC(EMPTY)
		break;
	case ForceRejoin:
		COPY_MAC(EMPTY)	// actually no response, it is re-join request
		break;
	case RejoinParamSetup:
		COPY_MAC(REJOINPARAMSETUP_RESP)
		break;
	// Class-B Section 14
	case PingSlotInfo:
		COPY_MAC(PINGSLOTINFO)
		break;
	case PingSlotChannel:
		COPY_MAC(PINGSLOTCHANNEL_RESP)
		break;
	// 0x12 has been deprecated in 1.1
	case BeaconTiming:
		COPY_MAC(EMPTY)
		break;
	case BeaconFreq:
		COPY_MAC(BEACONFREQUENCY_RESP)
		break;
	// Class-C
	case DeviceMode:
		COPY_MAC(DEVICEMODE)	// same
		break;
	default:
		return ERR_CODE_MAC_INVALID;
	}
	return r;
}
// -----------------------------------------

/**
 * Parse transmitted by the server client side
 * @param retval if not NULL, return pointer to MAC command
 * @param value packet to parseRX
 * @param sz packet size
 * @return size of first MAC command, if error, return <0
 */
int parseClientSidePtr(
	MAC_COMMAND **retval,
	const char* value,
	size_t sz
)
{
	if (sz < 1)
		return ERR_CODE_MAC_TOO_SHORT;
	int r = 0;
	switch ((uint8_t) *value) {
	case Reset:	// req
		PTR_MAC(RESET)	// same request and response
		break;
	case LinkCheck:
		PTR_MAC(LINK_CHECK)
		break;
	case LinkADR:
		PTR_MAC(LINK_ADR_REQ)
		break;
	case DutyCycle:
		PTR_MAC(DUTY_CYCLE)	// same request and response
		break;
	case RXParamSetup:
		PTR_MAC(RXRARAMSETUP_REQ)
		break;
	case DevStatus:
		PTR_MAC(EMPTY)
		break;
	case NewChannel:
		PTR_MAC(NEWCHANNEL_REQ)
		break;
	case RXTimingSetup:
		PTR_MAC(TIMINGSETUP)
		break;
	case TXParamSetup:
		PTR_MAC(TXPARAMSETUP)
		break;
	case DLChannel:
		PTR_MAC(DLCHANNEL_REQ)
		break;
	case Rekey:
		PTR_MAC(REKEY_RESP)
		break;
	case ADRParamSetup:
		PTR_MAC(ADRPARAMSETUP) 
		break;
	case DeviceTime:
		PTR_MAC(DEVICETIME)
		break;
	case ForceRejoin:
		PTR_MAC(FORCEREJOIN)
		break;
	case RejoinParamSetup:
		PTR_MAC(REJOINPARAMSETUP_REQ)
		break;
	// Class-B Section 14
	case PingSlotInfo:
		PTR_MAC(EMPTY)
		break;
	case PingSlotChannel:
		PTR_MAC(PINGSLOTCHANNEL_REQ)
		break;
	// 0x12 has been deprecated in 1.1
	case BeaconTiming:
		PTR_MAC(BEACONTIMING)
		break;
	case BeaconFreq:
		PTR_MAC(BEACONFREQUENCY_REQ)
		break;
	// Class-C
	case DeviceMode:
		PTR_MAC(DEVICEMODE)	// same
		break;
	default:
		return ERR_CODE_MAC_INVALID;
	}
	return r;
}

/**
 * Parse MAC command transmitted by end-device (server side)
 * @param retval if not NULL, return pointer to MAC command
 * @param value packet to parseRX
 * @param sz packet size
 * @return size of first MAC command, if error, return <0
 */
int parseServerSidePtr(
	MAC_COMMAND **retval,
	const char* value,
	size_t sz
)
{
	if (sz < 1)
		return ERR_CODE_MAC_TOO_SHORT;
	int r = 0;
	switch ((uint8_t) *value) {
	case Reset:	// req
		PTR_MAC(RESET)	// same request and response
		break;
	case LinkCheck:
		PTR_MAC(EMPTY)
		break;
	case LinkADR:
		PTR_MAC(LINK_ADR_RESP)
		break;
	case DutyCycle:
		PTR_MAC(EMPTY)
		break;
	case RXParamSetup:
		PTR_MAC(RXRARAMSETUP_RESP)
		break;
	case DevStatus:
		PTR_MAC(DEVSTATUS)
		break;
	case NewChannel:
		PTR_MAC(NEWCHANNEL_RESP)
		break;
	case RXTimingSetup:
		PTR_MAC(EMPTY)
		break;
	case TXParamSetup:
		PTR_MAC(EMPTY)
		break;
	case DLChannel:
		PTR_MAC(DLCHANNEL_RESP)
		break;
	case Rekey:
		PTR_MAC(REKEY_REQ)
		break;
	case ADRParamSetup:
		PTR_MAC(EMPTY) 
		break;
	case DeviceTime:
		PTR_MAC(EMPTY)
		break;
	case ForceRejoin:
		PTR_MAC(EMPTY)	// actually no response, it is re-join request
		break;
	case RejoinParamSetup:
		PTR_MAC(REJOINPARAMSETUP_RESP)
		break;
	// Class-B Section 14
	case PingSlotInfo:
		PTR_MAC(PINGSLOTINFO)
		break;
	case PingSlotChannel:
		PTR_MAC(PINGSLOTCHANNEL_RESP)
		break;
	// 0x12 has been deprecated in 1.1
	case BeaconTiming:
		PTR_MAC(EMPTY)
		break;
	case BeaconFreq:
		PTR_MAC(BEACONFREQUENCY_RESP)
		break;
	// Class-C
	case DeviceMode:
		PTR_MAC(DEVICEMODE)	// same
		break;
	default:
		return ERR_CODE_MAC_INVALID;
	}
	return r;
}

// ---------------- MacData ----------------

/**
 * Create empty MAC
 */
MacData::MacData()
	: errcode(0), isClientSide(false)
{
	memset(&command, 0, sizeof(MAC_COMMAND));
}

/**
 * Copy MAC
 */
MacData::MacData(
	const MacData &value
)
	: errcode(value.errcode), isClientSide(value.isClientSide)
{
	memmove(&command, &value.command, sizeof(MAC_COMMAND));
}

MacData::MacData(
	MAC_COMMAND &value
) 
	: errcode(0), isClientSide(false)
{
	memmove(&command, &value, sizeof(MAC_COMMAND));
}

MacData::MacData(
	const std::string &value,
	const bool sentByGateway
)
	: isClientSide(sentByGateway)
{
	int r;
	if (isClientSide)
		r = parseClientSide(command, value.c_str(), value.size());
	else
		r = parseServerSide(command, value.c_str(), value.size());

	if (r < 0)
		errcode = r;
	else
		errcode = 0;
}

/**
 * Set MAC command from array of paramaters
 * @param cid MAC command identifier
 * @param values MAC command paramaters in array
 * @return true if success
 */
bool MacData::set(
	enum MAC_CID cid,
	const std::vector <int> &values,
	bool clientSide
) {
	command.command = cid;
	isClientSide = clientSide;

	if (clientSide)	{
		// end device side (sent by gateway)
		switch (cid) {
			case Reset:	// req
				command.data.reset.rfu = 0;			// not used
				command.data.reset.minor = 1;		// LoRaWAN x.1
				break;
			case LinkCheck:
				if (values.size() < 2)
					break;
				command.data.linkcheck.margin = values[0];
				command.data.linkcheck.gwcnt = values[1];
				break;
			case LinkADR:
				if (values.size() < 5)
					break;
				command.data.linkadrreq.txpower = values[0];
				command.data.linkadrreq.datarate = values[1];
				command.data.linkadrreq.chmask = values[2];
				command.data.linkadrreq.nbtans = values[3];
				command.data.linkadrreq.chmaskcntl = values[4];
				command.data.linkadrreq.rfu = 0;
				break;
			case DutyCycle:
				if (values.size() < 1)
					break;
				command.data.dutycycle.maxdccycle = values[0];
				command.data.dutycycle.rfu = 0;
				break;
			case RXParamSetup:
				if (values.size() < 3)
					break;
				SET_FREQUENCY(command.data.rxparamsetupreq.frequency, values[0])
				command.data.rxparamsetupreq.rx1droffset = values[1];
				command.data.rxparamsetupreq.rx2datatrate = values[2];
				command.data.rxparamsetupreq.rfu = 0;
				break;
			case DevStatus:
				break;
			case NewChannel:
				if (values.size() < 4)
					break;
				command.data.newchacnnelreq.chindex = values[0];
				SET_FREQUENCY(command.data.newchacnnelreq.frequency, values[1])
				command.data.newchacnnelreq.mindr = values[2];
				command.data.newchacnnelreq.maxdr = values[3];
				break;
			case RXTimingSetup:
				if (values.size() < 1)
					break;
				command.data.timingsetup.delay = values[0];
				command.data.timingsetup.rfu = 0;
				break;
			case TXParamSetup:
				if (values.size() < 3)
					break;
				command.data.txparamsetup.downlinkdwelltime = values[0];
				command.data.txparamsetup.uplinkdwelltime = values[1];
				command.data.txparamsetup.maxeirp = values[2];
				command.data.txparamsetup.rfu = 0;
				break;
			case DLChannel:
				if (values.size() < 2)
					break;
				command.data.dlcchannelreq.chindex = values[0];
				SET_FREQUENCY(command.data.dlcchannelreq.frequency, values[1])
				break;
			case Rekey:
				command.data.rekeyreq.minor = 1;		// LoRaWAN x.1
				command.data.rekeyreq.rfu = 0;			// not used
				break;
			case ADRParamSetup:
				if (values.size() < 2)
					break;
				command.data.adrparamsetup.limitexp = values[0];
				command.data.adrparamsetup.delayexp = values[1];
				break;
			case DeviceTime:
				{
					struct timeval t;
					gettimeofday(&t, NULL);
					command.data.devicetime.gpstime = utc2gps(t.tv_sec);
					command.data.devicetime.frac = (uint8_t) t.tv_usec;
				}
				break;
			case ForceRejoin:
				if (values.size() < 3)
					break;
				command.data.forcerejoin.period = values[0];
				command.data.forcerejoin.maxretries = values[1];
				command.data.forcerejoin.rejointype = values[2];
				command.data.forcerejoin.rfu = 0;
				command.data.forcerejoin.rfu2 = 0;
				break;
			case RejoinParamSetup:
				if (values.size() < 2)
					break;
				command.data.rejoinparamsetupreq.maxtime = values[0];
				command.data.rejoinparamsetupreq.maxccount = values[1];
				break;
			// Class-B Section 14
			case PingSlotInfo:
				break;
			case PingSlotChannel:
				if (values.size() < 2)
					break;
				SET_FREQUENCY(command.data.pingslotchannelreq.frequency, values[0])
				command.data.pingslotchannelreq.dr = values[1];
				command.data.pingslotchannelreq.rfu = 0;
				break;
			// 0x12 has been deprecated in 1.1
			case BeaconTiming:
				if (values.size() < 2)
					break;
				command.data.beacontiming.delay = values[0];
				command.data.beacontiming.channel = values[1];
				break;
			case BeaconFreq:
				if (values.size() < 1)
					break;
				SET_FREQUENCY(command.data.beaconfrequencyreq.frequency, values[0])
				break;
			// Class-C
			case DeviceMode:
				if (values.size() < 1)
					break;
				command.data.devicemode.cl = values[0];
				break;
			default:
				break;
		}
	} else {
		// server-side (sent by end-device)
		switch (cid) {
			case Reset:	// req
				command.data.reset.rfu = 0;			// not used
				command.data.reset.minor = 1;		// LoRaWAN x.1
				break;
			case LinkCheck:
				break;
			case LinkADR:
				if (values.size() < 3)
					break;
				command.data.linkadrresp.powerack = values[0];
				command.data.linkadrresp.datarateack = values[1];
				command.data.linkadrresp.channelmaskack = values[2];
				command.data.linkadrresp.rfu = 0;
				break;
			case DutyCycle:
				break;
			case RXParamSetup:
				if (values.size() < 3)
					break;
				command.data.rxparamsetupresp.rx1droffsetack = values[0];
				command.data.rxparamsetupresp.rx2datatrateack = values[1];
				command.data.rxparamsetupresp.channelack = values[2];
				command.data.rxparamsetupresp.rfu = 0;
				break;
			case DevStatus:
				if (values.size() < 2)
					break;
				command.data.devstatus.battery = values[0];
				command.data.devstatus.margin = values[1];
				break;
			case NewChannel:
				if (values.size() < 2)
					break;
				command.data.newchacnnelresp.channelfrequencyack = values[0];
				command.data.newchacnnelresp.datarateack = values[1];
				command.data.newchacnnelresp.rfu = 0;
				break;
			case RXTimingSetup:
				break;
			case TXParamSetup:
				break;
			case DLChannel:
				if (values.size() < 2)
					break;
				command.data.dlcchannelresp.channelfrequencyack = values[0];
				command.data.dlcchannelresp.uplinkfrequencyexistsack = values[1];
				command.data.dlcchannelresp.rfu = 0;
				break;
			case Rekey:
				command.data.rekeyreq.minor = 1;		// LoRaWAN x.1
				command.data.rekeyreq.rfu = 0;			// not used
				break;
			case ADRParamSetup:
				break;
			case DeviceTime:
				break;
			case ForceRejoin:
				break;
			case RejoinParamSetup:
				if (values.size() < 1)
					break;
				command.data.rejoinparamsetupresp.timeack = values[0];
				command.data.rejoinparamsetupresp.rfu = 0;
				break;
			// Class-B Section 14
			case PingSlotInfo:
				if (values.size() < 1)
					break;
				command.data.pinginfoslot.periodicity = values[0];
				command.data.pinginfoslot.rfu = 0;
				break;
			case PingSlotChannel:
				if (values.size() < 2)
					break;
				command.data.pingslotchannelresp.drack = values[0];
				command.data.pingslotchannelresp.frequencyack = values[1];
				command.data.pingslotchannelresp.rfu = 0;
				break;
			// 0x12 has been deprecated in 1.1
			case BeaconTiming:
				break;
			case BeaconFreq:
				if (values.size() < 1)
					break;
				command.data.beaconfrequencyresp.frequencyack = values[0];
				command.data.beaconfrequencyresp.rfu = 0;
				break;
			// Class-C
			case DeviceMode:
				if (values.size() < 1)
					break;
				command.data.devicemode.cl = values[0];
				break;
			default:
				break;
			}
	}
	return true;
}

/**
 * Return copy of MAC as string 
 */
std::string MacData::toString() const
{
	return std::string((const char *) &this->command, size());
}

/**
 * Return hex repesentation of MAC
 */
std::string MacData::toHexString() const
{
	std::string r((const char *) &this->command, size());
	return hexString(r);
}

#define MDPREFIX(CMD) \
	ss << "\""#CMD"\": {";
#define MDSUFFIX(CMD) \
	ss << "}";
#define MD2JSONSS(CMD, FLD) \
	ss << "\""#FLD"\": " << (int) command.data.CMD.FLD;
#define MD2JSONSSCOMMA(CMD, FLD) \
	ss << "\""#FLD"\": " << (int) command.data.CMD.FLD << ", ";
#define MD2JSONSS_FREQUENCY(CMD) \
	ss << "\"frequency\": " << (int) (\
		command.data.CMD.frequency[0] + \
		(command.data.CMD.frequency[1] << 8) + \
		(command.data.CMD.frequency[2] << 16));
#define MD2JSONSS_COMMA() \
	ss << ", ";

/**
 * Serialize MAC command as JSON string.
 * Static method. 
 * @param command MAC command
 */
std::string MAC_DATA2JSONString(
	const MAC_COMMAND &command,
	const bool isClientSide 
)
{
	std::stringstream ss;
	if (isClientSide)	{
		// end device side (sent by network)
		switch (command.command) {
			case Reset:	// req
				MDPREFIX(reset)
				MD2JSONSSCOMMA(reset, rfu)
				MD2JSONSS(reset, minor)
				MDSUFFIX(reset)
				break;
			case LinkCheck:
				MDPREFIX(linkcheck)
				MD2JSONSSCOMMA(linkcheck, margin)
				MD2JSONSS(linkcheck, gwcnt)
				MDSUFFIX(linkcheck)
				break;
			case LinkADR:
				MDPREFIX(linkadrreq)
				MD2JSONSSCOMMA(linkadrreq, txpower)
				MD2JSONSSCOMMA(linkadrreq, datarate)
				MD2JSONSSCOMMA(linkadrreq, chmask)
				MD2JSONSSCOMMA(linkadrreq, nbtans)
				MD2JSONSSCOMMA(linkadrreq, chmaskcntl)
				MD2JSONSS(linkadrreq, rfu)
				MDSUFFIX(linkadrreq)
				break;
			case DutyCycle:
				MDPREFIX(dutycycle)
				MD2JSONSSCOMMA(dutycycle, maxdccycle)
				MD2JSONSS(dutycycle, rfu)
				MDSUFFIX(dutycycle)
				break;
			case RXParamSetup:
				MDPREFIX(rxparamsetupreq)
				MD2JSONSS_FREQUENCY(rxparamsetupreq) ss << ", ";
				MD2JSONSSCOMMA(rxparamsetupreq, rx1droffset)
				MD2JSONSSCOMMA(rxparamsetupreq, rx2datatrate)
				MD2JSONSS(rxparamsetupreq, rfu)
				MDSUFFIX(rxparamsetupreq)
				break;
			case DevStatus:
				MDPREFIX(devstatus)
				MDSUFFIX(devstatus)
				break;
			case NewChannel:
				MDPREFIX(newchacnnelreq)
				MD2JSONSSCOMMA(newchacnnelreq, chindex)
				MD2JSONSS_FREQUENCY(newchacnnelreq) ss << ", ";
				MD2JSONSSCOMMA(newchacnnelreq, mindr)
				MD2JSONSS(newchacnnelreq, maxdr)
				MDSUFFIX(newchacnnelreq)
				break;
			case RXTimingSetup:
				MDPREFIX(timingsetup)
				MD2JSONSSCOMMA(timingsetup, delay)
				MD2JSONSS(timingsetup, rfu)
				MDSUFFIX(timingsetup)
				break;
			case TXParamSetup:
				MDPREFIX(txparamsetup)
				MD2JSONSSCOMMA(txparamsetup, downlinkdwelltime)
				MD2JSONSSCOMMA(txparamsetup, uplinkdwelltime)
				MD2JSONSSCOMMA(txparamsetup, maxeirp)
				MD2JSONSS(txparamsetup, rfu)
				MDSUFFIX(txparamsetup)
				break;
			case DLChannel:
				MDPREFIX(dlcchannelreq)
				MD2JSONSSCOMMA(dlcchannelreq, chindex)
				MD2JSONSS_FREQUENCY(dlcchannelreq)
				MDSUFFIX(dlcchannelreq)
				break;
			case Rekey:
				MDPREFIX(rekeyreq)
				MD2JSONSSCOMMA(rekeyreq, minor)
				MD2JSONSS(rekeyreq, rfu)
				MDSUFFIX(rekeyreq)
				break;
			case ADRParamSetup:
				MDPREFIX(adrparamsetup)
				MD2JSONSSCOMMA(adrparamsetup, limitexp)
				MD2JSONSS(adrparamsetup, delayexp)
				MDSUFFIX(adrparamsetup)
				break;
			case DeviceTime:
				MDPREFIX(devicetime)
				MD2JSONSSCOMMA(devicetime, gpstime)
				MD2JSONSS(devicetime, frac)
				MDSUFFIX(devicetime)
				break;
			case ForceRejoin:
				MDPREFIX(forcerejoin)
				MD2JSONSSCOMMA(forcerejoin, period)
				MD2JSONSSCOMMA(forcerejoin, maxretries)
				MD2JSONSSCOMMA(forcerejoin, rejointype)
				MD2JSONSSCOMMA(forcerejoin, rfu)
				MD2JSONSS(forcerejoin, rfu2)
				MDSUFFIX(devicetime)
				break;
			case RejoinParamSetup:
				MDPREFIX(rejoinparamsetupreq)
				MD2JSONSSCOMMA(rejoinparamsetupreq, maxtime)
				MD2JSONSS(rejoinparamsetupreq, maxccount)
				MDSUFFIX(rejoinparamsetupreq)
				break;
			// Class-B Section 14
			case PingSlotInfo:
				MDPREFIX(pinginfoslot)
				MDSUFFIX(pinginfoslot)
				break;
			case PingSlotChannel:
				MDPREFIX(pingslotchannelreq)
				MD2JSONSS_FREQUENCY(pingslotchannelreq) ss << ", ";
				MD2JSONSSCOMMA(pingslotchannelreq, dr)
				MD2JSONSS(pingslotchannelreq, rfu)
				MDSUFFIX(pingslotchannelreq)
				break;
			// 0x12 has been deprecated in 1.1
			case BeaconTiming:
				MDPREFIX(beacontiming)
				MD2JSONSSCOMMA(beacontiming, delay)
				MD2JSONSS(beacontiming, channel)
				MDSUFFIX(beacontiming)
				break;
			case BeaconFreq:
				MDPREFIX(beaconfrequencyreq)
				MD2JSONSS_FREQUENCY(beaconfrequencyreq)
				MDSUFFIX(beaconfrequencyreq)
				break;
			// Class-C
			case DeviceMode:
				MDPREFIX(devicemode)
				MD2JSONSS(devicemode, cl)
				MDSUFFIX(devicemode)
				break;
			default:
				break;
		}
	} else {
		// server-side (sent by end-device)
		switch (command.command) {
			case Reset:	// req
				MDPREFIX(reset)
				MD2JSONSSCOMMA(reset, minor)
				MD2JSONSS(reset, rfu)
				MDSUFFIX(reset)
				break;
			case LinkCheck:
				MDPREFIX(linkcheck)
				MDSUFFIX(linkcheck)
				break;
			case LinkADR:
				MDPREFIX(linkadrresp)
				MD2JSONSSCOMMA(linkadrresp, powerack)
				MD2JSONSSCOMMA(linkadrresp, datarateack)
				MD2JSONSSCOMMA(linkadrresp, channelmaskack)
				MD2JSONSS(linkadrresp, rfu)
				MDSUFFIX(linkadrresp)
				break;
			case DutyCycle:
				MDPREFIX(dutycycle)
				MDSUFFIX(dutycycle)
				break;
			case RXParamSetup:
				MDPREFIX(rxparamsetupresp)
				MD2JSONSSCOMMA(rxparamsetupresp, rx1droffsetack)
				MD2JSONSSCOMMA(rxparamsetupresp, rx2datatrateack)
				MD2JSONSSCOMMA(rxparamsetupresp, channelack)
				MD2JSONSS(rxparamsetupresp, rfu)
				MDSUFFIX(rxparamsetupresp)
				break;
			case DevStatus:
				MDPREFIX(devstatus)
				MD2JSONSSCOMMA(devstatus, battery)
				MD2JSONSS(devstatus, margin)
				MDSUFFIX(devstatus)
				break;
			case NewChannel:
				MDPREFIX(newchacnnelresp)
				MD2JSONSSCOMMA(newchacnnelresp, channelfrequencyack)
				MD2JSONSSCOMMA(newchacnnelresp, channelfrequencyack)
				MD2JSONSS(newchacnnelresp, rfu)
				MDSUFFIX(newchacnnelresp)
				break;
			case RXTimingSetup:
				MDPREFIX(rxtimingsetup)
				MDSUFFIX(rxtimingsetup)
				break;
			case TXParamSetup:
				MDPREFIX(txtimingsetup)
				MDSUFFIX(txtimingsetup)
				break;
			case DLChannel:
				MDPREFIX(dlcchannelresp)
				MD2JSONSSCOMMA(dlcchannelresp, channelfrequencyack)
				MD2JSONSSCOMMA(dlcchannelresp, uplinkfrequencyexistsack)
				MD2JSONSS(dlcchannelresp, rfu)
				MDSUFFIX(dlcchannelresp)
				break;
			case Rekey:
				MDPREFIX(rekeyreq)
				MD2JSONSSCOMMA(rekeyreq, minor)
				MD2JSONSS(rekeyreq, rfu)
				MDSUFFIX(rekeyreq)
				break;
			case ADRParamSetup:
				MDPREFIX(adrparamsetup)
				MDSUFFIX(adrparamsetup)
				break;
			case DeviceTime:
				MDPREFIX(devicetime)
				MDSUFFIX(devicetime)
				break;
			case ForceRejoin:
				MDPREFIX(forcerejoin)
				MDSUFFIX(forcerejoin)
				break;
			case RejoinParamSetup:
				MDPREFIX(rejoinparamsetupresp)
				MD2JSONSSCOMMA(rejoinparamsetupresp, timeack)
				MD2JSONSS(rejoinparamsetupresp, rfu)
				MDSUFFIX(rejoinparamsetupresp)
				break;
			// Class-B Section 14
			case PingSlotInfo:
				MDPREFIX(pinginfoslot)
				MD2JSONSSCOMMA(pinginfoslot, periodicity)
				MD2JSONSS(pinginfoslot, rfu)
				MDSUFFIX(pinginfoslot)
				break;
			case PingSlotChannel:
				MDPREFIX(pingslotchannelresp)
				MD2JSONSSCOMMA(pingslotchannelresp, drack)
				MD2JSONSSCOMMA(pingslotchannelresp, frequencyack)
				MD2JSONSS(pingslotchannelresp, rfu)
				MDSUFFIX(pingslotchannelresp)
				break;
			// 0x12 has been deprecated in 1.1
			case BeaconTiming:
				MDPREFIX(beacontiming)
				MDSUFFIX(beacontiming)
				break;
			case BeaconFreq:
				MDPREFIX(beaconfrequencyresp)
				MD2JSONSS(beaconfrequencyresp, frequencyack)
				MDSUFFIX(beaconfrequencyresp)
				break;
			// Class-C
			case DeviceMode:
				MDPREFIX(devicemode)
				MD2JSONSS(devicemode, cl)
				MDSUFFIX(devicemode)
				break;
			default:
				break;
			}
	}
	return ss.str();
}

/**
 * Serialize MAC command as JSON string
 */
std::string MacData::toJSONString() const
{
	return MAC_DATA2JSONString(command, isClientSide);
}

/**
 * Return MAC command size
 * Static method.
 * @param value MAC command
 */
size_t commandSize(
	const MAC_COMMAND &value,
	bool clientSide
)
{
	if (clientSide)	{
		// sent by gateway
		switch (value.command) {
		case Reset:	// req
			return MAC_RESET_SIZE;
		case LinkCheck:
			return MAC_LINK_CHECK_SIZE;
		case LinkADR:
			return MAC_LINK_ADR_REQ_SIZE;
		case DutyCycle:
			return MAC_DUTY_CYCLE_SIZE;
		case RXParamSetup:
			return MAC_RXRARAMSETUP_REQ_SIZE;
		case DevStatus:
			return MAC_EMPTY_SIZE;
		case NewChannel:
			return MAC_NEWCHANNEL_REQ_SIZE;
		case RXTimingSetup:
			return MAC_TIMINGSETUP_SIZE;
		case TXParamSetup:
			return MAC_TXPARAMSETUP_SIZE;
		case DLChannel:
			return MAC_DLCHANNEL_REQ_SIZE;
		case Rekey:
			return MAC_REKEY_RESP_SIZE;
		case ADRParamSetup:
			return MAC_ADRPARAMSETUP_SIZE;
		case DeviceTime:
			return MAC_DEVICETIME_SIZE;
		case ForceRejoin:
			return MAC_FORCEREJOIN_SIZE;
		case RejoinParamSetup:
			return MAC_REJOINPARAMSETUP_REQ_SIZE;
		// Class-B Section 14
		case PingSlotInfo:
			return MAC_EMPTY_SIZE;
		case PingSlotChannel:
			return MAC_PINGSLOTCHANNEL_REQ_SIZE;
		// 0x12 has been deprecated in 1.1
		case BeaconTiming:
			return MAC_BEACONTIMING_SIZE;
		case BeaconFreq:
			return MAC_BEACONFREQUENCY_REQ_SIZE;
		// Class-C
		case DeviceMode:
			return MAC_DEVICEMODE_SIZE;

		default:
			return 0;
		}
	} else {
		// server-side (sent by end-device)
		switch (value.command) {
			case Reset:	// req
				return MAC_RESET_SIZE;
			case LinkCheck:
				return MAC_EMPTY_SIZE;
			case LinkADR:
				return MAC_LINK_ADR_RESP_SIZE;
			case DutyCycle:
				return MAC_EMPTY_SIZE;
			case RXParamSetup:
				return MAC_RXRARAMSETUP_RESP_SIZE;
			case DevStatus:
				return MAC_DEVSTATUS_SIZE;
			case NewChannel:
				return MAC_NEWCHANNEL_RESP_SIZE;
			case RXTimingSetup:
				return MAC_EMPTY_SIZE;
			case TXParamSetup:
				return MAC_EMPTY_SIZE;
			case DLChannel:
				return MAC_DLCHANNEL_RESP_SIZE;
			case Rekey:
				return MAC_REKEY_REQ_SIZE;			
			case ADRParamSetup:
				return MAC_EMPTY_SIZE;
			case DeviceTime:
				return MAC_EMPTY_SIZE;
			case ForceRejoin:
				return MAC_EMPTY_SIZE;
			case RejoinParamSetup:
				return MAC_REJOINPARAMSETUP_RESP_SIZE;
			// Class-B Section 14
			case PingSlotInfo:
				return MAC_PINGSLOTINFO_SIZE;
			case PingSlotChannel:
				return MAC_PINGSLOTCHANNEL_RESP_SIZE;
			// 0x12 has been deprecated in 1.1
			case BeaconTiming:
				return MAC_EMPTY_SIZE;
			case BeaconFreq:
				return MAC_BEACONFREQUENCY_RESP_SIZE;
			// Class-C
			case DeviceMode:
				return MAC_DEVICEMODE_SIZE;
			default:
				return 0;
			}
	}
}

/**
 * Return MAC command size
 */
size_t MacData::size() const
{
	if (errcode)
		return 0;
	return commandSize(command, isClientSide);
}

// ---------------- MacDataList ----------------

/**
 * Create empty list of MAC commands
 */
MacDataList::MacDataList()
	: isClientSide(false)
{

}

/**
 * Copy MAC commands list
 */
MacDataList::MacDataList(
	const MacDataList &value
)
	: isClientSide(false)
{
	list = value.list;
}

/**
 * Create MAC command list from MAC payload
 * @param value payload
 */
MacDataList::MacDataList(
	const std::string &value,
	const bool clientSide
)
	: isClientSide(clientSide)
{
	MacData data;
	int r;

	char *c = (char *) value.c_str();
	size_t sz = value.size();
	while(true) {
		if (clientSide)
			r = parseClientSide(data.command, c, sz);
		else
			r = parseServerSide(data.command, c, sz);
		if (r < 0)
			break;
		list.push_back(data);
		sz -= r;
		c+= r;
	}
}

/**
 * Returm MAC commands total size
 */
size_t MacDataList::size()
{
	size_t sz = 0;
	for (std::vector<MacData>::const_iterator it(list.begin()); it != list.end(); it++) {
		sz += it->size();
	}
	return sz;
}

/**
 * Return MAC commands as hex string
 */
std::string MacDataList::toHexString() const
{
	std::stringstream ss;
	for (std::vector<MacData>::const_iterator it(list.begin()); it != list.end(); it++) {
		ss << it->toHexString();
	}
	return ss. str();
}

/**
 * Serialize MAC commands as JSON string
 */
std::string MacDataList::toJSONString() const
{
	std::stringstream ss;
	ss << "[";
	bool needComma = false;
	for (std::vector<MacData>::const_iterator it(list.begin()); it != list.end(); it++) {
		if (needComma)
			ss << ", ";
		ss << it->toJSONString();
		needComma = true;
	}
	ss << "]";
	return ss. str();
}

/**
 * Build server -> to end-device request
 */

/**
 * @brief Reset end-device request
 */ 
MacDataClientReset::MacDataClientReset()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_RESET *v = (MAC_COMMAND_RESET*) &command;
	v->command = Reset;
	v->data.rfu = 0;		// not used
	v->data.minor = 1;		// LoRaWAN x.1
}

/**
 * @brief Check link answer
 */ 
MacDataClientLinkCheck::MacDataClientLinkCheck()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_LINK_CHECK *v = (MAC_COMMAND_LINK_CHECK*) &command;
	v->command = LinkCheck;
	v->data.gwcnt = 1;			// at least 1
	v->data.margin = 20;		// dB 255 reserved
}

/**
 * @param linkMarginDb dB 0..154, 255 reserverd. number of gateways that successfully received the last LinkCheckReq
 * @param gatewaysReceivedRequestCount  link margin in dB of the last successfully received LinkCheckReq command
 */ 
MacDataClientLinkCheck::MacDataClientLinkCheck(
	uint8_t linkMarginDb,
	uint8_t gatewaysReceivedRequestCount
) {
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_LINK_CHECK *v = (MAC_COMMAND_LINK_CHECK*) &command;
	v->command = LinkCheck;
	v->data.gwcnt = gatewaysReceivedRequestCount;		// at leat 1
	v->data.margin = linkMarginDb;		
}

/**
 * @brief Network Server requests an end-device to perform a rate adaptation
 */
MacDataClientLinkADR::MacDataClientLinkADR()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_LINK_ADR_REQ *v = (MAC_COMMAND_LINK_ADR_REQ*) &command;
	v->command = LinkADR;
	v->data.txpower = 0xf;		// ignore, keep the current parameter value. 
	v->data.datarate = 0;		// LoRa: SF12 / 125 kHz
	v->data.chmask = 0xffff;	// all 16 channels
	v->data.nbtans = 1;			// The default value is 1 corresponding to a single transmission of each frame. 
	v->data.chmaskcntl = 0;		// 0- mask, 6- all channels ON
	v->data.rfu = 0;
}

/**
 * @brief Network Server requests an end-device to perform a rate adaptation
 */
MacDataClientLinkADR::MacDataClientLinkADR(
	uint8_t txpower,
	uint8_t datarate,
	uint16_t chmask,
	uint8_t nbtans,
	uint8_t chmaskcntl
) {
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_LINK_ADR_REQ *v = (MAC_COMMAND_LINK_ADR_REQ*) &command;
	v->command = LinkADR;
	v->data.txpower = 0xf;		// ignore, keep the current parameter value. 
	v->data.datarate = 0;		// LoRa: SF12 / 125 kHz
	v->data.chmask = 0xffff;	// all 16 channels
	v->data.nbtans = 1;			// The default value is 1 corresponding to a single transmission of each frame. 
	v->data.chmaskcntl = 0;		// 0- mask, 6- all channels ON
	v->data.rfu = 0;
}

/**
 * @brief Network coordinator limit the maximum aggregated transmit duty cycle of an end-device.
 */
MacDataClientDutyCycle::MacDataClientDutyCycle()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_DUTY_CYCLE *v = (MAC_COMMAND_DUTY_CYCLE*) &command;
	v->command = DutyCycle;
	v->data.maxdccycle = 0; // 0- no duty cycle limitation except the one set by the regional regulation.
	v->data.rfu = 0;
}

/**
 * @param value limit 0..15,  0- no duty cycle limitation except the one set by the regional regulation.
 */
MacDataClientDutyCycle::MacDataClientDutyCycle(
	uint8_t value
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_DUTY_CYCLE *v = (MAC_COMMAND_DUTY_CYCLE*) &command;
	v->command = DutyCycle;
	v->data.maxdccycle = value & 0xf;
	v->data.rfu = 0;
}

/**
 * @brief Network server change frequency/data rate for the second receive window RX2
 */
MacDataClientRXParamSetup::MacDataClientRXParamSetup()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_RXRARAMSETUP_REQ *v = (MAC_COMMAND_RXRARAMSETUP_REQ*) &command;
	v->command = RXParamSetup;
	SET_FREQUENCY(v->data.frequency, DEF_FREQUENCY_100) 
	v->data.rx1droffset = 0;				// Default 0. offset between the uplink data rate and the downlink data rate used to communicate with the end-device on the first reception slot (RX1)
	v->data.rx2datatrate = 0;				// 0 means 1008 DR0/125kHz
	v->data.rfu = 0;						// not used
}

/**
 * @brief Network server change frequency/data rate for the second receive window RX2
 * @param frequency 100 * Hz
 * @param rx1droffset 0..7, default 7
 * @param rx2datatrate 0..15 means 1008 DR0/125kHz
 */
MacDataClientRXParamSetup::MacDataClientRXParamSetup(
	uint32_t frequency,
	uint8_t rx1droffset,
	uint8_t rx2datatrate
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_RXRARAMSETUP_REQ *v = (MAC_COMMAND_RXRARAMSETUP_REQ*) &command;
	v->command = RXParamSetup;
	SET_FREQUENCY(v->data.frequency, frequency) 	// 24 bit uyint, 100*Hz;
	v->data.rx1droffset = rx1droffset & 0x7;		// Default 0. offset between the uplink data rate and the downlink data rate used to communicate with the end-device on the first reception slot (RX1)
	v->data.rx2datatrate = rx1droffset & 0xf;		// 0 means 1008 DR0/125kHz
	v->data.rfu = 0;								// not used
}

/**
 * @brief Network Server may request status information from an end-device: battery, temperature
 */
MacDataClientDevStatus::MacDataClientDevStatus(
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_EMPTY *v = (MAC_COMMAND_EMPTY*) &command;
	v->command = DevStatus;
}

/**
 * @brief Network Server may request status information from an end-device: battery, temperature
 */
MacDataClientNewChannel::MacDataClientNewChannel(
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_NEWCHANNEL_REQ *v = (MAC_COMMAND_NEWCHANNEL_REQ*) &command;
	v->command = NewChannel;
	v->data.chindex = 0;
	SET_FREQUENCY(v->data.frequency, DEF_FREQUENCY_100)
	v->data.mindr = 0;
	v->data.maxdr = 0;
}

/**
 * @brief modify the parameters of an existing bidirectional channel or to create a new one
 * The commands SHALL not be answered by the device
 * @param channelIndex 0..15 channnel index 0..N N- see 
 * @param frequency 10 * Hz
 * @param mindr 0..15
 * @param maxdr 0..15
 */
MacDataClientNewChannel::MacDataClientNewChannel(
	uint8_t channelIndex,
	uint32_t frequency,
	uint8_t mindr,
	uint8_t maxdr
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_NEWCHANNEL_REQ *v = (MAC_COMMAND_NEWCHANNEL_REQ*) &command;
	v->command = NewChannel;
	v->data.chindex = channelIndex & 0xf;
	SET_FREQUENCY(v->data.frequency, frequency)
	v->data.mindr = mindr &0xf;
	v->data.maxdr = maxdr &0xf;
}

/**
 * @brief Configuring the delay between the end of the TX uplink and the opening of the first reception slot. The second reception slot opens one second after the first reception slot.
 */
MacDataClientRXTimingSetup::MacDataClientRXTimingSetup(
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_TIMINGSETUP *v = (MAC_COMMAND_TIMINGSETUP*) &command;
	v->command = RXTimingSetup;
	v->data.delay = 0;	// 1s
	v->data.rfu = 0;
}

/**
 * @brief Configuring the delay between the end of the TX uplink and the opening of the first reception slot. The second reception slot opens one second after the first reception slot.
 * @param delay 0..15 seconds + 1
 */
MacDataClientRXTimingSetup::MacDataClientRXTimingSetup(
	uint8_t secomdsPlus1
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_TIMINGSETUP *v = (MAC_COMMAND_TIMINGSETUP*) &command;
	v->command = RXTimingSetup;
	v->data.delay = secomdsPlus1;
	v->data.rfu = 0;
}


/**
 * @param downlinkDwellTime400ms true - 400ms, false- no limit
 * @param uplinkDwellTime400ms true - 400ms, false- no limit
 * @param maxEIRP 0..15 -> 8dBm 10 12 13 14 16 18 20 21 24 26 27 29 30 33 36dBm
*/
MacDataTXParamSetup::MacDataTXParamSetup(
	bool downlinkDwellTime,
	bool uplinkDwellTime,
	uint8_t maxEIRP
) {
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_TXPARAMSETUP *v = (MAC_COMMAND_TXPARAMSETUP*) &command;
	v->command = TXParamSetup;
	v->data.downlinkdwelltime = downlinkDwellTime ? 1 : 0;
	v->data.uplinkdwelltime = uplinkDwellTime ? 1 : 0;
	v->data.maxeirp = maxEIRP & 0xf;
	v->data.rfu = 0;
}

/**
 * @param downlinkDwellTime400ms true - 400ms, false- no limit
 * @param uplinkDwellTime400ms true - 400ms, false- no limit
 * @param maxEIRP 0..15 -> 8dBm 10 12 13 14 16 18 20 21 24 26 27 29 30 33 36dBm
*/
MacDataDLChannel::MacDataDLChannel()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_DLCHANNEL_REQ *v = (MAC_COMMAND_DLCHANNEL_REQ*) &command;
	v->command = DLChannel;
	v->data.chindex = 0;
	SET_FREQUENCY(v->data.frequency, DEF_FREQUENCY_100)
}

/**
 * @param chindex 0..15
 * @param frequency 10 * Hz
*/
MacDataDLChannel::MacDataDLChannel(
	uint8_t chindex,
	uint32_t frequency
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_DLCHANNEL_REQ *v = (MAC_COMMAND_DLCHANNEL_REQ*) &command;
	v->command = DLChannel;
	v->data.chindex = chindex & 0xf;
	SET_FREQUENCY(v->data.frequency, frequency)
}

/**
 * @brief Confirm security key update OTA devices
 */ 
MacDataRekey::MacDataRekey() {
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_REKEY_REQ *v = (MAC_COMMAND_REKEY_REQ*) &command;
	v->command = Rekey;
	v->data.minor = 1;
	v->data.rfu = 0;
}

/**
 * @brief Confirm security key update OTA devices
 */ 
MacDataADRParamSetup::MacDataADRParamSetup() {
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_ADRPARAMSETUP *v = (MAC_COMMAND_ADRPARAMSETUP*) &command;
	v->command = ADRParamSetup;
	v->data.delayexp = 0;
	v->data.limitexp = 0;
}

/**
 * @brief set ackLimit, ackDelay to 1(0)
 */ 
MacDataADRParamSetup::MacDataADRParamSetup(
	uint8_t ackLimit,
	uint8_t ackDelay
) {
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_ADRPARAMSETUP *v = (MAC_COMMAND_ADRPARAMSETUP*) &command;
	v->command = ADRParamSetup;
	v->data.delayexp = ackLimit & 0xf;
	v->data.limitexp = ackDelay & 0xf;
}

/**
 * @brief set ackLimit, ackDelay to 1(0)
 */ 
MacDataDeviceTime::MacDataDeviceTime()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_DEVICETIME *v = (MAC_COMMAND_DEVICETIME*) &command;
	v->command = DeviceTime;
	
	struct timeval t;
	gettimeofday(&t, NULL);
	v->data.gpstime = utc2gps(t.tv_sec);
	v->data.frac = (uint8_t) t.tv_usec;
}

/**
 * @brief network asks a device to immediately transmit a Rejoin-Request 
 * with a programmable number of retries,periodicity and data rate
 */
MacDataForceRejoin::MacDataForceRejoin()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_FORCEREJOIN *v = (MAC_COMMAND_FORCEREJOIN*) &command;
	v->command = ForceRejoin;
	v->data.period = 0;
	v->data.maxretries = 0;
	v->data.rejointype = 0;
	v->data.rfu = 0;
	v->data.rfu2 = 0;
}

/**
 * @brief network asks a device to immediately transmit a Rejoin-Request 
 * with a programmable number of retries,periodicity and data rate
 * @param period delay, s: 32s * 2^Period + Random(0..32)
 * @param maxretries 0- no retry, 1..7 
 * @param rejointype 0 or 1: A Rejoin-request type 0, 2: type 2
 * @param dr 0..15 data rateMakefile:2115: recipe for target 'lorawan_network_server-lorawan-mac.o' failed

 */
MacDataForceRejoin::MacDataForceRejoin(
	uint8_t period,
	uint8_t maxretries,
	uint8_t rejointype,
	uint8_t dr
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_FORCEREJOIN *v = (MAC_COMMAND_FORCEREJOIN*) &command;
	v->command = ForceRejoin;
	v->data.period = period & 7;
	v->data.maxretries = maxretries & 7;
	v->data.rejointype = rejointype & 7;
	v->data.dr = dr & 0xf;
	v->data.rfu = 0;
	v->data.rfu2 = 0;
}

/**
 * @brief network request end-device to periodically send a RejoinReq Type 0
 */ 
MacDataRejoinParamSetup::MacDataRejoinParamSetup()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_REJOINPARAMSETUP_REQ *v = (MAC_COMMAND_REJOINPARAMSETUP_REQ*) &command;
	v->command = ForceRejoin;
	v->data.maxtime = 0;
	v->data.maxccount = 0;
}

/**
 * @brief network request end-device to periodically send a RejoinReq Type 0
 * @param maxtime 0..15
 * @param maxcount 0..15
 */ 
MacDataRejoinParamSetup::MacDataRejoinParamSetup(
	uint8_t maxtime,
	uint8_t maxcount
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_REJOINPARAMSETUP_REQ *v = (MAC_COMMAND_REJOINPARAMSETUP_REQ*) &command;
	v->command = ForceRejoin;
	v->data.maxtime = maxtime & 0xf;
	v->data.maxccount = maxcount & 0xf;
}

// Class-B Section 14
/**
  * @brief Answer to  end-device informs the server of its unicast ping slot periodicity
  * No decription in spec. The command has no answer?
  */ 
MacDataPingSlotInfo::MacDataPingSlotInfo()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_EMPTY *v = (MAC_COMMAND_EMPTY*) &command;
	v->command = PingSlotInfo;
}

/**
  * @brief Answer to  end-device informs the server of its unicast ping slot periodicity
  * No decription in spec. The command has no answer?
  */ 
MacDataPingSlotChannel::MacDataPingSlotChannel()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_PINGSLOTCHANNEL_REQ *v = (MAC_COMMAND_PINGSLOTCHANNEL_REQ*) &command;
	v->command = PingSlotChannel;
	v->data.dr = 0;
	SET_FREQUENCY(v->data.frequency, DEF_FREQUENCY_100)
	v->data.rfu = 0;
}

/**
 * @param frequency 100 * Hz
 * @param datarate 0..15
 */
MacDataPingSlotChannel::MacDataPingSlotChannel(
	uint32_t frequency,
	uint8_t datarate
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_PINGSLOTCHANNEL_REQ *v = (MAC_COMMAND_PINGSLOTCHANNEL_REQ*) &command;
	v->command = PingSlotChannel;
	v->data.dr = datarate & 0xf;
	SET_FREQUENCY(v->data.frequency, frequency)
	v->data.rfu = 0;
}

/**
  * @brief 0x12 has been deprecated in 1.1
  */ 
MacDataBeaconTiming::MacDataBeaconTiming()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_BEACONTIMING *v = (MAC_COMMAND_BEACONTIMING*) &command;
	v->command = BeaconFreq;
	v->data.channel = 0;
	v->data.delay = 0;
}

/**
  * @brief 0x12 has been deprecated in 1.1
  */ 
MacDataBeaconTiming::MacDataBeaconTiming(
	uint16_t delay,
	uint8_t channel
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_BEACONTIMING *v = (MAC_COMMAND_BEACONTIMING*) &command;
	v->command = BeaconTiming;
	v->data.channel = channel & 0xf;
	v->data.delay = delay;
}

/**
 * @brief server modify the frequency on which end-device expects the beacon
 */ 
MacDataBeaconFreq::MacDataBeaconFreq()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_BEACONFREQUENCY_REQ *v = (MAC_COMMAND_BEACONFREQUENCY_REQ*) &command;
	v->command = BeaconFreq;
	SET_FREQUENCY(v->data.frequency, DEF_FREQUENCY_100)
}

MacDataBeaconFreq::MacDataBeaconFreq(
	uint32_t frequency
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_BEACONFREQUENCY_REQ *v = (MAC_COMMAND_BEACONFREQUENCY_REQ*) &command;
	v->command = BeaconFreq;
	SET_FREQUENCY(v->data.frequency, frequency)
}

/**
 * @brief @brief server set end-device class A or C
 */ 
// Set class A
MacDataDeviceMode::MacDataDeviceMode()
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_DEVICEMODE *v = (MAC_COMMAND_DEVICEMODE*) &command;
	v->command = DeviceMode;
	v->data.cl = 0; // class: 0- A, 1- RFU, 2- C
}

MacDataDeviceMode::MacDataDeviceMode(
	bool classC
)
{
	errcode = 0;
	isClientSide = false;
	MAC_COMMAND_DEVICEMODE *v = (MAC_COMMAND_DEVICEMODE*) &command;
	v->command = DeviceMode;
	v->data.cl = classC ? 2 : 0; // class: 0- A, 1- RFU, 2- C
}

MacPtr::MacPtr(
	const std::string &parseData,
	const bool aClientSide
)
	: clientSide(aClientSide)
{
	parse(parseData);
}

/**
 * Set size, errorcode
 */
void MacPtr::parse(
	const std::string &parseData
) 
{
	MAC_COMMAND *m;
	int r;
    std::cerr << "MacPtr::parseRX " << hexString(parseData) << std::endl;
	const char *p = parseData.c_str();
	size_t sz = parseData.size();
	while (sz > 0)
	{
		if (clientSide)
			r = parseClientSidePtr(&m, p, sz);
		else
			r = parseServerSidePtr(&m, p, sz);
		if (r < 0)
			break;
		sz -= r;
		p += sz;
		mac.push_back(m);
	}
	errorcode = r < 0 ? r : 0;
}

std::string MacPtr::toHexString() const
{
	std::stringstream ss;
	for (std::vector<MAC_COMMAND* >::const_iterator it(mac.begin()); it != mac.end(); it++ ) {
		ss << std::hex << (int) (*it)->command;
		size_t sz = commandSize(*(*it), clientSide);
		ss << hexString((const char *) &(*it)->data, sz);
	}
	return ss.str();
}

std::string MacPtr::toJSONString() const
{
	std::stringstream ss;
	ss << "[";
	bool needComma = false;
	for (std::vector<MAC_COMMAND* >::const_iterator it(mac.begin()); it != mac.end(); it++ ) {
		if (needComma)
			ss << ", ";
		ss << "{" << MAC_DATA2JSONString(*(*it), clientSide) << "}";
		needComma = true;
	}
	ss << "]";
	return ss.str();
}

/**
 * Respond on MAC command
 * @param outMacCommand return MAC command
 * @param inMacCommand MAC command to response
 * @param packet Received Semtech packet to answer
 * @return true if has answer
 */
bool MacPtr::mkResponseMAC(
        MAC_COMMAND &outMacCommand,
        const MAC_COMMAND *inMacCommand,
        SemtechUDPPacket &packet
)
{
	outMacCommand.command = inMacCommand->command;
	switch (inMacCommand->command)
	{
	case LinkCheck:
		// fill out
		{
			float lorasnr;
			uint64_t gwa = packet.getBestGatewayAddress(&lorasnr);
			// gateways received packet
			outMacCommand.data.linkcheck.gwcnt = (uint8_t) packet.metadata.size();
			outMacCommand.data.linkcheck.margin = (uint8_t) loraMargin(6, lorasnr);
		}
		break;
	default:
		return false;
	}
	return true;
}

/**
 * Request MAC command from server side
 * @param outMacCommand return MAC command
 * @param inMacCommand MAC command to response
 * @param packet Received Semtech packet to inject request
 * @return true success
 */
bool MacPtr::mkRequestMAC(
        MAC_COMMAND &outMacCommand,
        const uint8_t macCommandCode,
        SemtechUDPPacket &packet
)
{
	outMacCommand.command = macCommandCode;
	switch (macCommandCode)
	{
	case DevStatus:
		break;
	default:
		return false;
	}
	return true;
}

/**
 * Produce MAC command(s) response, return MAC response payload in the retval parameter 
 * @param retval JSON txpk string to be sent over Semtech gateway
 * @param packet Received Semtech packet to answer
 * @param offset default 0
 * @return -1 no more, otherwise count of MAC answered (response can be too long)
 */
int MacPtr::mkResponseMACs(
        std::ostream &retval,
        SemtechUDPPacket &packet
)
{
	for (int i = 0; i < mac.size(); i++) {
		MAC_COMMAND rmac;
		if (mkResponseMAC(rmac, mac[i], packet)) {
			std::string m = MAC_COMMANDResponse2binary(rmac);
			if (m.size() == 0)
				return -1;
			else
				retval << m;
		}
	}
	return (int) mac.size();
}

/**
 * Produce MAC command request, return MAC response payload in the retval parameter 
 * @param retval JSON txpk string to be sent over Semtech gateway
 * @param packet Received Semtech packet to answer
 * @return -1 no more, otherwise count of MAC answered (response can be too long)
 */
int MacPtr::mkRequestMACs(
        std::ostream &retval,
        SemtechUDPPacket &packet
)
{
	std::vector<uint8_t> requestMacCommands;

	// TODO
	requestMacCommands.push_back(DevStatus);

	for (std::vector<uint8_t>::const_iterator it(requestMacCommands.begin()); it != requestMacCommands.end(); it++) 
	{
		MAC_COMMAND rmac;
		if (mkRequestMAC(rmac, *it, packet)) {
			std::string m = MAC_COMMANDRequest2binary(rmac);
			if (m.size() == 0)
				return -1;
			else
				retval << m;
		}
	}
	return (int) mac.size();
}

std::string MAC_COMMANDResponse2binary(
	MAC_COMMAND &c
)
{
	switch (c.command) {
		case Reset:	// req
			break;
		case LinkCheck:
			return std::string((const char *) &c, MAC_LINK_CHECK_SIZE);
			break;
		case LinkADR:
			break;
		case DutyCycle:
			break;
		case RXParamSetup:
			break;
		case DevStatus:
			break;
		case NewChannel:
			break;
		case RXTimingSetup:
			break;
		case TXParamSetup:
			break;
		case DLChannel:
			break;
		case Rekey:
			break;
		case ADRParamSetup:
			break;
		case DeviceTime:
			break;
		case ForceRejoin:
			break;
		case RejoinParamSetup:
			break;
		// Class-B Section 14
		case PingSlotInfo:
			break;
		case PingSlotChannel:
			break;
		// 0x12 has been deprecated in 1.1
		case BeaconTiming:
			break;
		case BeaconFreq:
			break;
		// Class-C
		case DeviceMode:
			break;
		default:
			break;
	}
	return "";
}

std::string MAC_COMMANDRequest2binary(
	MAC_COMMAND &c
)
{
	return std::string((const char *) &c, 1);
}
