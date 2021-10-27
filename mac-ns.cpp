/**
 * @brief Send MAC command(s) to the end-device class C directly over specified network server
 * @file mac-ns.cpp
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
#include <sys/select.h>

#include "argtable3/argtable3.h"
#include "platform.h"
#include "utilstring.h"
#include "daemonize.h"

#include "errlist.h"
#include "utillora.h"
#include "utilstring.h"
#include "utildate.h"

#include "udp-listener.h"
#include "config-json.h"
#include "macgw-config-json.h"
#include "lora-packet-handler-impl.h"
#include "identity-service-file-json.h"

#include "gateway-list.h"
#include "config-filename.h"
#include "control-packet.h"

const std::string programName = "mac-ns";
// same config as mac-gw
#define DEF_CONFIG_FILE_NAME "mac-gw.json"
#define DEF_IDENTITY_STORAGE_NAME "identity.json"
#define DEF_GATEWAYS_STORAGE_NAME "gateway.json"
#define DEF_TIME_FORMAT "%FT%T"
#define DEF_NS_PORT		5000
#define DEF_FPORT		223
#define DEF_S_FPORT		"223"
#define MAX_RECV_BUFFER_SIZE	4096

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
int parseCmd
(
	std::string &retNetworkServiceAaddress,
	int &retFPort, 
	std::string &master_eui,
	std::string &master_devicename,
	Configuration *config,
	MacGwConfig *macGwConfig,
	int argc,
	char *argv[])
{
	// device path
	struct arg_str *a_ns_address = arg_str1("a", "address", "<address:port>", "Address or name of the network service and port number (default port 5000)");
	struct arg_str *a_command = arg_strn(NULL, NULL, "<command>", 0, 255, "mac command");
	struct arg_str *a_config = arg_str0("c", "config", "<file>", "configuration file. Default ./" DEF_CONFIG_FILE_NAME ". ~/" DEF_CONFIG_FILE_NAME);
	struct arg_int *a_fport = arg_int0("f", "fport", "<1..223>", "FPort reserved by service, default " DEF_S_FPORT);
	//  ", storage ~/" DEF_IDENTITY_STORAGE_NAME ", gateways ~/" DEF_GATEWAYS_STORAGE_NAME );
	struct arg_str *a_gatewayid = arg_strn("g", "gateway", "<id>", 0, 100, "gateway identifier. Mask \"*\" - all");
	struct arg_str *a_gatewayname = arg_strn("G", "gatewayname", "<name>", 0, 100, "gateway name. Mask \"*\" - all");
	
	struct arg_str *a_master_eui = arg_str0("m", "master-eui", "<id>", "end-device identifier.");
	struct arg_str *a_master_devicename = arg_str0("M", "master-devicename", "<name>", "end-device name.");

	struct arg_str *a_eui = arg_strn("e", "eui", "<id>", 0, 100, "end-device identifier. Mask \"*\" - all");
	struct arg_str *a_devicename = arg_strn("E", "devicename", "<name>", 0, 100, "end-device name. Mask \"*\" - all");

	struct arg_str *a_payload_hex = arg_str0("x", "payload", "<hex>", "payload bytes, in hex");

	struct arg_int *a_gateway_port = arg_int0("p", "port", "<1..65535>", "gateway port, default 4242");
	
	struct arg_lit *a_regex = arg_lit0("x", "regex", "Use regular expression in -g, -e options. By default wildcards (*, ?) can be used.");
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void *argtable[] = {
		a_config, a_ns_address, a_fport,
		a_command, a_gatewayid, a_gatewayname,
		
		a_master_eui,
		a_master_devicename,

		a_eui, a_devicename,
		a_payload_hex, a_gateway_port,
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

	retNetworkServiceAaddress = "";
	retFPort = DEF_FPORT;

	if (!nerrors) {
		if (a_ns_address->count)
			retNetworkServiceAaddress = a_ns_address->sval[0];
		for (int i = 0; i < a_command->count; i++) {
			macGwConfig->cmd.push_back(a_command->sval[i]);
		}
		for (int i = 0; i < a_gatewayid->count; i++) {
			macGwConfig->gatewayMasks.push_back(a_gatewayid->sval[i]);
		}
		for (int i = 0; i < a_gatewayname->count; i++) {
			macGwConfig->gatewayNames.push_back(a_gatewayname->sval[i]);
		}

		if (a_master_eui->count) {
			master_eui = *a_master_eui->sval;
		}
		if (a_master_devicename->count) {
			master_devicename = *a_master_devicename->sval;
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
			config->gatewayPort = (*a_gateway_port->ival);
		}
		if (a_fport->count) {
			retFPort = (*a_fport->ival);
			if ((retFPort <= 0) || (retFPort > 223))
				nerrors++;
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

int recvACK
(
	UDPSocket &socket,
	uint64_t token,
	int timeoutSec
)
{
	fd_set readHandles;
	FD_ZERO(&readHandles);
	FD_SET(socket.sock, &readHandles);

	struct timeval timeoutInterval;
	timeoutInterval.tv_sec = timeoutSec;
	timeoutInterval.tv_usec = 0;

	int rs = select(socket.sock + 1, &readHandles, NULL, NULL, &timeoutInterval);
	// error or timeout

	if (rs <= 0)
		return rs;

	std::string recvBuffer;
	recvBuffer.resize(MAX_RECV_BUFFER_SIZE + 1);
	struct sockaddr_in6 cliAddr;
	socklen_t cliAddrLen = sizeof(cliAddr);
	int rr = recvfrom(socket.sock, (void*) recvBuffer.c_str(), MAX_RECV_BUFFER_SIZE, 0, (struct sockaddr*) &cliAddr, &cliAddrLen);
	if (rr < 0) { 
		std::cerr << ERR_SOCKET_READ
			<< " " << UDPSocket::addrString(&socket.addrStorage)
			<< " errno " << errno << ": " << strerror(errno)
			<< std::endl;
		return ERR_CODE_SOCKET_READ;
	}
	
	if (rr != sizeof(SEMTECH_ACK)) { 
		std::cerr << ERR_INVALID_PACKET
			<< " " << UDPSocket::addrString(&socket.addrStorage)
			<< " wrong size " << rr 
			<< std::endl;
		return ERR_CODE_INVALID_PACKET;
	}
	
	SEMTECH_ACK *ack = (SEMTECH_ACK *) recvBuffer.c_str();			// 4 bytes
	if (ack->version != 2) {
		std::cerr << ERR_INVALID_PACKET
			<< " " << UDPSocket::addrString(&socket.addrStorage)
			<< " wrong version " << (int) ack->version 
			<< std::endl;
		return ERR_CODE_INVALID_PACKET;
	}

	if (!(ack->tag == 1 || ack->tag == 4)) {
		std::cerr << ERR_INVALID_PACKET
			<< " " << UDPSocket::addrString(&socket.addrStorage)
			<< " wrong tag " << (int) ack->tag 
			<< std::endl;
		return ERR_CODE_INVALID_PACKET;
	}

	if (!(ack->tag == 1 || ack->tag == 4)) {
		std::cerr << ERR_INVALID_PACKET
			<< " " << UDPSocket::addrString(&socket.addrStorage)
			<< " wrong tag " << (int) ack->tag 
			<< std::endl;
		return ERR_CODE_INVALID_PACKET;
	}

	if (ack->token != token) {
		std::cerr << ERR_INVALID_PACKET
			<< " " << UDPSocket::addrString(&socket.addrStorage)
			<< " wrong token " << (int) ack->tag 
			<< std::endl;
		return ERR_CODE_INVALID_PACKET;
	}
	return LORA_OK;
}

int main(
	int argc,
	char *argv[])
{
	config = new Configuration("");
	macGwConfig = new MacGwConfig();
	// load config file and get macGwConfig from the command line
	std::string networkServiceAaddress;
	int fport;
	std::string master_eui = "";
	std::string master_devicename = "";
	// master device identifier, only one
	std::vector<TDEVEUI> masterDevEUIs;

	if (parseCmd(networkServiceAaddress, fport, 
		master_eui, master_devicename,
		config, macGwConfig, argc, argv) != 0) {
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
		std::cerr << ERR_MESSAGE << identityService.errmessage << std::endl;
	}
	if (config->serverConfig.verbosity > 2)
		std::cerr << "Devices: " << std::endl <<identityService.toJsonString() << std::endl;


	// try to find out master EUI
	if (!master_eui.empty()) {
		std::vector <std::string> master_euis;
		master_euis.push_back(master_eui);
		if (identityService.parseIdentifiers(masterDevEUIs, master_euis, false)) {
			std::cerr << ERR_INVALID_DEVICE_EUI << " " << master_eui << std::endl;
			exit(ERR_CODE_INVALID_DEVICE_EUI);
		}
	}

	// .. or by name
	if (!master_devicename.empty()) {
		std::vector <std::string> master_devicenames;
		master_devicenames.push_back(master_devicename);
		if (identityService.parseNames(masterDevEUIs, master_devicenames, false)) {
			std::cerr << ERR_INVALID_DEVICE_EUI << " " << master_eui << std::endl;
			exit(ERR_CODE_INVALID_DEVICE_EUI);
		}
	}

	if (masterDevEUIs.empty()) {
		std::cerr << ERR_CONTROL_DEVICE_NOT_FOUND << std::endl;
		exit(ERR_CODE_CONTROL_DEVICE_NOT_FOUND);
	}

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

	// add default port value if missed
	std::string address = networkServiceAaddress;
	if (address.find(":") == std::string::npos) {
		// add port
		std::stringstream ss;
		ss << address << ":" << DEF_NS_PORT;
		address = ss.str();
	}

	UDPSocket socket(address, MODE_OPEN_SOCKET_CONNECT, MODE_FAMILY_HINT_UNSPEC);
	
	if (socket.errcode) {
		std::cerr << ERR_MESSAGE << socket.errcode << ": " << strerror_lorawan_ns(socket.errcode)
			<< ", address: " << address
			<< std::endl;
		exit(socket.errcode);
	}

	// master device
	NetworkIdentity masterNetId;
	if (identityService.getNetworkIdentity(masterNetId, masterDevEUIs[0].eui) != 0) {
		std::cerr << ERR_INVALID_DEVICE_EUI << std::endl;
		exit(ERR_CODE_INVALID_DEVICE_EUI);
	}

	for (int d = 0; d < macGwConfig->euis.size(); d++) {
		for (int g = 0; g < macGwConfig->gatewayIds.size(); g++) {
			// target device
			NetworkIdentity netId;
			if (identityService.getNetworkIdentity(netId, macGwConfig->euis[d].eui) != 0) {
				std::cerr << ERR_INVALID_DEVICE_EUI << std::endl;
				exit(ERR_CODE_INVALID_DEVICE_EUI);
			}

			// compose packet
			SemtechUDPPacket packet;
			rfmMetaData rfmMD;
			packet.metadata.push_back(rfmMD);
			// packet.setFOpts(macdatabin);
			packet.header.fport = fport;

			packet.payload = mkControlPacket(macGwConfig->euis[d].eui, macGwConfig->gatewayIds[g], macdatabin);
			std::cerr << "==Payload: " << hexString(packet.payload) << std::endl;

			memmove(packet.prefix.mac, macGwConfig->euis[d].eui, sizeof(DEVEUI));
			memmove(packet.header.header.devaddr, netId.devaddr, sizeof(DEVADDR));
			packet.devId = netId;
			//memmove(packet.devId.deviceEUI, netId.deviceEUI, sizeof(DEVEUI));

			// TODO form correct data
			// packet.setPayload();
			std::string response = packet.toString();
			ssize_t r = sendto(socket.sock, response.c_str(), response.size(), 0,
				&socket.addrStorage,
				((socket.addrStorage.sa_family == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)));

			
			if (r != response.size())
				std::cerr << ERR_SOCKET_WRITE 
					<< " " << UDPSocket::addrString(&socket.addrStorage)
					<< " sendto() return " << r
					<< " errno " << errno << ": " << strerror(errno)
					<< std::endl;
			else
				if (config->serverConfig.verbosity > 0)
					std::cerr << MSG_SEND_TO 
						<< UDPSocket::addrString(&socket.addrStorage)
						<< " " << r << " bytes, "
						<< "packet: " << packet.toJsonString() << std::endl
						<< ", hex: " << hexString(response)
						<< std::endl;

			
			// read ACK response
			if (recvACK(socket, packet.prefix.token, 1) == 0) {
				std::cerr << "Received ACK successfully"
					<< std::endl;
			}
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
