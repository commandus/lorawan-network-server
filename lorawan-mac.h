#ifndef LORAWAN_MAC_H
#define LORAWAN_MAC_H 1

#include <string>
#include "platform.h"

// MAC commands as specified by the LoRaWAN R1.0 specs section 5
enum MAC_CID {
	// Class-A
	Reset 			 = 0x01,	// Reset ABP-activated device for ABP activated devices only
	LinkCheck        = 0x02,
	LinkADR          = 0x03,
	DutyCycle        = 0x04,
	RXParamSetup     = 0x05,
	DevStatus        = 0x06,
	NewChannel       = 0x07,
	RXTimingSetup    = 0x08,
	TXParamSetup     = 0x09,
	DLChannel        = 0x0a,
	Rekey            = 0x0b,	// Rekey indication for OTAA only (LoRaWAN1.1 only,  LoRaWAN1.0 not available)
	ADRParamSetup    = 0x0c,
	DeviceTime       = 0x0d,	// LoRaWAN1.1 only,  LoRaWAN1.0 not available
	ForceRejoin      = 0x0e,	// The command has no answer
	RejoinParamSetup = 0x0f,	

	// Class-B Section 14
	PingSlotInfo     = 0x10,	// The command has no answer 
	PingSlotChannel  = 0x11,	// 

	// 0x12 has been deprecated in 1.1
	BeaconTiming     = 0x12,	//  Deprecated
	BeaconFreq       = 0x13,

	// Class-C
	DeviceMode       = 0x20
	// 0x80 to 0xFF reserved for proprietary network command extensions
};

// macPayloadInfo contains the info about a MAC payload
typedef struct {
	uint8_t size_uplink;	// sent by end-device
	int8_t size_downlink;	// sent by LoraWAN network server
} MAC_PAYLOAD_SIZE;

static const MAC_PAYLOAD_SIZE macPayloadRegistry[] {
	{0, 0},	// CID	Request						Response
	{1, 1},	// 1	MAC_RESET 					MAC_RESET
	{0, 2},	// 2	MAC_EMPTY					MAC_LINK_CHECK
	{4, 1},	// 3	MAC_LINK_ADR_REQ			MAC_LINK_ADR_RESP
	{1, 1},	// 4	MAC_DUTY_CYCLE				MAC_DUTY_CYCLE
	{4, 1},	// 5	MAC_RARAMSETUP_REQ			MAC_RARAMSETUP_RESP
	{5, 2},	// 6	MAC_EMPTY					MAC_DEVSTATUS_RESP
	{1, 1},	// 7	MAC_NEWCHANNEL_REQ			MAC_NEWCHANNEL_RESP
	{1, 1},	// 8	MAC_TIMINGSETUP				MAC_EMPTY
	{1, 1},	// 9	MAC_PARAMSETUP				MAC_EMPTY
	{4, 1},	// a	MAC_DLCHANNEL_REQ			MAC_DLCHANNEL_RESP
	{1, 1},	// b	MAC_REKEY_REQ				MAC_REKEY_RESP
	{1, 0},	// c	MAC_ADRPARAMSETUP			MAC_EMPTY
	{0, 1},	// d	MAC_EMPTY					MAC_DEVICETIME
	{2, -1},// e	MAC_FORCEREJOIN				The command has no answer
	{2, 1},	// f	MAC_REJOINPARAMSETUP_REQ	MAC_REJOINPARAMSETUP_RESP
	{1, 1},	// 10	MAC_PINGSLOTINFO			MAC_EMPTY? No decription in spec. The command has no answer?
	{4, 1}, // 11	MAC_PINGSLOTCHANNEL_REQ		MAC_PINGSLOTCHANNEL_RESP
	{1, 1},	// 12	MAC_EMPTY					MAC_BEACONTIMING
	{1, 1}	// 13	MAC_BEACONFREQUENCY_REQ		MAC_BEACONFREQUENCY_REЫЗ
			// 20	MAC_DEVICEMODE				MAC_DEVICEMODE
};

// 1) Reset ABP-activated device
// ABP activated devices only
typedef ALIGN struct {
	uint8_t minor: 4;
	uint8_t rfu: 4;
} PACKED MAC_RESET;				// 1 byte

// 2) Link check request
typedef ALIGN struct {
} MAC_EMPTY;					// zero bytes

