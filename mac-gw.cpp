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
#define DEF_CONFIG_FILE_NAME ".mac-gw"
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
		gatewayList->save();
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
	struct arg_str *a_command = arg_strn(NULL, NULL, "<command>", 0, 8, "see ooc");
	struct arg_str *a_config = arg_str0("c", "config", "<file>", "configuration file. Default ~/" DEF_CONFIG_FILE_NAME ", stotage ~/" DEF_IDENTITY_STORAGE_NAME ", gateways ~/" DEF_GATEWAYS_STORAGE_NAME );
	struct arg_str *a_gatewayid = arg_str0("g", "gateway", "<id>", "gateway identifier");
	struct arg_str *a_eui = arg_str0("e", "eui", "<id>", "end-devide identifier");
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void *argtable[] = {
		a_config,
		a_command, a_gatewayid, a_eui, 
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

	if (a_config->count)
		config->configFileName = *a_config->sval;
	else
		config->configFileName = getDefaultConfigFileName(DEF_CONFIG_FILE_NAME);

	config->serverConfig.daemonize = false;
	config->serverConfig.verbosity = a_verbosity->count;

	if (!nerrors)
	{
		for (int i = 0; i < a_command->count; i++)
		{
			macGwConfig->cmd.push_back(a_command->sval[i]);
		}
	}

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors)
	{
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << MSG_PROG_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
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
	if (parseCmd(config, macGwConfig, argc, argv) != 0)
	{
		exit(ERR_CODE_COMMAND_LINE);
	}
	if (!config->configFileName.empty())
	{
		std::string js = file2string(config->configFileName.c_str());
		if (!js.empty())
		{
			config->parse(js.c_str());
		}
	}
	if (config->serverConfig.identityStorageName.empty()) {
		config->serverConfig.identityStorageName = getDefaultConfigFileName(DEF_IDENTITY_STORAGE_NAME);
	}
	if (config->gatewaysFileName.empty()) {
		config->gatewaysFileName = getDefaultConfigFileName(DEF_GATEWAYS_STORAGE_NAME);
	}
	std::cerr << config->toString() << std::endl;

	gatewayList = new GatewayList(config->gatewaysFileName);
	
	std::cerr << gatewayList->toJsonString() << std::endl;

	LoraPacketProcessor processor;
	JsonFileIdentityService identityService;
	identityService.init(config->serverConfig.identityStorageName, NULL);
	
	processor.setLogger(onLog);
	processor.setIdentityService(&identityService);

#ifdef _MSC_VER
#else
	setSignalHandler();
#endif
	run();
	done();
	return 0;
}
