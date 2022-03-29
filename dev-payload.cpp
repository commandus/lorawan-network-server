/**
 * @brief Simulate sending payload to the network server from device
 * @file dev-payload.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <iostream>
#include <cstring>
#include <fstream>

#include <sys/time.h>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <climits>
#include <cerrno>
#include <sys/select.h>

#include "argtable3/argtable3.h"

#include "platform.h"
#include "utilstring.h"

#include "errlist.h"
#include "utillora.h"
#include "utilstring.h"
#include "udp-socket.h"

#include "identity-service-file-json.h"

#include "config-filename.h"

#define DEF_NS_PORT		        5000
#define MAX_RECV_BUFFER_SIZE	4096

const std::string programName = "dev-payload";

#define DEF_IDENTITY_STORAGE_NAME "identity.json"


static void done()
{
	// destroy and free all
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
	std::string &identityFileName,
	std::string &eui,
	std::string &deviceName,
    std::string &payload,
    std::string &network_server_address,
    int &verbosity,
	int argc,
	char *argv[])
{
	// device path
	struct arg_str *a_identity_fn = arg_str0("i", "identity", "<file>", "identity JSON file. Default ./" DEF_IDENTITY_STORAGE_NAME);

	struct arg_str *a_eui = arg_str0("e", "eui", "<id>", "end-device identifier.");
	struct arg_str *a_device_name = arg_str0("E", "devicename", "<name>", "end-device name.");

	struct arg_str *a_payload_hex = arg_str0("x", "payload", "<hex>", "payload bytes, in hex");
    struct arg_str *a_network_server_address = arg_str0("a", "address", "<IP:port>", "Send packet to network server (default port 5000");

	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void *argtable[] = {
        a_identity_fn, a_eui, a_device_name, a_payload_hex,
        a_network_server_address,
		a_verbosity, a_help, a_end};

	int nerrors;

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}
	// Parse the command line as defined by argtable[]
	nerrors = arg_parse(argc, argv, argtable);

    if (!nerrors) {
        if (a_identity_fn->count)
            identityFileName = *a_identity_fn->sval;
        else
            identityFileName = getDefaultConfigFileName(DEF_IDENTITY_STORAGE_NAME);

		if (a_eui->count) {
			eui = *a_eui->sval;
		}
		if (a_device_name->count) {
			deviceName = *a_device_name->sval;
		}
		if (a_payload_hex->count) {
			payload = hex2string(*a_payload_hex->sval);
		}
        if (a_network_server_address->count) {
            network_server_address = std::string(*a_network_server_address->sval);
        }
        verbosity = a_verbosity->count;
	}

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, programName.c_str());
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
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
	int rr = (int) recvfrom(socket.sock, (void*) recvBuffer.c_str(), MAX_RECV_BUFFER_SIZE, 0, (struct sockaddr*) &cliAddr, &cliAddrLen);
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
    std::vector<TDEVEUI> devEUIs;
    std::string identityFileName;
    std::string eui;
    std::string deviceName;
    std::string payload;
    std::string network_server_address;
    int verbosity;

#ifdef _MSC_VER
#else
    setSignalHandler();
#endif

    if (parseCmd(identityFileName, eui, deviceName, payload, network_server_address, verbosity, argc, argv) != 0) {
		exit(ERR_CODE_COMMAND_LINE);
	}

    JsonFileIdentityService identityService;
    // load device list
    if (identityService.init(identityFileName, NULL) != 0) {
        std::cerr << ERR_MESSAGE << identityService.errmessage << std::endl;
        exit(ERR_CODE_NO_CONFIG);
    }

    if (verbosity > 2) {
        std::vector<NetworkIdentity> identities;
        std::cerr << MSG_DEVICES << std::endl;
        identityService.list(identities, 0, 0);
        for (std::vector<NetworkIdentity>::const_iterator it(identities.begin()); it != identities.end(); it++) {
            std::cerr << "\t" << DEVADDR2string(it->devaddr)
                      << "\t" << DEVEUI2string(it->devEUI)
                      << "\t" << DEVICENAME2string(it->name);
            if (identityService.canControlService(it->devaddr))
                std::cerr << "\tmaster";
            std::cerr << std::endl;
        }
    }

	// try to find out EUI
	if (!eui.empty()) {
		std::vector <std::string> euis;
		euis.push_back(eui);
		if (identityService.parseIdentifiers(devEUIs, euis, false)) {
			std::cerr << ERR_MESSAGE << ERR_CODE_INVALID_DEVICE_EUI << ": " << ERR_INVALID_DEVICE_EUI << " " << eui << std::endl;
			exit(ERR_CODE_INVALID_DEVICE_EUI);
		}
	}

	// .. or by name
	if (!deviceName.empty()) {
		std::vector <std::string> deviceNames;
		deviceNames.push_back(deviceName);
		if (identityService.parseNames(devEUIs, deviceNames, false)) {
            std::cerr << ERR_MESSAGE << ERR_CODE_DEVICE_NAME_NOT_FOUND << ": "
                << ERR_DEVICE_NAME_NOT_FOUND << " (name: " << deviceName << ")" << std::endl;
			exit(ERR_CODE_DEVICE_NAME_NOT_FOUND);
		}
	}

	if (devEUIs.empty()) {
        std::cerr << ERR_MESSAGE << ERR_CODE_DEVICE_NAME_NOT_FOUND << ": "
                  << ERR_DEVICE_NAME_NOT_FOUND << " (name: " << deviceName << ")" << std::endl;
        exit(ERR_CODE_DEVICE_NAME_NOT_FOUND);
	}

    // device identity
    NetworkIdentity deviceNetId;
    if (identityService.getNetworkIdentity(deviceNetId, devEUIs[0].eui) != 0) {
        std::cerr << ERR_MESSAGE <<  ERR_CODE_INVALID_DEVICE_EUI << ": " << ERR_INVALID_DEVICE_EUI << std::endl;
        exit(ERR_CODE_INVALID_DEVICE_EUI);
    }


    // compose packet
    SemtechUDPPacket packet;
    rfmMetaData rfmMD;
    rfmMD.setDatr("SF7BW125");
    packet.metadata.push_back(rfmMD);
    // packet.setFOpts(macdatabin);
    packet.header.fport = 0;

    packet.payload = mkControlPacket(devEUIs[0].eui, macGwConfig->gatewayIds[g], macdatabin);
    std::cerr << "Payload: " << hexString(packet.payload) << std::endl;

    memmove(packet.prefix.mac, macGwConfig->euis[d].eui, sizeof(DEVEUI));
    memmove(packet.header.header.devaddr, netId.devaddr, sizeof(DEVADDR));
    packet.devId = netId;
    //memmove(packet.devId.devEUI, netId.devEUI, sizeof(DEVEUI));

    // TODO form correct data
    // packet.setPayload();


    // add default port value if missed
    if (network_server_address.empty()) {

        std::cout << packet.toJsonString() << std::endl;

    } else {
        if (network_server_address.find(":") == std::string::npos) {
            // add port
            std::stringstream ss;
            ss << network_server_address << ":" << DEF_NS_PORT;
            network_server_address = ss.str();
        }

        UDPSocket socket(address, MODE_OPEN_SOCKET_CONNECT, MODE_FAMILY_HINT_UNSPEC);

        if (socket.errcode) {
            std::cerr << ERR_MESSAGE << socket.errcode << ": " << strerror_lorawan_ns(socket.errcode)
                      << ", address: " << address
                      << std::endl;
            exit(socket.errcode);
        }

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
        if (verbosity > 0)
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

	done();
	return 0;
}