// 2) Link check answer
// @param margin 0..254- dB of the last successfully received LinkCheckReq command, e.g. 20- 20dB. 255 is reserved.
// @param gwcnt  number of gateways that successfully received the last LinkCheckReq command
typedef ALIGN struct {
	uint8_t margin;
	uint8_t gwcnt;
} PACKED MAC_LINK_CHECK;

// 3) Link addr request
// @param datarate 15- keep current data rate, 0- min, 14- max
// @param txpower 15- keep current transmaiision power, 0- min, 14- max
// @param chmask frequency (channel) mask. LSB 0- channel 1, msb bit 7 (bit 15)- channel 16
// @param nbtans 1..15 number of transmissions for each uplink message. 1- single transmission of each frame.
// @param chmaskcntl 0- channels 1..16. 1-5, 7- RFU, 6- All channels ON The device should enable all currently defined channels independently of the ChMask field value. 
typedef ALIGN struct {
	uint8_t txpower : 4;
	uint8_t datarate : 4;
	uint16_t chmask;				// LSB 0- channel 1, msb bit 7 (bit 15)- channel 16
	uint8_t nbtans: 4;				// 
	uint8_t chmaskcntl: 3;			// 
	uint8_t rfu: 1;					// not used
} PACKED MAC_LINK_ADR_REQ;			// 4 bytes

// 3) Link addr request
// @param datarate 15- keep current data rate, 0- min, 14- max
// @param txpower 15- keep current transmaiision power, 0- min, 14- max
// @param chmask frequency (channel) mask. LSB 0- channel 1, msb bit 7 (bit 15)- channel 16
// @param nbtans 1..15 number of transmissions for each uplink message. 1- single transmission of each frame.
// @param chmaskcntl 0- channels 1..16. 1-5, 7- RFU, 6- All channels ON The device should enable all currently defined channels independently of the ChMask field value. 
typedef ALIGN struct {
	uint8_t channelmaskack: 1;
	uint8_t datarateack: 1;
	uint8_t powerack: 1;
	uint8_t rfu: 5;
} PACKED MAC_LINK_ADR_RESP;			// 1 bуte

// 4) Transmit duty cycle
// @param maxdccycle 0- no limit 1..15- limit
// @param rfu no description in the spec
typedef ALIGN struct {
	uint8_t maxdccycle: 4;
	uint8_t rfu: 4;
} PACKED MAC_DUTY_CYCLE;

// 5) Receive Windows Parameters
// @param rx2datatrate data rate of a downlink using the second receive window following the same convention as the LinkADRReq command (0 - DR0,..)
// @param rx1droffset
// @param rfu no description in the s LoRaWAN1.0pec
// @param frequency 24 bits 100 * Hz
typedef ALIGN struct {

	uint8_t rx2datatrate: 4;
	uint8_t rx1droffset: 3;
	uint8_t rfu: 1;
	uint8_t frequency[3];			// 24 bit uyint, 100*Hz
} PACKED MAC_RARAMSETUP_REQ;		// 4 bytes

// @param channelack
// @param rx2datatrateack
// @param rx1droffsetack
// @param rfu no description
typedef ALIGN struct {
	uint8_t channelack: 1;
	uint8_t rx2datatrateack: 1;
	uint8_t rx1droffsetack: 1;
	uint8_t rfu: 5;
} PACKED MAC_RARAMSETUP_RESP;		// 1 byte

// 6) End-Device Status response
// @param battery 0- external power source, 1- min, 254- max, 255- end-device was not able to mearure battery level
// @param margin demodulation signal-to-noise ratio -32..31
typedef ALIGN struct {
	uint8_t battery;
	int8_t margin;
} PACKED MAC_DEVSTATUS_RESP;		// 2 bytes

// 7) Creation / Modification of a Channel
// @param chindex N,,15 where N- channels count. e.g. Russia N = 2, additioonal channels 2..15
// @param freq
// @param mindr 0 -  0 DR0/125kHz
// @param mandr 0 -  0 DR0/125kHz
typedef ALIGN struct {
	uint8_t chindex;
	uint8_t freq[3];
	uint8_t mindr: 4;
	uint8_t maxdr: 4;
} PACKED MAC_NEWCHANNEL_REQ;		// 5 bytes

