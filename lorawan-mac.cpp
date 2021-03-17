#include "lorawan-mac.h"
#include <cstring>

#include "errlist.h"

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

/**
 * Parse transmitted by the server
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
		COPY_MAC(LINK_CHECK)	// empty request
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
 * Parse transmitted by client (end-device)
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
		break;	// same request and response
	case DutyCycle:
		COPY_MAC(DUTY_CYCLE)	// same request and response
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

// ---------------- MacData ----------------

MacData::MacData()
	: errcode(0), isClientSide(false)
{
	memset(&command, 0, sizeof(MAC_COMMAND));
}

MacData::MacData(
	const MacData &value
)
	: errcode(0), isClientSide(false)
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
	const bool clientSide
)
	: isClientSide(clientSide)
{
	int r;
	if (clientSide)
		r = parseClientSide(command, value.c_str(), value.size());
	else
		r = parseServerSide(command, value.c_str(), value.size());

	if (r < 0)
		errcode = r;
	else
		errcode = 0;
}

size_t commandSize(
	const MAC_COMMAND &value,
	bool clientSide
)
{
	if (clientSide)	{
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
		switch (value.command) {
			case Reset:	// req
				return MAC_RESET_SIZE;
			case LinkCheck:
				return MAC_EMPTY_SIZE;
			case LinkADR:
				return MAC_LINK_ADR_RESP_SIZE;
			case DutyCycle:
				return MAC_DUTY_CYCLE_SIZE;
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

size_t MacData::size() const
{
	if (errcode)
		return 0;
	return commandSize(command, isClientSide);
}

// ---------------- MacDataList ----------------

MacDataList::MacDataList()
	: isClientSide(false)
{

}

MacDataList::MacDataList(
	const MacDataList &value
)
	: isClientSide(false)
{
	list = value.list;
}

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

size_t MacDataList::size()
{
	size_t sz = 0;
	for (std::vector<MacData>::const_iterator it(list.begin()); it != list.end(); it++) {
		sz += it->size();
	}
	return sz;
}
