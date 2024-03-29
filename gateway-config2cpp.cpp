/**
 * @brief Gateway JSON config file to c++ source convert utility
 * @file gateway-config2cpp.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 * Usage:
 *   ./gateway-config2cpp -h /home/andrei/git/rak_common_for_gateway/lora/rak2287/global_conf_usb/*.json > gateway_usb_conf.cpp
 * @file /home/andrei/git/rak_common_for_gateway/lora/rak2287/global_conf_usb/
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
    bool header;
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
    struct arg_str *a_file_names = arg_strn(nullptr, nullptr, "<file>", 1, 100, "Gateway config JSON file name");
    struct arg_lit *a_header = arg_lit0("h", "header", "C++11 header style");
    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_file_names, a_header, a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }
    // Parse the command line as defined by argtable[]
    int nerrors = arg_parse(argc, argv, argtable);

    config->header = a_header->count;
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
        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        std::cerr
            << "Convert gateway JSON config file to c++ source e.g.\n"
            "\t./gateway-config2cpp -h ~/git/rak_common_for_gateway/lora/rak2287/global_conf_usb/*.json > gateway_usb_conf.h"
            << std::endl;

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
static std::string fileName2VarName(
    const std::string &fileName
) {
    size_t last = fileName.find_last_of('.');
    if (last == std::string::npos)
        return fileName;
    std::string r = fileName.substr(0, last);
    last = r.find_last_of('.');
    if (last == std::string::npos)
        return r;
    return r.substr(last + 1);
}

/**
 * as_915_921 -> AS915-921
 * @param fileName
 * @return
 */
static std::string varName2RegionalSettingsName(
    const std::string &varName
)
{
    std::stringstream r;
    size_t p = varName.find('_');
    if (p == std::string::npos)
        return varName;
    std::string s(varName.substr(0, p));
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
    r << s;
    size_t p2 = varName.find('_', p + 1);
    if (p2 == std::string::npos) {
        r << varName.substr(p + 1);
    } else {
        r << varName.substr(p + 1, p2 - p - 1)
            << "-" << varName.substr(p2 + 1);
    }
    return r.str();
}

static void addPrefixCPP(
    std::ostream &strm,
    const std::vector<std::string> &files
)
{
    strm
        << "/**\n"
        << " *\n"
        << " * Autogenerated file. Do not modify.\n"
        << " *\n"
        << " * Generated by:\n"
        << " *      ./gateway-config2cpp \n";
    for (auto it(files.begin()); it != files.end(); it++) {
        strm << *it << " ";
    }
    strm
        << " *\n"
        << " */\n"
        << "#include <string>\n"
        << "#include <cstring>\n"
        << "#include \"gateway-settings.h\"\n\n"

        << "class MemGatewaySettingsStorage {\n"
          "public:\n"
          "    sx1261_config_t sx1261;\n"
          "    sx130x_config_t sx130x;\n"
          "    gateway_t gateway;\n"
          "    struct lgw_conf_debug_s debug;\n"
          "    std::string serverAddr;\n"
          "    std::string gpsTtyPath;\n"
          "    const char *name;\n"
          "};\n\n"

        << "typedef void (*setupMemGatewaySettingsStorageFunc)(MemGatewaySettingsStorage &value);\n\n"
        << "typedef struct {\n"
        << "    std::string name;\n"
        << "    setupMemGatewaySettingsStorageFunc setup;\n"
        << "} setupMemGatewaySettingsStorage;\n\n"

        << "class MemGatewaySettings : public GatewaySettings {\n"
          "public:\n"
          "    MemGatewaySettingsStorage storage;\n"
          "\n"
          "    sx1261_config_t *sx1261() override { return &storage.sx1261; };\n"
          "    sx130x_config_t *sx130x() override { return &storage.sx130x; };\n"
          "    gateway_t *gateway() override { return &storage.gateway; };\n"
          "    struct lgw_conf_debug_s *debug() override { return &storage.debug; };\n"
          "    std::string *serverAddress() override { return &storage.serverAddr; };\n"
          "    std::string *gpsTTYPath() override { return &storage.gpsTtyPath; };\n"
          "};\n";
}

