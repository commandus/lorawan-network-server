/**
 * @dile test-mac-gw.cpp
 */ 
#include <iostream>

#include "macgw-config-json.h"

#include "errlist.h"

void printSyntax() {
	std::string s = macCommandlist();
	std::cerr << s << std::endl;
}

void checkReset() {
	MacGwConfig c;
	c.cmd.push_back("reset");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "reset: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "       " << d.toHexString() << std::endl;
	std::cerr << "       " << d.toJSONString() << std::endl;
}

void checkLinkCheck() {
	MacGwConfig c;
	c.cmd.push_back("linkcheck");
	c.cmd.push_back("1");
	c.cmd.push_back("2");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "linkcheck: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "           " << d.toHexString() << std::endl;
	std::cerr << "           " << d.toJSONString() << std::endl;
}

void checkLinkAdr() {
	MacGwConfig c;
	c.cmd.push_back("linkadr");
	c.cmd.push_back("1");
	c.cmd.push_back("2");
	c.cmd.push_back("3");
	c.cmd.push_back("4");
	c.cmd.push_back("5");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "linkadr: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "         " << d.toHexString() << std::endl;
	std::cerr << "         " << d.toJSONString() << std::endl;
}

void checkDutyCycle() {
	MacGwConfig c;
	c.cmd.push_back("dutycycle");
	c.cmd.push_back("1");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "dutycycle: " << c.macCommands.toHexString() << std::endl;
	std::cerr << "dutycycle: " << c.macCommands.toJSONString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "           " << d.toHexString() << std::endl;
	std::cerr << "           " << d.toJSONString() << std::endl;
}

void checkRXParamSetup() {
	MacGwConfig c;
	c.cmd.push_back("rxparamsetup");
	c.cmd.push_back("7999999");
	c.cmd.push_back("7");
	c.cmd.push_back("15");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "rxparamsetup: " << c.macCommands.toHexString() << std::endl;
	std::cerr << "rxparamsetup: " << c.macCommands.toJSONString() << std::endl;
	
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "              " << d.toHexString() << std::endl;
	std::cerr << "              " << d.toJSONString() << std::endl;
}

void checkDevStatus() {
	MacGwConfig c;
	c.cmd.push_back("devstatus");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "devstatus: " << c.macCommands.toHexString() << std::endl;
	std::cerr << "devstatus: " << c.macCommands.toJSONString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "           " << d.toHexString() << std::endl;
	std::cerr << "           " << d.toJSONString() << std::endl;
}

void checkNewChannel() {
	MacGwConfig c;
	c.cmd.push_back("newchannel");
	c.cmd.push_back("1");
	c.cmd.push_back("7999999");
	c.cmd.push_back("3");
	c.cmd.push_back("4");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "newchannel: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "            " << d.toHexString() << std::endl;
	std::cerr << "            " << d.toJSONString() << std::endl;
}

void checkRXTimingSetup() {
	MacGwConfig c;
	c.cmd.push_back("rxtiming");
	c.cmd.push_back("1");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "rxtiming: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "          " << d.toHexString() << std::endl;
	std::cerr << "          " << d.toJSONString() << std::endl;
}

void checkTXParamSetup() {
	MacGwConfig c;
	c.cmd.push_back("dwelltime");
	c.cmd.push_back("1");
	c.cmd.push_back("2");
	c.cmd.push_back("3");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "dwelltime: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "           " << d.toHexString() << std::endl;
	std::cerr << "           " << d.toJSONString() << std::endl;
}

void checkDLChannel() {
	MacGwConfig c;
	c.cmd.push_back("dlchannel");
	c.cmd.push_back("1");
	c.cmd.push_back("7999999");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "dlchannel: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "           " << d.toHexString() << std::endl;
	std::cerr << "           " << d.toJSONString() << std::endl;
}

void checkRekey() {
	MacGwConfig c;
	c.cmd.push_back("rekey");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "rekey: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "       " << d.toHexString() << std::endl;
	std::cerr << "       " << d.toJSONString() << std::endl;
}

void checkADRParamSetup() {
	MacGwConfig c;
	c.cmd.push_back("acklimit");
	c.cmd.push_back("1");
	c.cmd.push_back("2");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "acklimit: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "          " << d.toHexString() << std::endl;
	std::cerr << "          " << d.toJSONString() << std::endl;
}

void checkDeviceTime() {
	MacGwConfig c;
	c.cmd.push_back("devicetime");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "devicetime: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "            " << d.toHexString() << std::endl;
	std::cerr << "            " << d.toJSONString() << std::endl;
}

void checkForceRejoin() {
	MacGwConfig c;
	c.cmd.push_back("forcerejoin");
	c.cmd.push_back("1");
	c.cmd.push_back("2");
	c.cmd.push_back("3");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "forcerejoin: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "             " << d.toHexString() << std::endl;
	std::cerr << "             " << d.toJSONString() << std::endl;
}

void checkRejoinParamSetup() {
	MacGwConfig c;
	c.cmd.push_back("rejoinsetup");
	c.cmd.push_back("7999999");
	c.cmd.push_back("2");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "rejoinsetup: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "             " << d.toHexString() << std::endl;
	std::cerr << "             " << d.toJSONString() << std::endl;
}

void checkPingSlotInfo() {
	MacGwConfig c;
	c.cmd.push_back("ping");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "ping: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "      " << d.toHexString() << std::endl;
	std::cerr << "      " << d.toJSONString() << std::endl;
}

void checkPingSlotChannel() {
	MacGwConfig c;
	c.cmd.push_back("pingchannel");
	c.cmd.push_back("1");
	c.cmd.push_back("2");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "rejoinsetup: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "             " << d.toHexString() << std::endl;
	std::cerr << "             " << d.toJSONString() << std::endl;
}

void checkBeaconTiming() {
	MacGwConfig c;
	c.cmd.push_back("beacontiming");
	c.cmd.push_back("1");
	c.cmd.push_back("2");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "beacontiming: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "              " << d.toHexString() << std::endl;
	std::cerr << "              " << d.toJSONString() << std::endl;
}

void checkBeaconFreq() {
	MacGwConfig c;
	c.cmd.push_back("beaconfreq");
	c.cmd.push_back("0x01eeff");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "beaconfreq: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "            " << d.toHexString() << std::endl;
	std::cerr << "            " << d.toJSONString() << std::endl;
}

void checkDeviceMode() {
	MacGwConfig c;
	c.cmd.push_back("mode");
	c.cmd.push_back("1");
	int r = c.parse(true);
	if (r) {
		std::cerr << "Error " << r << ": " << c.errmessage << std::endl;
		return;
	}
	std::cerr << "mode: " << c.macCommands.toHexString() << std::endl;
	MacData d (c.macCommands.list[0].toString(), true);
	std::cerr << "      " << d.toHexString() << std::endl;
	std::cerr << "      " << d.toJSONString() << std::endl;
}

int main(int argc, char **argv) {
	checkBeaconFreq();

	printSyntax();
	// checkReset();
	// checkLinkCheck();
	checkLinkAdr();
	checkDutyCycle();
	checkRXParamSetup();
	checkDevStatus();
	checkNewChannel();
	checkRXTimingSetup();
	checkTXParamSetup();
	checkDLChannel();
	checkRekey();
	checkADRParamSetup();
	// checkDeviceTime();
	checkForceRejoin();
	checkRejoinParamSetup();
	checkPingSlotInfo();
	checkPingSlotChannel();
	// checkBeaconTiming();
	checkBeaconFreq();
	// checkDeviceMode();
}
