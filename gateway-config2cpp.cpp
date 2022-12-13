/**
 * @brief Gateway JSON config file to c++ source convert utility
 * @file gateway-config2cpp.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "argtable3/argtable3.h"
#include "utilstring.h"
#include "gateway-file-json.h"
#include "errlist.h"

const std::string programName = "gateway-config2cpp";

#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#define ENABLE_TERM_COLOR	1
#endif

class GatewayConfig2CppConfiguration {
public:
    std::vector<std::string> fileNames;
    bool printRange;
    int verbosity;			// verbosity level
};

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
        GatewayConfig2CppConfiguration *config,
        int argc,
        char *argv[])
{
    // device path
    struct arg_str *a_file_names = arg_strn(nullptr, nullptr, "<file>", 1, 100, "JSON file name");

    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_file_names, a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }
    // Parse the command line as defined by argtable[]
    int nerrors = arg_parse(argc, argv, argtable);

    config->verbosity = a_verbosity->count;
    for (size_t i = 0; i < a_file_names->count; i++) {
        config->fileNames.push_back(std::string(a_file_names->sval[i]));
    }
    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nerrors) {
        if (nerrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << "Usage: " << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr
            << "Convert gateway JSON config file to c++ source" << std::endl;
        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_COMMAND_LINE;
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}

const char* TAB_DELIMITER = "\t";

int main(int argc, char **argv)
{
    GatewayConfig2CppConfiguration config;
    int r = parseCmd(&config, argc, argv);
    if (r != 0)
        exit(ERR_CODE_COMMAND_LINE);
    for (std::vector<std::string>::const_iterator it(config.fileNames.begin()); it != config.fileNames.end(); it++) {
        GatewayConfigFileJson gwcfj;
        std::string s = file2string(it->c_str());
        gwcfj.parseString(s);
        std::cout << gwcfj.toCppString() << std::endl;
    }
}
