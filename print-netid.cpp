/**
 * @brief LoRaWAN 1.0.x print NetId details utility
 * @file print-netid.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <bitset>

#include "argtable3/argtable3.h"
#include "errlist.h"
#include "dev-addr.h"

const std::string programName = "print-netid";

#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#define ENABLE_TERM_COLOR	1
#endif

class PrintNetIdConfiguration {
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
        PrintNetIdConfiguration *config,
        int argc,
        char *argv[])
{
    // device path
    struct arg_str *a_value_hex = arg_str0(NULL, NULL, "<NetId>", "NetId, 3 bytes, hex");
    struct arg_int *a_nettype = arg_int0("t", "type", "<0..7>", "Network type");
    struct arg_str *a_netid_hex = arg_str0("n", "net", "<id>", "Network identifier, hex");

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
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_NETID_OR_NETTYPE_MISSED;
    }

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nerrors) {
        if (nerrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << "Usage: " << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr
            << "Print NetId by value, e.g." << std::endl
            << "  print-snetid C0004A" << std::endl
            << "or compose NetId from the network type and network identifier, e.g." << std::endl
            << "  print-netid -t 7 -n 1 " << std::endl
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
#ifdef ENABLE_TERM_COLOR
        << "\033[0;32m"
        << std::bitset<8>{value.devaddr[3]}.to_string()
        << "\033[0m"
        << std::bitset<8>{value.devaddr[2]}.to_string()
        << "\033[0;32m"
        << std::bitset<8>{value.devaddr[1]}.to_string()
        << "\033[0m"
        << std::bitset<8>{value.devaddr[0]}.to_string();
#else
    << std::bitset<8>{value.devaddr[3]}.to_string() << " "
    << std::bitset<8>{value.devaddr[2]}.to_string() << " "
    << std::bitset<8>{value.devaddr[1]}.to_string() << " "
    << std::bitset<8>{value.devaddr[0]}.to_string();
#endif
    return ss.str();
}

static std::string getBitExplanationString(const DevAddr &value)
{
    std::stringstream ss;
    uint8_t typ = value.getNetIdType();

    uint8_t prefixLen = DevAddr::getTypePrefixBitsCount(typ);
    uint8_t nwkIdLen = DevAddr::getNwkIdBitsCount(typ);
    uint8_t nwkAddrLen = DevAddr::getNwkAddrBitsCount(typ);
    ss
        << std::setw(prefixLen) << std::setfill('T') << "T"
        << std::setw(nwkIdLen) << std::setfill('n') << "n"
        << std::setw(nwkAddrLen) << std::setfill('A') << "A";

    return ss.str();
}

static std::string getAddressDetailsString(const DevAddr &value)
{
    std::stringstream ss;
    ss
        << std::hex
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
            << "DevAddr max"
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
        << std::endl;

    if (verbosity > 1) {
        std::cout << std::endl
                  << "binary:" << std::endl
                  #ifdef ENABLE_TERM_COLOR
                  << std::bitset<8>{value.netid[2]}.to_string()
                  << "\033[0;32m"
                  << std::bitset<8>{value.netid[1]}.to_string()
                  << "\033[0m"
                  << std::bitset<8>{value.netid[0]}.to_string()
                  #else
                  << std::bitset<8>{value.netid[2]}.to_string() << " "
        << std::bitset<8>{value.netid[1]}.to_string() << " "
        << std::bitset<8>{value.netid[0]}.to_string()
                  #endif
                  << std::endl;
        // Highlight bits
        std::cout
                << std::setw(3) << std::setfill('T') << "T";
        if (value.getRFUBitsCount())
            std::cout
                    << std::setw(value.getRFUBitsCount()) << std::setfill(' ') << " ";
        std::cout
                << std::setw(value.getNetIdBitsCount()) << std::setfill('N') << "N"
                << std::endl;
    }

    if (verbosity > 1) {
        std::cout
            << std::endl
            << "DevAddr:" << std::endl
            << "Min "
            << getAddressBitsString(minAddr) << " "
            << getAddressDetailsString(minAddr) << std::endl
            << "    " << getBitExplanationString(minAddr)
            << std::endl
            << "Max "
            << getAddressBitsString(maxAddr)  << " "
            << getAddressDetailsString(maxAddr) << std::endl
            << "    " << getBitExplanationString(maxAddr)
            << std::endl;
    }
}

static void printClass
(
    const NetId &netid
) {
    DevAddr minAddr(netid, false);
    DevAddr maxAddr(netid, true);
    std::cout
            << netid.toString() << TAB_DELIMITER
            << std::hex
            << (int) netid.getType() << TAB_DELIMITER
            << netid.getNetId() << TAB_DELIMITER
            << netid.getNwkId() << TAB_DELIMITER
            << minAddr.toString() << TAB_DELIMITER
            << maxAddr.toString() << TAB_DELIMITER
            << std::endl;
}

static void printAllClasses() {
    NetId netid;
    // print header
    std::cout
        << "NetId" << TAB_DELIMITER
        << "Type" << TAB_DELIMITER
        << "Id" << TAB_DELIMITER
        << "NwkId" << TAB_DELIMITER
        << "DevAddr min" << TAB_DELIMITER
        << "DevAddr max"
        << std::endl;

    netid.set(0, 0);
    printClass(netid);
    netid.set(0, (1 << 6) - 1);
    printClass(netid);

    netid.set(1, 0);
    printClass(netid);
    netid.set(1, (1 << 6) - 1);
    printClass(netid);

    netid.set(2, 0);
    printClass(netid);
    netid.set(2, (1 << 9) - 1);
    printClass(netid);

    netid.set(3, 0);
    printClass(netid);
    netid.set(3, (1 << 21) - 1);
    printClass(netid);

    netid.set(4, 0);
    printClass(netid);
    netid.set(4, (1 << 21) - 1);
    printClass(netid);

    netid.set(5, 0);
    printClass(netid);
    netid.set(5, (1 << 21) - 1);
    printClass(netid);

    netid.set(6, 0);
    printClass(netid);
    netid.set(6, (1 << 21) - 1);
    printClass(netid);

    netid.set(7, 0);
    printClass(netid);
    netid.set(7, (1 << 21) - 1);
    printClass(netid);
}

int main(int argc, char **argv)
{
    PrintNetIdConfiguration printNetidConfig;
    int r = parseCmd(&printNetidConfig, argc, argv);
    if (r == ERR_CODE_NETID_OR_NETTYPE_MISSED) {
        // print all classes
        printAllClasses();
    }
    if (r != 0) {
        exit(ERR_CODE_COMMAND_LINE);
    }
    NetId netid;
    if (printNetidConfig.hasValue)
        netid.set(printNetidConfig.value);
    else {
        int r = netid.set(printNetidConfig.netTypeId, printNetidConfig.netId);
        if (r) {
            std::cerr << ERR_MESSAGE << r << ": " << strerror_lorawan_ns(r) << std::endl;
            exit(r);
        }
        std::cerr << netid.toString() << std::endl;
        std::cerr << (int) printNetidConfig.netTypeId << std::endl;
        std::cerr << printNetidConfig.netId << std::endl;
    }
    printNetId(netid, printNetidConfig.verbosity);
}