static void addPrefixHeader(
    std::ostream &strm,
    const std::vector<std::string> &files
)
{
    strm
        << "/**\n"
        << " *\n"
        << " * Autogenerated file. Do not modify.\n"
        << " *\n"
        << " * Generated by:\n"
        << " *      ./gateway-config2cpp -h ";
    for (auto it(files.begin()); it != files.end(); it++) {
        strm << *it << " ";
    }
    strm << "\n *\n */\n\n"
        "#include \"gateway-settings.h\"\n\n"
        "GatewaySettings lorawanGatewaySettings[] = {";
}

static void addSuffixCPP(
    std::ostream &strm,
    const std::vector<std::string> &fileNames
) {
    strm << "const setupMemGatewaySettingsStorage memSetupMemGatewaySettingsStorage[] = {\n";
    bool isFirst = true;
    for (std::vector<std::string>::const_iterator it(fileNames.begin()); it != fileNames.end(); it++) {
        std::string vn = fileName2VarName(*it);
        std::string rsn = varName2RegionalSettingsName(vn);
        if (isFirst)
            isFirst = false;
        else
            strm << ",\n";

        strm << "\t{\"" << rsn << "\", &setup_" << vn << "}";
    }
    strm << "\n};\n";
}

static void addSuffixHeader(
    std::ostream &strm,
    const std::vector<std::string> &fileNames
) {
    strm << "};\n";
}

static void addFilePrefixCPP(
    std::ostream &strm,
    const GatewayConfigFileJson &value,
    const std::string &varName
)
{
    strm << "void setup_" << varName << "(MemGatewaySettingsStorage &" << varName << ") {\n"
            << "\tmemset(&" << varName << ", 0, sizeof(MemGatewaySettingsStorage));\n";
}

static void addFilePrefixHeader(
    std::ostream &strm,
    const GatewayConfigFileJson &value,
    const std::string &varName
)
{
    strm << "\n";
}

static void addFileSuffixCPP(
    std::ostream &strm,
    const GatewayConfigFileJson&value,
    const std::string &varName
)
{
    strm << "};   // " << varName << "\n";
}

static void addFileSuffixHeader(
    std::ostream &strm,
    const GatewayConfigFileJson&value,
    const std::string &varName
)
{
}

int main(int argc, char **argv)
{
    GatewayConfig2CppConfiguration config;
    int r = parseCmd(&config, argc, argv);
    if (r != 0)
        exit(ERR_CODE_COMMAND_LINE);
    bool isFirstFile = true;
    if (config.header) {
        addPrefixHeader(std::cout, config.fileNames);
        std::cout << "\n";

        for (std::vector<std::string>::const_iterator it(config.fileNames.begin()); it != config.fileNames.end(); it++) {
            if (isFirstFile)
                isFirstFile = false;
            else
                std::cout << ",";
            GatewayConfigFileJson gwcfj;
            std::string s = file2string(it->c_str());
            gwcfj.parseString(s);
            if (config.verbosity > 0)
                std::cerr << gwcfj.toString();
            std::string vn = fileName2VarName(*it);

            addFilePrefixHeader(std::cout, gwcfj, vn);
            gwcfj.toHeader(std::cout, vn);
            addFileSuffixHeader(std::cout, gwcfj, vn);
        }
        addSuffixHeader(std::cout, config.fileNames);
    } else {
        addPrefixCPP(std::cout, config.fileNames);
        std::cout << "\n";
        for (std::vector<std::string>::const_iterator it(config.fileNames.begin()); it != config.fileNames.end(); it++) {
            GatewayConfigFileJson gwcfj;
            std::string s = file2string(it->c_str());
            gwcfj.parseString(s);
            if (config.verbosity > 0)
                std::cerr << gwcfj.toString();
            std::string vn = fileName2VarName(*it);

            addFilePrefixCPP(std::cout, gwcfj, vn);
            gwcfj.toCpp(std::cout, vn);
            std::cout << "\n";
            addFileSuffixCPP(std::cout, gwcfj, vn);
            std::cout << "\n";
        }
        addSuffixCPP(std::cout, config.fileNames);
    }
}
