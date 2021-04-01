#include <inttypes.h>
#include <iostream>
#include <iomanip>

#include "macgw-config-json.h"

#include "errlist.h"

typedef struct {
	uint32_t minvalue;
	uint32_t maxvalue;
	std::string name;
} COMMAND_PARAM_CHOICE;

typedef struct {
	std::string name;
	uint8_t count;
	const COMMAND_PARAM_CHOICE **list;
} COMMAND_PARAM;

typedef struct {
	uint8_t cmd;
	uint8_t paramcount;
	std::string shortname;
	std::string fullname;
	std::string descpription;
	const COMMAND_PARAM** param;
} COMMAND_DESCRIPTION;

// ------------------------- Param choice -------------------------

static const COMMAND_PARAM_CHOICE CHOICE_MARGIN_1_254 = {
	0, 254, "dB"
};

static const COMMAND_PARAM_CHOICE CHOICE_GWCNT_1_255 = {
	1, 255, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_TXPOWER = {
	0, 7, ""
};
static const COMMAND_PARAM_CHOICE CHOICE_TXPOWER_15 = {
	15, 15, "asis"
};
static const COMMAND_PARAM_CHOICE CHOICE_DATARATE = {
	0, 7, ""
};
static const COMMAND_PARAM_CHOICE CHOICE_DATARATE_15 = {
	15, 15, "asis"
};
static const COMMAND_PARAM_CHOICE CHOICE_CHMASK = {
	1, 255, ""
};
static const COMMAND_PARAM_CHOICE CHOICE_NBTRANS = {
	1, 15, ""
};
static const COMMAND_PARAM_CHOICE CHOICE_CHMASKCTL = {
	0, 7, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_DUTY_CYCLE_LIMIT = {
	0, 15, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_FREQUENCY = {
	7999999, 9999999, " * 100Hz"
};

static const COMMAND_PARAM_CHOICE CHOICE_RX1_DR_OFFSET = {
	0, 7, "s"
};

static const COMMAND_PARAM_CHOICE CHOICE_CHANNEL_INDEX = {
	0, 15, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_RXTIMING = {
	0, 15, " + 1s"
};

static const COMMAND_PARAM_CHOICE CHOICE_DWELLTIME_0 = {
	0, 0, "no-limit"
};

static const COMMAND_PARAM_CHOICE CHOICE_DWELLTIME_1 = {
	1, 1, "400"
};

static const COMMAND_PARAM_CHOICE CHOICE_MAXEIRP = {
	0, 15, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_1_32768 = {
	0, 15, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_1_7 = {
	0, 7, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_REJOIN_TYPE_0 = {
	0, 0, "0"
};

static const COMMAND_PARAM_CHOICE CHOICE_REJOIN_TYPE_2 = {
	2, 2, "2"
};

static const COMMAND_PARAM_CHOICE CHOICE_MAXTIME = {
	0, 15, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_MAXCOUNT = {
	0, 15, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_BEACON_DELAY = {
	0, 65535, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_BEACON_CHANNEL = {
	0, 15, ""
};

static const COMMAND_PARAM_CHOICE CHOICE_DEVICEMODE_A = {
	0, 0, "A"
};

static const COMMAND_PARAM_CHOICE CHOICE_DEVICEMODE_C = {
	2, 2, "C"
};

// ------------------------- Param choice -------------------------

static const COMMAND_PARAM_CHOICE *CHOICES_GWCNT[] = { &CHOICE_GWCNT_1_255 };
static const COMMAND_PARAM_CHOICE *CHOICES_MARGIN[] = { &CHOICE_MARGIN_1_254 };
static const COMMAND_PARAM_CHOICE *CHOICES_TXPOWER[] = { &CHOICE_TXPOWER, &CHOICE_TXPOWER_15 };
static const COMMAND_PARAM_CHOICE *CHOICES_DATARATE[] = { &CHOICE_DATARATE, &CHOICE_DATARATE_15 };
static const COMMAND_PARAM_CHOICE *CHOICES_CHMASK[] = { &CHOICE_CHMASK };
static const COMMAND_PARAM_CHOICE *CHOICES_NBTRANS[] = { &CHOICE_NBTRANS };
static const COMMAND_PARAM_CHOICE *CHOICES_CHMASKCTL[] = { &CHOICE_CHMASKCTL };
static const COMMAND_PARAM_CHOICE *CHOICES_DUTY_CYCLE_LIMIT[] = { &CHOICE_DUTY_CYCLE_LIMIT };
static const COMMAND_PARAM_CHOICE *CHOICES_FREQUENCY[] = { &CHOICE_FREQUENCY };
static const COMMAND_PARAM_CHOICE *CHOICES_RX1_DR_OFFSET[] = { &CHOICE_RX1_DR_OFFSET };
static const COMMAND_PARAM_CHOICE *CHOICES_CHANNEL_INDEX[] = { &CHOICE_CHANNEL_INDEX };
static const COMMAND_PARAM_CHOICE *CHOICES_RXTIMING[] = { &CHOICE_RXTIMING };
static const COMMAND_PARAM_CHOICE *CHOICES_DWELLTIME[] = { &CHOICE_DWELLTIME_0, &CHOICE_DWELLTIME_1 };
static const COMMAND_PARAM_CHOICE *CHOICES_MAXEIRP[] = { &CHOICE_MAXEIRP };
static const COMMAND_PARAM_CHOICE *CHOICES_1_32768[] = { &CHOICE_1_32768 };
static const COMMAND_PARAM_CHOICE *CHOICES_1_7[] = { &CHOICE_1_7 };
static const COMMAND_PARAM_CHOICE *CHOICES_REJOIN_TYPES[] = { &CHOICE_REJOIN_TYPE_0, &CHOICE_REJOIN_TYPE_2 };
static const COMMAND_PARAM_CHOICE *CHOICES_MAXTIME[] = { &CHOICE_MAXTIME };
static const COMMAND_PARAM_CHOICE *CHOICES_MAXCOUNT[] = { &CHOICE_MAXCOUNT };
static const COMMAND_PARAM_CHOICE *CHOICES_BEACON_DELAY[] = { &CHOICE_BEACON_DELAY };
static const COMMAND_PARAM_CHOICE *CHOICES_BEACON_CHANNEL[] = { &CHOICE_BEACON_CHANNEL };
static const COMMAND_PARAM_CHOICE *CHOICES_DEVICEMODE[] = { &CHOICE_DEVICEMODE_A, &CHOICE_DEVICEMODE_C };

// ------------------------- Param -------------------------

static const COMMAND_PARAM PARAM_GWCNT = {
	"gateways count",
	1, CHOICES_GWCNT
};

static const COMMAND_PARAM PARAM_MARGIN = {
	"link margin",
	1, CHOICES_MARGIN
};

static const COMMAND_PARAM PARAM_TXPOWER = {
	"tx power",
	2, CHOICES_TXPOWER
};
static const COMMAND_PARAM PARAM_DATARATE = {
	"data rate",
	2, CHOICES_DATARATE
};
static const COMMAND_PARAM PARAM_CHMASK = {
	"channel mask",
	1, CHOICES_CHMASK
};
static const COMMAND_PARAM PARAM_NBTRANS = {
	"transmissions per message",
	1, CHOICES_NBTRANS
};
static const COMMAND_PARAM PARAM_CHMASKCTL = {
	"mask control",
	1, CHOICES_CHMASKCTL
};

static const COMMAND_PARAM PARAM_DUTY_CYCLE_LIMIT = {
	"limit",
	1, CHOICES_DUTY_CYCLE_LIMIT
};

static const COMMAND_PARAM PARAM_FREQUENCY = {
	"frequency",
	1, CHOICES_FREQUENCY
};

static const COMMAND_PARAM PARAM_RX1_DR_OFFSET = {
	"RX1 offset",
	1, CHOICES_RX1_DR_OFFSET
};

static const COMMAND_PARAM PARAM_CHANNEL_INDEX = {
	"channel index",
	1, CHOICES_CHANNEL_INDEX
};

static const COMMAND_PARAM PARAM_MIN_DR = {
	"min data rate",
	1, CHOICES_DATARATE
};

static const COMMAND_PARAM PARAM_MAX_DR = {
	"max data rate",
	1, CHOICES_DATARATE
};

static const COMMAND_PARAM PARAM_RXTIMING = {
	"TX - RX delay",
	1, CHOICES_RXTIMING
};

static const COMMAND_PARAM PARAM_DOWNLINK_DWELLTIME = {
	"downlink dwell time",
	2, CHOICES_DWELLTIME
};

static const COMMAND_PARAM &PARAM_UPLINK_DWELLTIME = {
	"uplink dwell time",
	2, CHOICES_DWELLTIME
};

static const COMMAND_PARAM PARAM_MAXEIRP = {
	"max EIRP",
	1, CHOICES_MAXEIRP
};

static const COMMAND_PARAM PARAM_LIMIT_EXP = {
	"ADR ACK limit",
	1, CHOICES_1_32768
};

static const COMMAND_PARAM PARAM_DELAY_EXP = {
	"ADR ACK delay",
	1, CHOICES_1_32768
};

static const COMMAND_PARAM PARAM_REJOIN_PERIOD = {
	"retransmission delay",
	1, CHOICES_1_7
};

static const COMMAND_PARAM PARAM_REJOIN_RETRIES = {
	"max retransmission",
	1, CHOICES_1_7
};

static const COMMAND_PARAM PARAM_REJOIN_TYPE = {
	"rejoin type ",
	2, CHOICES_REJOIN_TYPES
};

static const COMMAND_PARAM PARAM_MAXTIME = {
	"max time ",
	1, CHOICES_MAXTIME
};

static const COMMAND_PARAM PARAM_MAXCOUNT = {
	"max count ",
	1, CHOICES_MAXCOUNT
};

static const COMMAND_PARAM PARAM_BEACON_DELAY = {
	"beacon delay ",
	1, CHOICES_BEACON_DELAY
};

static const COMMAND_PARAM PARAM_BEACON_CHANNEL = {
	"beacon channel ",
	1, CHOICES_BEACON_CHANNEL
};

static const COMMAND_PARAM PARAM_DEVICE_MODE = {
	"device mode ",
	2, CHOICES_DEVICEMODE
};

// ------------------------- Params -------------------------

const COMMAND_PARAM *LINK_CHECK_PARAMS[] = { &PARAM_MARGIN, &PARAM_GWCNT };
const COMMAND_PARAM *LINK_ADR_PARAMS[] = { &PARAM_TXPOWER, &PARAM_DATARATE, &PARAM_CHMASK, &PARAM_NBTRANS, &PARAM_CHMASKCTL };
const COMMAND_PARAM *DUTY_CYCLE_PARAMS[] = { &PARAM_DUTY_CYCLE_LIMIT };
const COMMAND_PARAM *RXPARAMSETUP_PARAMS[] = { &PARAM_FREQUENCY, &PARAM_RX1_DR_OFFSET, &PARAM_DATARATE};
const COMMAND_PARAM *NEWS_CHANNEL_PARAMS[] = { &PARAM_CHANNEL_INDEX, &PARAM_FREQUENCY, &PARAM_MIN_DR, &PARAM_MAX_DR };
const COMMAND_PARAM *RXTIMING_PARAMS[] = { &PARAM_RXTIMING };
const COMMAND_PARAM *TXPARAMSETUP_PARAMS[] = { &PARAM_DOWNLINK_DWELLTIME, &PARAM_UPLINK_DWELLTIME, &PARAM_MAXEIRP };
const COMMAND_PARAM *DLCHANNEL_PARAMS[] = { &PARAM_CHANNEL_INDEX, &PARAM_FREQUENCY };
const COMMAND_PARAM *ADRPARAMSETUP_PARAMS[] = { &PARAM_LIMIT_EXP, &PARAM_DELAY_EXP };
const COMMAND_PARAM *FORCE_REJOIN_PARAMS[] = { &PARAM_REJOIN_PERIOD, &PARAM_REJOIN_RETRIES, &PARAM_REJOIN_TYPE, &PARAM_DATARATE };
const COMMAND_PARAM *REJOIN_SETUP_PARAMS[] = { &PARAM_MAXTIME, &PARAM_MAXCOUNT };
const COMMAND_PARAM *PINGSLOTCHANNEL_PARAMS[] = { &PARAM_FREQUENCY, &PARAM_DATARATE };
const COMMAND_PARAM *BEACONTIMING_PARAMS[] = { &PARAM_BEACON_DELAY, &PARAM_BEACON_CHANNEL };
const COMMAND_PARAM *BEACON_FREAUENCY_PARAMS[] = { &PARAM_FREQUENCY };
const COMMAND_PARAM *DEVICE_MODE_PARAMS[] = { &PARAM_DEVICE_MODE };

// ------------------------- Commands -------------------------

static COMMAND_DESCRIPTION gatewayCommands[] = {
	{ LinkADR, 5, "a", "linkadr", "Rate adaptation", LINK_ADR_PARAMS },
	{ DutyCycle, 1, "d", "dutycycle", "Limit transmit duty cycle", DUTY_CYCLE_PARAMS },
	{ RXParamSetup, 3,  "rx", "rxparamsetup", "Change frequency/data RX2", RXPARAMSETUP_PARAMS },
	{ DevStatus, 0, "s", "devstatus", "Request device battery, temperature" },
	{ NewChannel, 4, "n", "newchannel", "Set channel frequency/ data rate", NEWS_CHANNEL_PARAMS },
	{ RXTimingSetup, 1, "rx", "rxtiming", "Set delay between TX and RX1", RXTIMING_PARAMS },
	{ TXParamSetup, 3, "tx", "dwelltime", "Set maximum allowed dwell time", TXPARAMSETUP_PARAMS },
	{ DLChannel, 2, "dl", "dlchannel", "Set RX1 slot frequency", DLCHANNEL_PARAMS },
	{ Rekey, 0, "k", "rekey", "Answer security OTA key update" },
	{ ADRParamSetup, 2, "al", "acklimit", "Set ADR_ACK_LIMIT, ADR_ACK_DELAY", ADRPARAMSETUP_PARAMS },
	{ ForceRejoin, 3, "j", "forcerejoin", "Request immediately Rejoin-Request", FORCE_REJOIN_PARAMS },
	{ RejoinParamSetup, 2, "js", "rejoinsetup", "Request periodically send Rejoin-Request", REJOIN_SETUP_PARAMS },
	{ PingSlotInfo, 0, "p", "ping", "Answer to unicast ping slot" },
	{ PingSlotChannel, 2, "pc", "pingchannel", "Set ping slot channel", PINGSLOTCHANNEL_PARAMS },
	{ BeaconFreq, 1, "bf", "beaconfreq", "Set beacon frequency", BEACON_FREAUENCY_PARAMS },
};

static COMMAND_DESCRIPTION enDeviceCommands[] = {
	{ Reset , 0, "r", "reset", "Reset end-device request", {}},	
	{ LinkCheck, 2, "l", "linkcheck", "Check link answer", LINK_CHECK_PARAMS },	 
	{ DeviceTime, 0, "t", "devicetime", "Answer date/time" },
	{ BeaconTiming, 2, "bt", "beacontiming", "Deprecated", BEACONTIMING_PARAMS },
	{ DeviceMode, 1, "m", "mode", "Set device mode", DEVICE_MODE_PARAMS }
};

MacGwConfig::MacGwConfig() 
	: gatewayId(""), eui(""), errcode(0), errmessage("")
{

}

void MacGwConfig::clear() {
	macCommands.list.clear();
	errcode = 0;
}

static int commandIndex(
	const std::string &value
)
{
	for (int i = 0; i < sizeof(gatewayCommands) / sizeof(COMMAND_DESCRIPTION); i++) {
		COMMAND_DESCRIPTION *c = &gatewayCommands[i];
		if ((value == c->shortname) || (value == c->fullname)) {
			return i;
		}
	}
	return -1;
}

static int64_t paramValNum(
	int64_t v,
	int paramIndex,
	const COMMAND_DESCRIPTION &cmd
) {
	const COMMAND_PARAM *p = cmd.param[paramIndex];
	int choiceCount = p->count;
	for (int c = 0; c < choiceCount; c++) {
		const COMMAND_PARAM_CHOICE *choice = p->list[c];
		if (v >= choice->minvalue && c <= choice->maxvalue) {
			return v;
		}
	}
	return  0;
}

static int64_t paramValAlias(
	const std::string &v,
	int paramIndex,
	const COMMAND_DESCRIPTION &cmd
) {
	const COMMAND_PARAM *p = cmd.param[paramIndex];
	int choiceCount = p->count;
	for (int c = 0; c < choiceCount; c++) {
		const COMMAND_PARAM_CHOICE *choice = p->list[c];
		if (v == choice->name) {
			return choice->minvalue;
		}
	}
	return  0;
}

static int paramValue(
	const std::string &value,
	int paramIndex,
	const COMMAND_DESCRIPTION &cmd
)
{
	if (paramIndex >= cmd.paramcount)
		return ERR_CODE_MAC_INVALID;
	
	uint64_t v = atoll(value.c_str());
	if (v == 0) {
		// check is a number
		std::string cc(value + "1");
		uint64_t c = atoll(cc.c_str());
		if (c == 0) {
			// NaN, return alias value if exists
			return paramValAlias(value, paramIndex, cmd);
		}
	}
	return paramValNum(v, paramIndex, cmd);
}

static void putMacCommand(
	MacData &retval,
	const COMMAND_DESCRIPTION &command,
	const std::vector<int> &paramval,
	bool sentByServerSide
)
{
	retval.set((enum MAC_CID) command.cmd, paramval, sentByServerSide);
}

/**
 * @brief parse already tokenized cmd array (from the command line)
 * @return 0- success, <0- error
 * states: 0- wait next command, 1- wait next parameters (if exists), 2- eof
 */ 
int MacGwConfig::parse(
	bool sentByServerSide
) {
	clear();
	int state = 0;
	std::vector<int> paramval;

	std::string v;
	int cmdIdx;
	int val;
	for (std::vector<std::string>::const_iterator it(cmd.begin()); it != cmd.end(); it++) {
		if (state == 2)
			break;
		v = *it;
		switch (state) {
			case 0:	// wait command mnemonic
				cmdIdx = commandIndex(v);
				if (cmdIdx < 0) {
					errcode = ERR_CODE_MAC_INVALID;
					errmessage = std::string(ERR_MAC_INVALID) + " '" + v + "'";
					return ERR_CODE_MAC_INVALID;
				}
				paramval.clear();
				if (gatewayCommands[cmdIdx].paramcount) {
					state = 1;
				} else {
					// got all parameters, next command
					MacData md;
					putMacCommand(md, gatewayCommands[cmdIdx], paramval, sentByServerSide);
					macCommands.list.push_back(md);
				}
				break;
			case 1:	// wait parameter
				val = paramValue(v, paramval.size(), gatewayCommands[cmdIdx]);
				if (val < 0) {
					errcode = ERR_CODE_PARAM_INVALID;
					errmessage = std::string(ERR_PARAM_INVALID) + " '" + v + "' of " 
						+ std::string(gatewayCommands[cmdIdx].fullname);
					return ERR_CODE_PARAM_INVALID;
				}
				paramval.push_back(val);
				if (paramval.size() >= gatewayCommands[cmdIdx].paramcount) {
					// got all parameters, next command
					MacData md;
					putMacCommand(md, gatewayCommands[cmdIdx], paramval, sentByServerSide);
					macCommands.list.push_back(md);
					state = 0;
					paramval.clear();
				}
				break;
			default:
				break;
		}
	}
	errcode = paramval.size() ? ERR_CODE_INSUFFICIENT_PARAMS : 0;
	if (errcode) {
		std::stringstream ss;
		ss << ERR_INSUFFICIENT_PARAMS << " for '" 
			<< gatewayCommands[cmdIdx].fullname << "'. Expected "
			<< (int) gatewayCommands[cmdIdx].paramcount << ", got " << paramval.size();
		errmessage = ss.str();
	}
	return errcode;
}

std::string macCommandlist() {
	std::stringstream ss;
	for (int i = 0; i < sizeof(gatewayCommands) / sizeof(COMMAND_DESCRIPTION); i++) {
		COMMAND_DESCRIPTION *c = &gatewayCommands[i];
		ss << std::left << std::setw(2) << c->shortname << " "
			<< std::setw(13) << c->fullname
			<< c->descpription << std::endl;
		// parameters
		for (int p = 0; p < c->paramcount; p++) {
			const COMMAND_PARAM *cp = (c->param[p]);
			ss << "     " << cp->name << ":";
			for (int c = 0; c < cp->count; c++) {
				const COMMAND_PARAM_CHOICE *ch = (cp->list[c]);
				if (c > 0)
					ss << ",";
				if (ch->maxvalue != ch->minvalue) {
					ss << " " << (int) ch->minvalue << ".." << (int) ch->maxvalue << ch->name;
				} else {
					ss << " " << ch->name << "(" << (int) ch->minvalue << ")";
				}
			}
			ss << std::endl;
		}
	}
	return ss.str();
}
