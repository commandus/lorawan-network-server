/**
 * @brief Gateway JSON config file to c++ source convert utility
 * @file gateway-config2cpp.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 * Usage:
 *   ./gateway-config2cpp /home/andrei/git/rak_common_for_gateway/lora/rak2287/global_conf_usb/*.json > gateway_usb_conf.cpp
 * @file /home/andrei/git/rak_common_for_gateway/lora/rak2287/global_conf_usb/*.json
 */
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

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

/**
 * global_conf.as_915_921.json  global_conf.as_917_920.json  global_conf.au_915_928.json  global_conf.eu_433.json      global_conf.in_865_867.json  global_conf.ru_864_870.json
 * global_conf.as_915_928.json  global_conf.as_920_923.json  global_conf.cn_470_510.json  global_conf.eu_863_870.json  global_conf.kr_920_923.json  global_conf.us_902_928.json
 * -> as_915_921 ...
 */
static std::string fileName2VarName(std::string fileName) {
    size_t last = fileName.find_last_of('.');
    if (last == std::string::npos)
        return fileName;
    std::string r = fileName.substr(0, last);
    last = r.find_last_of('.');
    if (last == std::string::npos)
        return r;
    return r.substr(last + 1);
}

static std::string addPrefix()
{
    std::stringstream ss;
    ss << "#include <string>\n"
        << "#include <cstring>\n"
        << "#include \"gateway-settings.h\"\n\n";

    ss << "class MemGatewaySettingsStorage {\n"
          "public:\n"
          "    sx1261_config_t sx1261;\n"
          "    sx130x_config_t sx130x;\n"
          "    gateway_t gateway;\n"
          "    struct lgw_conf_debug_s debug;\n"
          "    std::string serverAddr;\n"
          "    std::string gpsTtyPath;\n"
          "    std::string name;\n"
          "};\n\n";

    ss << "typedef void (*setupMemGatewaySettingsStorageFunc)(MemGatewaySettingsStorage &value);\n\n";
    ss << "typedef struct {\n"
        << "    std::string name;\n"
        << "    setupMemGatewaySettingsStorageFunc setup;\n"
        << "} setupMemGatewaySettingsStorage;\n\n";

    ss << "class MemGatewaySettings : public GatewaySettings {\n"
          "public:\n"
          "    MemGatewaySettingsStorage storage;\n"
          "\n"
          "    sx1261_config_t *sx1261() override { return &storage.sx1261; };\n"
          "    sx130x_config_t *sx130x() override { return &storage.sx130x; };\n"
          "    gateway_t *gateway() override { return &storage.gateway; };\n"
          "    struct lgw_conf_debug_s *debug() override { return &storage.debug; };\n"
          "    std::string *serverAddress() override { return &storage.serverAddr; };\n"
          "    std::string *gpsTTYPath() override { return &storage.gpsTtyPath; };\n"
          "};\n\n";
    return ss.str();
}

static std::string addSuffix(
    const std::vector<std::string> &fileNames
) {
    std::stringstream ss;
    ss << "const setupMemGatewaySettingsStorage memSetupMemGatewaySettingsStorage[] = {\n";
    bool isFirst = true;
    for (std::vector<std::string>::const_iterator it(fileNames.begin()); it != fileNames.end(); it++) {
        std::string vn = fileName2VarName(*it);
        if (isFirst)
            isFirst = false;
        else
            ss << ",\n";

        std::string niceName(vn);
        std::replace(niceName.begin(), niceName.end(), '_', ' ');
        ss << "\t{\"" << niceName << "\", &setup_" << vn << "}";
    }
    ss << "\n};\n";
    return ss.str();
}

static std::string addFilePrefix(
    const GatewayConfigFileJson &value,
    const std::string &varName
)
{
    std::stringstream ss;
        ss << "void setup_" << varName << "(MemGatewaySettingsStorage &" << varName << ") {\n";
    return ss.str();
}

static std::string addFileSuffix(
        const GatewayConfigFileJson&value,
        const std::string &varName
)
{
    std::stringstream ss;
    ss << "};   // " << varName << "\n";
    return ss.str();
}

int main(int argc, char **argv)
{
    GatewayConfig2CppConfiguration config;
    int r = parseCmd(&config, argc, argv);
    if (r != 0)
        exit(ERR_CODE_COMMAND_LINE);
    std::cout << addPrefix() << "\n\n";
    for (std::vector<std::string>::const_iterator it(config.fileNames.begin()); it != config.fileNames.end(); it++) {
        GatewayConfigFileJson gwcfj;
        std::string s = file2string(it->c_str());
        gwcfj.parseString(s);
        if (config.verbosity > 0)
            std::cerr << gwcfj.toString();
        std::string vn = fileName2VarName(*it);

        std::cout << addFilePrefix(gwcfj, vn)
            << gwcfj.toCppString(vn) << "\n"
            << addFileSuffix(gwcfj, vn) << "\n";
    }
    std::cout << addSuffix(config.fileNames);
}
