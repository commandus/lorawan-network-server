/**
 * @brief Send MAC command(s) to the end-device class C directly over specified gateway
 * @file mac-gw.cpp
 * Copyright (c) 2021 andrey.ivanov@ikfia.ysn.ru Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <iostream>
#include <cstring>
#include <fstream>

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>

#include "argtable3/argtable3.h"
#include "platform.h"
#include "utilstring.h"
#include "daemonize.h"

#include "errlist.h"
#include "utillora.h"
#include "utilstring.h"
#include "utildate.h"

#include "udp-emitter.h"
#include "udp-listener.h"
#include "config-json.h"
#include "macgw-config-json.h"
#include "lora-packet-handler-impl.h"
#include "identity-service-file-json.h"

#include "gateway-list.h"
#include "config-filename.h"

const std::string progname = "mac-gw";
#define DEF_CONFIG_FILE_NAME ".mac-gw.json"
#define DEF_IDENTITY_STORAGE_NAME "identity.json"
#define DEF_GATEWAYS_STORAGE_NAME "gateway.json"
#define DEF_TIME_FORMAT "%FT%T"

static Configuration *config = NULL;
static MacGwConfig *macGwConfig = NULL;
static GatewayList *gatewayList = NULL;

static UDPEmitter emitter;

static void done()
{
	// destroy and free all
	if (config && config->serverConfig.verbosity > 1)
		std::cerr << MSG_GRACEFULLY_STOPPED << std::endl;
	if (gatewayList) {
		delete gatewayList;
		gatewayList = NULL;
	}
	exit(0);
}

static void stop()
{
	emitter.stopped = true;
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
	Configuration *config,
	MacGwConfig *macGwConfig,
	int argc,
	char *argv[])
{
	// device path
	struct arg_str *a_command = arg_strn(NULL, NULL, "<command>", 0, 255, "mac command");
	struct arg_str *a_config = arg_str0("c", "config", "<file>", "configuration file. Default ~/" DEF_CONFIG_FILE_NAME ", storage ~/" DEF_IDENTITY_STORAGE_NAME ", gateways ~/" DEF_GATEWAYS_STORAGE_NAME );
	struct arg_str *a_gatewayid = arg_strn("g", "gateway", "<id>", 0, 100, "gateway identifier, *- all");
	struct arg_str *a_gatewayname = arg_strn("G", "gatewayname", "<name>", 0, 100, "gateway name, *- all");
	struct arg_str *a_eui = arg_strn("e", "eui", "<id>", 0, 100, "end-device identifier, *- all");
	struct arg_str *a_devicename = arg_strn("E", "devicename", "<name>", 0, 100, "end-device name, *- all");
	struct arg_str *a_payload_hex = arg_str0("p", "payload", "<hex>", "payload bytes, in hex");
	
	struct arg_lit *a_regex = arg_lit0("x", "regex", "Use regular expression in -g, -e options. By default wildcards (*, ?) can be used.");
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void *argtable[] = {
		a_config,
		a_command, a_gatewayid, a_gatewayname, a_eui, a_devicename, a_payload_hex,
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
		config->configFileName = *a_config->sval;
	else
		config->configFileName = getDefaultConfigFileName(DEF_CONFIG_FILE_NAME);

	config->serverConfig.daemonize = false;
	config->serverConfig.verbosity = a_verbosity->count;

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
	}

	macGwConfig->useRegex = a_regex->count > 0;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		// print commands avaliable
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
	if (!config->configFileName.empty()) {
		std::string js = file2string(config->configFileName.c_str());
		if (!js.empty()) {
			config->parse(js.c_str());
			hasConfig = true;
		}
	}
	if (!hasConfig) {
		std::cerr << ERR_NO_CONFIG << std::endl;
		exit(ERR_CODE_NO_CONFIG);
	}

	// get gateway and device list storage path from the config file
	if (config->serverConfig.identityStorageName.empty()) {
		config->serverConfig.identityStorageName = getDefaultConfigFileName(DEF_IDENTITY_STORAGE_NAME);
	}
	if (config->gatewaysFileName.empty()) {
		config->gatewaysFileName = getDefaultConfigFileName(DEF_GATEWAYS_STORAGE_NAME);
	}
	
	// load gateway list
	gatewayList = new GatewayList(config->gatewaysFileName);
	
	if (config->serverConfig.verbosity > 2)
		std::cerr << "Config: " << std::endl << config->toString() << std::endl;
	if (config->serverConfig.verbosity > 2)
		std::cerr << "Gateways: " << std::endl <<gatewayList->toJsonString() << std::endl;
	
	// parse gateway ids, expand regex 
	int r;	
	if ((r = gatewayList->parseIdentifiers(macGwConfig->gatewayIds, macGwConfig->gatewayMasks, macGwConfig->useRegex))) {
		std::cerr << ERR_MESSAGE << r << ": " << strerror_client(r) << std::endl;
		exit(ERR_CODE_INVALID_GATEWAY_ID);
	}
	if ((r = gatewayList->parseNames(macGwConfig->gatewayIds, macGwConfig->gatewayNames, macGwConfig->useRegex))) {
		std::cerr << ERR_MESSAGE << r << ": " << strerror_client(r) << std::endl;
		exit(ERR_CODE_INVALID_GATEWAY_ID);
	}
	if (macGwConfig->gatewayIds.size() == 0) {
		// At leaast one gateway required
		std::cerr << ERR_MESSAGE << ERR_CODE_INVALID_GATEWAY_ID << ": " << ERR_INVALID_GATEWAY_ID << std::endl;
		exit(ERR_CODE_INVALID_GATEWAY_ID);
	}

	// load device list
	JsonFileIdentityService identityService;
	if (identityService.init(config->serverConfig.identityStorageName, NULL) != 0) {
		std::cerr << ERR_MESSAGE << identityService.errmessage << std::endl;
	}
	if (config->serverConfig.verbosity > 2)
		std::cerr << "Devices: " << std::endl <<identityService.toJsonString() << std::endl;

	// parse device ids, expand regex
	if (identityService.parseIdentifiers(macGwConfig->euis, macGwConfig->euiMasks, macGwConfig->useRegex)) {
		std::cerr << ERR_INVALID_DEVICE_EUI << std::endl;
		exit(ERR_CODE_INVALID_DEVICE_EUI);
	}
	// parse device names, expand regex
	if (identityService.parseNames(macGwConfig->euis, macGwConfig->deviceNames, macGwConfig->useRegex)) {
		std::cerr << ERR_INVALID_DEVICE_EUI << std::endl;
		exit(ERR_CODE_INVALID_DEVICE_EUI);
	}

	// parse MAC command(s)
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
	if (config->serverConfig.verbosity > 2)
		std::cerr << "command line parameters: " << std::endl << macGwConfig->toJsonString() << std::endl;

	// form MAC data
	std::vector<MacData> md;
	for (int i = 0; i < macGwConfig->macCommands.list.size(); i++) {
		MacData d(macGwConfig->macCommands.list[i].toString(), true);
		md.push_back(d);
	}

	for (int i = 0; i < macGwConfig->gatewayIds.size(); i++) {
		// open socket
		UDPSocket socket(gatewayList->getAddress(macGwConfig->gatewayIds[i]), MODE_OPEN_SOCKET_CONNECT, MODE_FAMILY_HINT_UNSPEC);
		if (socket.errcode) {
			std::cerr << ERR_MESSAGE << socket.errcode << ": " << strerror_client(socket.errcode) << std::endl;
			exit(socket.errcode);
		}
		for (int d = 0; d < macGwConfig->euis.size(); d++) {
			NetworkIdentity netId;
			if (identityService.getNetworkIdentity(netId, macGwConfig->euis[i].eui) != 0) {
				std::cerr << ERR_INVALID_DEVICE_EUI << std::endl;
				exit(ERR_CODE_INVALID_DEVICE_EUI);
			}
			// compose packet
			semtechUDPPacket packet;
			std::string s = packet.toString();
			if (config->serverConfig.verbosity > 0)
				std::cerr << MSG_SEND_TO 
					<< UDPSocket::addrString(&socket.addrStorage)
					<< s.size() << " bytes, "
					<< "packet: " << hexString(s)
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
