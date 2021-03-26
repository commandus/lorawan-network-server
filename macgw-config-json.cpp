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

//
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

static const COMMAND_PARAM_CHOICE CHOICE_TXPARAMSETUP = {
	0, 15, " + 1s"
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

static const COMMAND_PARAM_CHOICE *CHOICES_DOWNLINK_DWELLTIME[] = { &CHOICE_DOWNLINK_DWELLTIME };
static const COMMAND_PARAM_CHOICE *CHOICES_UPLINK_DWELLTIME[] = { &CHOICE_UPLINK_DWELLTIME };
static const COMMAND_PARAM_CHOICE *CHOICES_MAXEIRP[] = { &CHOICES_MAXEIRP };

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
	1, CHOICES_DOWNLINK_DWELLTIME
};

static const COMMAND_PARAM &PARAM_UPLINK_DWELLTIME = {
	"uplink dwell time",
	1, CHOICES_UPLINK_DWELLTIME
};

static const COMMAND_PARAM PARAM_MAXEIRP = {
	"max EIRP",
	1, CHOICES_MAXEIRP
};

// ------------------------- Params -------------------------

const COMMAND_PARAM *LINK_CHECK_PARAMS[] = { &PARAM_MARGIN, &PARAM_GWCNT };

const COMMAND_PARAM *LINK_ADR_PARAMS[] = { &PARAM_TXPOWER, &PARAM_DATARATE,
	&PARAM_CHMASK, &PARAM_NBTRANS, &PARAM_CHMASKCTL };

const COMMAND_PARAM *DUTY_CYCLE_PARAMS[] = { &PARAM_DUTY_CYCLE_LIMIT };

const COMMAND_PARAM *RXPARAMSETUP_PARAMS[] = { &PARAM_FREQUENCY, &PARAM_RX1_DR_OFFSET, &PARAM_DATARATE};

const COMMAND_PARAM *NEWS_CHANNEL_PARAMS[] = { &PARAM_CHANNEL_INDEX, &PARAM_FREQUENCY, &PARAM_MIN_DR, &PARAM_MAX_DR };

const COMMAND_PARAM *RXTIMING_PARAMS[] = { &PARAM_RXTIMING };

const COMMAND_PARAM *TXPARAMSETUP_PARAMS[] = { &PARAM_DOWNLINK_DWELLTIME, &PARAM_UPLINK_DWELLTIME, &PARAM_MAXEIRP };

// ------------------------- Commands -------------------------

static COMMAND_DESCRIPTION validCommands[] = {
	{ Reset , 0, "r", "reset", "Reset end-device request", {}},	
	{ LinkCheck, 2, "l", "linkcheck", "Check link answer", LINK_CHECK_PARAMS },	 
	{ LinkADR, 5, "a", "linkadr", "Rate adaptation", LINK_ADR_PARAMS },
	{ DutyCycle, 1, "d", "dutycycle", "Limit transmit duty cycle", DUTY_CYCLE_PARAMS },
	{ RXParamSetup ,3,  "rx", "rxparamsetup", "Change frequency/data RX2", RXPARAMSETUP_PARAMS },
	{ DevStatus, 0, "s", "devstatus", "Request device battery, temperature" },
	{ NewChannel, 4, "n", "newchannel", "Set channel frequency/ data rate", NEWS_CHANNEL_PARAMS },
	{ RXTimingSetup, 1, "rx", "rxtiming", "Set delay between TX and RX1", RXTIMING_PARAMS },
	{ TXParamSetup, 3, "tx", "dwelltime", "Set maximum allowed dwell time", TXPARAMSETUP_PARAMS },
	{ DLChannel, 0, "dl", "dlchannel", "Set RX1 slot frequency" },
	{ Rekey, 0, "k", "rekey", "Answer security OTA key update" },
	{ ADRParamSetup, 0, "al", "acklimit", "Set ADR_ACK_LIMIT, ADR_ACK_DELAY" },
	{ DeviceTime, 0, "t", "devicetime", "Answer date/time" },
	{ ForceRejoin, 0, "j", "forcerejoin", "Request immediately Rejoin-Request" },
	{ RejoinParamSetup, 0, "js", "rejoinsetup", "Request periodically send Rejoin-Request" },
	{ PingSlotInfo, 0, "p", "ping", "Answer to unicast ping slot" },
	{ PingSlotChannel, 0, "pc", "pingchannel", "Set ping slot channel" },
	{ BeaconTiming, 0, "bt", "beacontiming", "Deprecated" },
	{ BeaconFreq, 0, "bf", "beaconfreq", "Set beacon frequency" },
	{ DeviceMode, 0, "m", "mode", "Set Set device mode" }
};

MacGwConfig::MacGwConfig() 
	: gatewayId(""), eui(""), errcode(0), errmessage("")
{

}

void MacGwConfig::clear() {
	macCommands.list.clear();
	errcode = 0;
}

int MacGwConfig::parse() {
	clear();
	int state = 0;
	for (std::vector<std::string>::const_iterator it(cmd.begin()); it != cmd.end(); it++) {
		switch (state) {
			case 0:	// wait command mnemonic

				break;

		}
	}
}

std::string macCommandlist() {
	std::stringstream ss;
	for (int i = 0; i < sizeof(validCommands) / sizeof(COMMAND_DESCRIPTION); i++) {
		COMMAND_DESCRIPTION *c = &validCommands[i];
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
