#include "lorawan-mac.h"
#include <cstring>
#include <sys/time.h>

#include "utildate.h"

#include "errlist.h"

#define DEF_FREQUENCY_100 868900

#define SET_FREQUENCY(arr, value) \
	arr[0] = value && 0xff; \
	arr[1] = (value >> 8) && 0xff; \
	arr[2] = (value >> 16) && 0xff;

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
	v->data.gwcnt = 1;		// at leat 1
	v->data.margin = 20;		// dB 255 reserverd
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
MacDataClientRXParamSetup::MacDataClientRXParamSetup(
)
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
	SET_FREQUENCY(v->data.freq, DEF_FREQUENCY_100)
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
	SET_FREQUENCY(v->data.freq, frequency)
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
	v->data.frac = t.tv_usec;
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
 * @param dr 0..15 data rate
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
	uint8_t channel,
	uint16_t delay
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