typedef ALIGN struct {
	uint8_t channelfrequencyack: 1;
	uint8_t datarateack: 1;
	uint8_t rfu: 6;
} PACKED MAC_NEWCHANNEL_RESP;		// 1 byte

// 8) Setting delay between TX and RX
// @param delay 0- 1s(default), 1- 1s, 2- 2s.. 15- 15s
// @param rfu not used
typedef ALIGN struct {
	uint8_t delay: 4;
	uint8_t rfu: 4;
} PACKED MAC_TIMINGSETUP;			// 1 byte

// 9) Setting maximum continuous transmission time of a packet over the air (EIRP)
// @param maxeirp
// value          0 1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
// Max EIRP (dBm) 8 10 12 13 14 16 18 20 21 24 26 27 29 30 33 36
// @param uplinkdwelltime 0- no limit, 1- 400ms
// @param downlinkdwelltime 0- no limit, 1- 400ms
// @param rfu not used
typedef ALIGN struct {
	uint8_t maxeirp: 4;
	uint8_t uplinkdwelltime: 1;
	uint8_t downlinkdwelltime: 1;
	uint8_t rfu: 2;
} PACKED MAC_PARAMSETUP;			// 1 byte

// A) DlChannelReq Associate a different downlink frequency to the RX1 slot. 
// Available for EU and China but not for US or Australia
// @param chindex 0..15
// @param freq 24 bin integer frequncy in 100 * Hz
typedef ALIGN struct {
	uint8_t chindex;
	uint8_t freq[3];
} PACKED MAC_DLCHANNEL_REQ;			// 4 bytes

// @param channelfrequencyack 1- ok
// @param uplinkfrequencyexistsack	1- ok
typedef ALIGN struct {
	uint8_t channelfrequencyack: 1;
	uint8_t uplinkfrequencyexistsack: 1;
	uint8_t rfu: 6;
} PACKED MAC_DLCHANNEL_RESP;		// 1 byte

// B) Rekey indication for OTAA only LoRaWAN1.1 servers (LoRaWAN1.0 does notr provide it)
// @param minor 
// @param rfu not used
typedef ALIGN struct {
	uint8_t minor: 4;				// LoRaWAN minor version, e.g. 1 for LoRaWAN 1.1
	uint8_t rfu: 4;
} PACKED MAC_REKEY_REQ;				// 1 byte

typedef ALIGN struct {
	uint8_t version;				// 1 for LoRaWAN 1.1 version
} PACKED MAC_REKEY_RESP;			// 1 byte

// C) Setup ADR_ACK_LIMIT and ADR_ACK_DELAY
// @param delayexp 0..15 ADR_ACK_DELAY power of 2: 0- 1, 1- 2, 2- 4, .. 15- 32768
// @param delayexp 0..15 ADR_ACK_LIMIT power of 2: 0- 1, 1- 2, 2- 4, .. 15- 32768
typedef ALIGN struct {
	uint8_t delayexp: 4;			// ADR_ACK_DELAY power of 2: 0- 1, 1- 2, 2- 4, .. 15- 32768
	uint8_t limitexp: 4;			// ADR_ACK_LIMIT power of 2: 0- 1, 1- 2, 2- 4, .. 15- 32768
} PACKED MAC_ADRPARAMSETUP;			// 1 byte

// D) request from the network the current network date and time
// @param minor 
// @param rfu not used
typedef ALIGN struct {
	uint32_t gpstime;				// GPS epoch seconds
	uint8_t frac;					// 1/256 seconds
} PACKED MAC_DEVICETIME;			// 1 byte

// E) network asks a device to immediately transmit a Rejoin-Request
// The command has no answer,
// @param rejointype rejoin-request frame SHALL be transmitted using the data rate DR. 
// @param period delay between retransmissions 32 seconds x 2** Period where Rand32 is a pseudo-random number in the [0:32] range. 
// @param maxretries device will retry the Rejoin-request.0- only once, no retry) 1..7
// @param rfu not used
// @param rfu2 not used
typedef ALIGN struct {
	uint8_t dr: 4;					// rejoin-request frame SHALL be transmitted using the data rate DR. 
	uint8_t rejointype: 3;			// 0, 1- rejoin-request type 0 shall be transmitted 2- type 2, 3..7- RFU
	uint8_t rfu: 1;					// not used

	uint8_t maxretries: 3;			// device will retry the Rejoin-request.0- only once, no retry) 1..7
	uint8_t period: 3;				// The delay between retransmissions 32 seconds x 2** Period where Rand32 is a pseudo-random number in the [0:32] range. 
	uint8_t rfu2: 2;				// not used
} PACKED MAC_FORCEREJOIN;			// 2 bytes

