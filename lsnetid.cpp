/**
 * @brief LoRaWAN 1.0.x packet decoder utility
 * @file lsnetid.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <string>
#include <iostream>
#include <bitset>

#include "argtable3/argtable3.h"
#include "errlist.h"
#include "dev-addr.h"

const std::string programName = "lsnetid";

class LsNetIdConfiguration {
public:
    uint32_t value;         // or
    uint8_t netTypeId;
    uint32_t netId;
    int verbosity;			// verbosity level
    bool hasValue;
};

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
        LsNetIdConfiguration *config,
        int argc,
        char *argv[])
{
    // device path
    struct arg_str *a_value_hex = arg_str0("x", "payload", "<hex>", "payload bytes, in hex");
    struct arg_str *a_nettype_hex = arg_str0("x", "payload", "<hex>", "payload bytes, in hex");
    struct arg_str *a_netid_hex = arg_str0("x", "payload", "<hex>", "payload bytes, in hex");

    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_value_hex, a_nettype_hex, a_netid_hex,
        a_verbosity, a_help, a_end
    };

    int nerrors;

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }
    // Parse the command line as defined by argtable[]
    nerrors = arg_parse(argc, argv, argtable);

    if (!nerrors) {
        if (a_nettype_hex->count) {
            std::stringstream ss(*a_nettype_hex->sval);
            ss >> config->netTypeId;
        } else
            config->netTypeId = 0;
        if (a_netid_hex->count) {
            std::stringstream ss(*a_nettype_hex->sval);
            ss >> config->netId;
        } else
            config->netId = 0;
        if (a_value_hex->count) {
            std::stringstream ss(*a_value_hex->sval);
            ss >> config->netId;

        } else
            config->value = 0;
    }
    config->hasValue = a_value_hex->count == 0;
    if ((!config->hasValue) && (a_nettype_hex->count == 0 || a_netid_hex->count == 0 )) {
        std::cerr << ERR_NETID_OR_NETTYPE_MISSED << std::endl;
        nerrors++;
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

const char* TAB_DELIMITER = "\t";

void printNetId(
        const NetId &value,
        int verbosity
) {
    if (verbosity > 0) {
        // print header
        std::cout << "NetId" << TAB_DELIMITER
                << "Type" << TAB_DELIMITER
                << "Id" << TAB_DELIMITER
                << "NwkId" << TAB_DELIMITER
                << "Binary"
                << std::endl;
    }

    std::cout
            << value.toString() << TAB_DELIMITER
            << std::hex
            << (int) value.getType() << TAB_DELIMITER
            << value.getNetId() << TAB_DELIMITER
            << value.getNwkId() << TAB_DELIMITER
            << std::bitset<8>{value.netid[2]}.to_string() << " "
            << std::bitset<8>{value.netid[1]}.to_string() << " "
            << std::bitset<8>{value.netid[0]}.to_string() << std::endl;

}

int main(int argc, char **argv)
{
    LsNetIdConfiguration lsnetidConfig;
    if (parseCmd(&lsnetidConfig, argc, argv) != 0) {
        exit(ERR_CODE_COMMAND_LINE);
    }
    NetId netid;
    if (lsnetidConfig.hasValue)
        netid.set(lsnetidConfig.value);
    if (lsnetidConfig.netTypeId)
        netid.set(lsnetidConfig.netTypeId, lsnetidConfig.netId);
    printNetId(netid, lsnetidConfig.verbosity);
}
