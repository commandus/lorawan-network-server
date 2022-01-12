/**
 * @brief LoRaWAN 1.0.x packet decoder utility
 * @file lsnetid.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <string>
#include <iostream>
#include <iomanip>
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
    struct arg_str *a_value_hex = arg_str0(NULL, NULL, "<NetId>", "NetId, 3 bytes, hex");
    struct arg_int *a_nettype = arg_int0("t", "type", "<0..7>", "Network type");
    struct arg_str *a_netid_hex = arg_str0("i", "id", "<hex>", "Network identifier, hex");

    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_value_hex, a_nettype, a_netid_hex,
        a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }
    // Parse the command line as defined by argtable[]
    int nerrors = arg_parse(argc, argv, argtable);

    if (!nerrors) {
        config->verbosity = a_verbosity->count;
        if (a_nettype->count) {
            config->netTypeId = *a_nettype->ival;
        } else
            config->netTypeId = 0;
        if (a_netid_hex->count) {
            std::stringstream ss(*a_netid_hex->sval);
            ss >> std::hex >> config->netId;
        } else
            config->netId = 0;
        if (a_value_hex->count) {
            std::stringstream ss(*a_value_hex->sval);
            ss >> std::hex >> config->value;
        } else
            config->value = 0;
    }
    config->hasValue = a_value_hex->count > 0;
    if ((!config->hasValue) && (a_nettype->count == 0 || a_netid_hex->count == 0 )) {
        std::cerr << ERR_NETID_OR_NETTYPE_MISSED << std::endl;
        nerrors++;
    }

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nerrors) {
        if (nerrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << "Usage: " << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr
            << "Print NetId by value, e.g." << std::endl
            << "  lsnetid C0004A" << std::endl
            << "or compose NetId from the network type and network identifier, e.g." << std::endl
            << "  lsnetid -t 7 -n 1 " << std::endl
            << "where " << std::endl;
        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_COMMAND_LINE;
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}

const char* TAB_DELIMITER = "\t";

static std::string getAddressBitsString(const DevAddr &value)
{
    std::stringstream ss;
    ss
        << std::bitset<8>{value.devaddr[3]}.to_string() << " "
        << std::bitset<8>{value.devaddr[2]}.to_string() << " "
        << std::bitset<8>{value.devaddr[1]}.to_string() << " "
        << std::bitset<8>{value.devaddr[0]}.to_string();
    return ss.str();
}

static std::string getAddressDetailsString(const DevAddr &value)
{
    std::stringstream ss;
    ss
        << std::hex
        << "type " << value.getNetIdType() << " "
        << "NwkId: " << std::setw(4) << value.getNwkId() << " "
        << "NetAddr: " << value.getNwkAddr();
    return ss.str();
}

static void printNetId(
        const NetId &value,
        int verbosity
) {
    DevAddr minAddr(value, false);
    DevAddr maxAddr(value, true);

    if (verbosity > 0) {
        // print header
        std::cout
            << "NetId" << TAB_DELIMITER
            << "Type" << TAB_DELIMITER
            << "Id" << TAB_DELIMITER
            << "NwkId" << TAB_DELIMITER
            << "DevAddr min" << TAB_DELIMITER
            << "DevAddr max" << TAB_DELIMITER
            << "NetId, binary"
            << std::endl;
    }

    std::cout
        << value.toString() << TAB_DELIMITER
        << std::hex
        << (int) value.getType() << TAB_DELIMITER
        << value.getNetId() << TAB_DELIMITER
        << value.getNwkId() << TAB_DELIMITER
        << minAddr.toString() << TAB_DELIMITER
        << maxAddr.toString() << TAB_DELIMITER
        << std::bitset<8>{value.netid[2]}.to_string() << " "
        << std::bitset<8>{value.netid[1]}.to_string() << " "
        << std::bitset<8>{value.netid[0]}.to_string() << std::endl;

    if (verbosity > 1) {
        std::cout
            << std::endl
            << "DevAddr:" << std::endl
            << "Min "
            << minAddr.toString() << " "
            << getAddressBitsString(minAddr) << " "
            << getAddressDetailsString(minAddr) << std::endl
            << "Max "
            << maxAddr.toString() << " "
            << getAddressBitsString(maxAddr)  << " "
            << getAddressDetailsString(maxAddr) << std::endl;
    }
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