// F) network request end-device to periodically send a RejoinReq Type 0 message
// The command has no answer,
// @param maxcount MUST send a Rejoin-request type 0 at least every 2^C+4 uplink messages. 0- 16, 1- 32...
// @param maxtime 2^T+10: 0- 17 minutes, 15- 1 year
typedef ALIGN struct {
	uint8_t maxccount: 4;			// MUST send a Rejoin-request type 0 at least every 2^C+4 uplink messages. 0- 16, 1- 32...
	uint8_t maxtime: 4;				//  2^T+10: 0- 17 minutes, 15- 1 year
} PACKED MAC_REJOINPARAMSETUP_REQ;	// 1 byte

typedef ALIGN struct {
	uint8_t timeack: 1;				// 1- ok (end-device may not set time periodicity)
	uint8_t rfu: 7;					// not used
} PACKED MAC_REJOINPARAMSETUP_RESP;	// 1 byte

// 10) end-device informs the server of its unicast ping slot periodicity
// @param periodicity power of 2, 0- opens a ping slot every ~1 second during the beacon_window interval, 7- 128 seconds
// @param rfu not used
typedef ALIGN struct {
	uint8_t periodicity: 3;			// power of 2, 0- opens a ping slot every ~1 second during the beacon_window interval, 7- 128 seconds
	uint8_t rfu: 5;					// 
} PACKED MAC_PINGSLOTINFO;			// 1 byte

// 11) end-device informs the server of its unicast ping slot periodicity
// @param dr data rate
// @param frequency 24bit integer 100*Hz
typedef ALIGN struct {
	uint8_t dr: 4;					// data rate
	uint8_t rfu: 4;					// not used
	uint8_t frequency[3];			// 24bit integer 100*Hz
} PACKED MAC_PINGSLOTCHANNEL_REQ;	// 4 bytes

// @param dr data rate
// @param frequency 24bit integer 100*Hz
typedef ALIGN struct {
	uint8_t frequencyack: 1;		// channel frequency ok
	uint8_t drack: 1;				// data rate ok
	uint8_t fru: 6;					// not used
} PACKED MAC_PINGSLOTCHANNEL_RESP;	// 1 byte

// 12) Deprecated.
// @param channel index of the beaconing channel on which the next beacon will be broadcasted.
// @param delay 30 ms x (Delay+1) > RTime >= 30 ms x Delay
typedef ALIGN struct {
	uint8_t channel;				// index of the beaconing channel on which the next beacon will be broadcasted.
	uint16_t delay;					// delay 30 ms x (Delay+1) > RTime >= 30 ms x Delay
} PACKED MAC_BEACONTIMING_REQ;		// 3 bytes

typedef ALIGN struct {
	uint8_t channel;				// index of the beaconing channel on which the next beacon will be broadcasted.
	uint16_t delay;					// delay 30 ms x (Delay+1) > RTime >= 30 ms x Delay
} PACKED MAC_BEACONTIMING;			// 3 bytes

// 13) server request modify the frequency on which this end-device expects the beacon.
// @param frequency 24 bit integer 100 * Hz
typedef ALIGN struct {
	uint8_t frequency[3];			// frequency 24 bit integer 100 * Hz
} PACKED MAC_BEACONFREQUENCY_REQ;	// 3 bytes

typedef ALIGN struct {
	uint8_t frequencyack: 1;		// 1- frequency ok
	uint8_t rfu: 7;					// not used
} PACKED MAC_BEACONFREQUENCY_RESP;	// 1 byte

// 20)  end-device indicates that it wants to  operate either in class A or C.
// @param frequency 24 bit integer 100 * Hz
typedef ALIGN struct {
	uint8_t cl;						// class: 0- A, 1- RFU, 2- C
} PACKED MAC_DEVICEMODE;			// 1 byte

#endif
