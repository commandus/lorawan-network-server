/**
 * @brief Send MAC command(s) to the end-device class C directly over specified gateway
 * @file mac-gw.cpp
 * Copyright (c) 2021 andrey.ivanov@ikfia.ysn.ru Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>

#include <signal.h>

#include "argtable3/argtable3.h"
#include "utilstring.h"

#include "errlist.h"
#include "utillora.h"

#include "config-json.h"
#include "macgw-config-json.h"
#include "identity-service-file-json.h"

#include "gateway-list.h"
#include "config-filename.h"

const std::string programName = "mac-gw";
#define DEF_CONFIG_FILE_NAME "mac-gw.json"
#define DEF_IDENTITY_STORAGE_NAME "identity.json"
#define DEF_GATEWAYS_STORAGE_NAME "gateway.json"

static Configuration *config = NULL;
static MacGwConfig *macGwConfig = NULL;
static GatewayList *gatewayList = NULL;

static void done()
{
	// destroy and free all
	if (gatewayList) {
		delete gatewayList;
		gatewayList = NULL;
	}
	exit(0);
}

static void stop()
{
}

void signalHandler(int signal)
{
	switch (signal)
	{
	case SIGINT:
		std::cerr << MSG_INTERRUPTED << std::endl;
		stop();
		done();
		break;
	default:
		break;
	}
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = &signalHandler;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
}
#endif

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
	Configuration *programConfig,
	MacGwConfig *macGwConfig,
	int argc,
	char *argv[])
{
	// device path
	struct arg_str *a_command = arg_strn(NULL, NULL, "<command>", 0, 255, "mac command");
	struct arg_str *a_config = arg_str0("c", "programConfig", "<file>", "configuration file. Default ./" DEF_CONFIG_FILE_NAME ". ~/" DEF_CONFIG_FILE_NAME);
	//  ", storage ~/" DEF_IDENTITY_STORAGE_NAME ", gateways ~/" DEF_GATEWAYS_STORAGE_NAME );
	struct arg_str *a_gatewayid = arg_strn("g", "gateway", "<id>", 0, 100, "gateway identifier. Mask \"*\" - all");
	struct arg_str *a_gatewayname = arg_strn("G", "gatewayname", "<name>", 0, 100, "gateway name. Mask \"*\" - all");
	struct arg_str *a_eui = arg_strn("e", "eui", "<id>", 0, 100, "end-device identifier. Mask \"*\" - all");
	struct arg_str *a_devicename = arg_strn("E", "devicename", "<name>", 0, 100, "end-device name. Mask \"*\" - all");
	struct arg_str *a_payload_hex = arg_str0("x", "payload", "<hex>", "payload bytes, in hex");

	struct arg_int *a_gateway_port = arg_int0("p", "port", "<1..65535>", "gateway port, default 4242");
	
	struct arg_lit *a_regex = arg_lit0("x", "regex", "Use regular expression in -g, -e options. By default wildcards (*, ?) can be used.");
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void *argtable[] = {
		a_config,
		a_command, a_gatewayid, a_gatewayname, a_eui, a_devicename, a_payload_hex, a_gateway_port,
		a_regex, a_verbosity, a_help, a_end};

	int nerrors;

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}
	// Parse the command line as defined by argtable[]
	nerrors = arg_parse(argc, argv, argtable);

	if (a_config->count)
        programConfig->configFileName = getDefaultConfigFileName(argv[0], *a_config->sval);
	else
        programConfig->configFileName = getDefaultConfigFileName(argv[0], DEF_CONFIG_FILE_NAME);

    programConfig->serverConfig.daemonize = false;
    programConfig->serverConfig.verbosity = a_verbosity->count;

	if (!nerrors) {
		for (int i = 0; i < a_command->count; i++) {
			macGwConfig->cmd.push_back(a_command->sval[i]);
		}
		for (int i = 0; i < a_gatewayid->count; i++) {
			macGwConfig->gatewayMasks.push_back(a_gatewayid->sval[i]);
		}
		for (int i = 0; i < a_gatewayname->count; i++) {
			macGwConfig->gatewayNames.push_back(a_gatewayname->sval[i]);
		}
		for (int i = 0; i < a_eui->count; i++) {
			macGwConfig->euiMasks.push_back(a_eui->sval[i]);
		}
		for (int i = 0; i < a_devicename->count; i++) {
			macGwConfig->deviceNames.push_back(a_devicename->sval[i]);
		}
		if (a_payload_hex->count) {
			macGwConfig->payload = hex2string(*a_payload_hex->sval);
		}
		
		if (a_gateway_port->count) {
            programConfig->gatewayPort = (*a_gateway_port->ival);
		}
	}

	macGwConfig->useRegex = a_regex->count > 0;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, programName.c_str());
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		// print commands available
		std::cerr << MSG_MAC_COMMANDS << ": " << std::endl << macCommandlist() << std::endl;
		return ERR_CODE_COMMAND_LINE;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

void onLog(
	int level,
	int modulecode,
	int errorcode,
	const std::string &message
)
{
	std::cerr << message << std::endl;
}

static void run()
{
	if (!macGwConfig)
		return;
	macGwConfig->parse(true);
	if (macGwConfig->errcode) {
		std::cerr << "Error " << macGwConfig->errcode << ": " 
			<< macGwConfig->errmessage << std::endl;
		return;
	}
}

int main(
	int argc,
	char *argv[])
{
	config = new Configuration("");
	macGwConfig = new MacGwConfig();
	// load config file and get macGwConfig from the command line
	if (parseCmd(config, macGwConfig, argc, argv) != 0) {
		exit(ERR_CODE_COMMAND_LINE);
	}
	// reload config if required
	bool hasConfig = false;
    if (config) {
        if (!config->configFileName.empty()) {
            std::string js = file2string(config->configFileName.c_str());
            if (!js.empty()) {
                config->parse(js.c_str());
                hasConfig = true;
            }
        }
    }
	if (!hasConfig) {
		std::cerr << ERR_NO_CONFIG << std::endl;
		exit(ERR_CODE_NO_CONFIG);
	}

	// get gateway and device list storage path from the config file
	if (config->serverConfig.identityStorageName.empty())
		config->serverConfig.identityStorageName = getDefaultConfigFileName(argv[0], DEF_IDENTITY_STORAGE_NAME);
	else
        config->serverConfig.identityStorageName = getDefaultConfigFileName(argv[0], config->serverConfig.identityStorageName);

	if (config->gatewaysFileName.empty())
		config->gatewaysFileName = getDefaultConfigFileName(argv[0], DEF_GATEWAYS_STORAGE_NAME);
    else
        config->gatewaysFileName = getDefaultConfigFileName(argv[0], config->gatewaysFileName);

    // load gateway list
	gatewayList = new GatewayList(config->gatewaysFileName);
	
	if (config->serverConfig.verbosity > 2)
		std::cerr << "Config: " << std::endl << config->toString() << std::endl;
	if (config->serverConfig.verbosity > 2)
		std::cerr << "Gateways: " << std::endl <<gatewayList->toJsonString() << std::endl;
	
	// parseRX gateway ids, expand regex
	int r;	
	if ((r = gatewayList->parseIdentifiers(macGwConfig->gatewayIds, macGwConfig->gatewayMasks, macGwConfig->useRegex))) {
		std::cerr << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r) << std::endl;
		exit(ERR_CODE_INVALID_GATEWAY_ID);
	}
	if ((r = gatewayList->parseNames(macGwConfig->gatewayIds, macGwConfig->gatewayNames, macGwConfig->useRegex))) {
		std::cerr << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r) << std::endl;
		exit(ERR_CODE_INVALID_GATEWAY_ID);
	}

	// load device list
	JsonFileIdentityService identityService;
	if (identityService.init(config->serverConfig.identityStorageName, NULL) != 0) {
		std::cerr << ERR_MESSAGE << identityService.errMessage << std::endl;
	}
	if (config->serverConfig.verbosity > 2)
		std::cerr << "Devices: " << std::endl <<identityService.toJsonString() << std::endl;

	// parseRX device ids, expand regex
	if (identityService.parseIdentifiers(macGwConfig->euis, macGwConfig->euiMasks, macGwConfig->useRegex)) {
		std::cerr << ERR_INVALID_DEVICE_EUI << std::endl;
		exit(ERR_CODE_INVALID_DEVICE_EUI);
	}
	// parseRX device names, expand regex
	if (identityService.parseNames(macGwConfig->euis, macGwConfig->deviceNames, macGwConfig->useRegex)) {
		std::cerr << ERR_INVALID_DEVICE_EUI << std::endl;
		exit(ERR_CODE_INVALID_DEVICE_EUI);
	}

	// check at least one device specified
	if (macGwConfig->euis.size() == 0) {
		std::cerr << ERR_MISSED_DEVICE << std::endl;
		exit(ERR_CODE_MISSED_DEVICE);
	}

	// check at least one device specified
	if (macGwConfig->gatewayIds.size() == 0) {
		std::cerr << ERR_MISSED_GATEWAY << std::endl;
		exit(ERR_CODE_MISSED_GATEWAY);
	}

	// parseRX MAC command(s)
	r = macGwConfig->parse(true);
	if (r) {
		std::cerr << "MAC command error " << r << ": " << macGwConfig->errmessage << std::endl;
		exit(r);
	}

	// check MAC commands
	if ((macGwConfig->macCommands.list.size() == 0) && (macGwConfig->payload.size() == 0)) {
		std::cerr << ERR_NO_MAC_NO_PAYLOAD << std::endl;
		exit(ERR_CODE_NO_MAC_NO_PAYLOAD);
	}
	// form MAC data ?!!
	std::vector<MacData> md;
	for (int i = 0; i < macGwConfig->macCommands.list.size(); i++) {
		// std::cerr << macGwConfig->macCommands.list[i].toJSONString() << std::endl;
		MacData d(macGwConfig->macCommands.list[i].toString(), true);
		md.push_back(d);
	}

	if (config->serverConfig.verbosity > 2) {
		std::cerr << "command line parameters: " << std::endl << macGwConfig->toJsonString() << std::endl << std::endl;
		std::cerr << std::dec << std::endl;
		for (int i = 0; i < macGwConfig->macCommands.list.size(); i++) {
			std::cerr << macGwConfig->macCommands.toJSONString() << std::endl << std::endl;
		}

		std::cerr << "MAC: " << std::endl;
		for (std::vector<MacData>::const_iterator it (md.begin()); it != md.end(); it++) {
			std::cerr << "\t" << it->toJSONString() 
				<< ", hex: " << hexString(it->toString())
				<< std::endl;
		}
	}

	std::stringstream ss;
	for (std::vector<MacData>::const_iterator it (md.begin()); it != md.end(); it++) {
		ss << it->toString();
	}
	std::string macdatabin = ss.str();
	if (config->serverConfig.verbosity > 2) {
		std::cerr << "MAC data: " << hexString(macdatabin) << ", " << macdatabin.size() << " bytes." <<  std::endl;
	}

	for (int g = 0; g < macGwConfig->gatewayIds.size(); g++) {
		// open socket
		std::string a = gatewayList->getAddress(macGwConfig->gatewayIds[g]);
		if (a.find(":") == std::string::npos) {
			// add port
			std::stringstream ss;
			ss << a << ":" << config->gatewayPort;
			a = ss.str();
		}
		UDPSocket socket(a, MODE_OPEN_SOCKET_CONNECT, MODE_FAMILY_HINT_UNSPEC);
		if (socket.errcode) {
			std::cerr << ERR_MESSAGE << socket.errcode << ": " << strerror_lorawan_ns(socket.errcode)
				<< ", gateway " << std::hex << macGwConfig->gatewayIds[g] << std::dec << " address: " << a
				<< std::endl;
			exit(socket.errcode);
		}
		for (int d = 0; d < macGwConfig->euis.size(); d++) {
			NetworkIdentity netId;
			if (identityService.getNetworkIdentity(netId, macGwConfig->euis[d].eui) != 0) {
				std::cerr << ERR_INVALID_DEVICE_EUI << std::endl;
				exit(ERR_CODE_INVALID_DEVICE_EUI);
			}
			// compose packet
			SemtechUDPPacket packet;
			packet.setFOpts(macdatabin);
			memmove(packet.prefix.mac, netId.devEUI, sizeof(DEVEUI));
			memmove(packet.header.header.devaddr, netId.devaddr, sizeof(DEVADDR));
			packet.devId = netId;
			//memmove(packet.devId.devEUI, netId.devEUI, sizeof(DEVEUI));

			// TODO form correct data
			// packet.setPayload();
			std::string s = packet.toString();
			if (config->serverConfig.verbosity > 0)
				std::cerr << MSG_SEND_TO 
					<< UDPSocket::addrString(&socket.addrStorage)
					<< " " << s.size() << " bytes, "
					<< "packet: " << packet.toJsonString()
					<< ", hex: " << hexString(s)
					<< std::endl;
		}
	}

#ifdef _MSC_VER
#else
	setSignalHandler();
#endif

	run();
	done();
	return 0;
}
