#ifndef MACGW_CONFIG_JSON_H
#define MACGW_CONFIG_JSON_H 1

#include <string>
#include <vector>

#include "lorawan-mac.h"
#include "utillora.h"

class MacGwConfig {
	private:
		void clear();
	public:
		MacDataList macCommands;
		std::vector<uint64_t> gatewayIds;
		std::vector<TDEVEUI> euis;
		// read from command line
		std::vector<std::string> cmd;
		int errcode;
		std::string errmessage;
		int parse(bool sentByServerSide);
		MacGwConfig();
};

std::string macCommandlist();

#endif
