#include "payload-insert-plugin.h"
#include <sstream>
#include <algorithm>
#include "utilfile.h"
#include "log-intf.h"

#define FUNC_NAME_INIT              "pluginInit"
#define FUNC_NAME_DONE              "pluginDone"
#define FUNC_NAME_PREPARE           "payloadPrepare"
#define FUNC_NAME_INSERT            "payload2InsertClauses"
#define FUNC_NAME_AFTER_INSERT      "afterInsert"
#define FUNC_NAME_CREATE            "payloadCreate"

#ifdef _MSC_VER
#else
#include <dlfcn.h>
#include <algorithm>

#endif

#define OUTPUT_FORMAT_NAMES_SIZE 14

const char *OUTPUT_FORMAT_NAMES[OUTPUT_FORMAT_NAMES_SIZE] = {
    "json", "csv", "tab", "sql", "sql2", "pbtext", "dbg", "hex", "bin",
    "", "", // reserved
    "csv_header", "tab_header", "insert"
};

/**
 * Get output format name
 * @param index output format number 0- json, 1- csv, 2- tab, 3- sql, 4- sql2, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv_header, 12- tab_header 13- insert
 * @return output format name, "" if invalid
 */
std::string getOutputFormatName(
    int index
)
{
    if (index < 0 || index > 13)
        return "";
    return OUTPUT_FORMAT_NAMES[index];
}

/**
 * Get output format index
 * @param name output format number 0- json, 1- csv, 2- tab, 3- sql, 4- sql2, 5- pbtext, 6- dbg, 7- hex, 8- bin, 11- csv_header, 12- tab_header 13- insert
 * @return -1- inavalid name, 0..13:  output format index
 */
int getOutputFormatNumber(
    const std::string &name
)
{
    for (int i = 0; i < OUTPUT_FORMAT_NAMES_SIZE; i++) {
        if (name == std::string(OUTPUT_FORMAT_NAMES[i])) {
            return i;
        }
    }
    return -1;
}

PayloadInsertPlugins::PayloadInsertPlugins()
    : dbByConfig(nullptr)
{

}

// load plugins
PayloadInsertPlugins::PayloadInsertPlugins(
    const std::string &pluginDirectory
)
{
	load(pluginDirectory);
}

// unload plugins
PayloadInsertPlugins::~PayloadInsertPlugins()
{
	unload();
}

int PayloadInsertPlugins::init(
    const std::map<std::string, std::vector <std::string> > &params,
    LogIntf *log,
    int rfu
)
{
    for (std::vector<Payload2InsertPluginInitFuncs>::iterator it(funcs.begin()); it != funcs.end(); it++) {
        if (it->init != nullptr)
            it->env = it->init(dbByConfig, params, log, rfu);
        else
            it->env = nullptr;
    }
    return 0;
}

void PayloadInsertPlugins::done()
{
    for (std::vector<Payload2InsertPluginInitFuncs>::const_iterator it(funcs.begin()); it != funcs.end(); it++) {
        if (it->done != nullptr)
            it->done(it->env);
    }
}

int PayloadInsertPlugins::insert(
    std::vector<std::string> &retClauses,
    const std::string &message,
    int inputFormat,
    int outputFormat,
    int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::string &data,
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties,
    const std::string &nullValueString
)
{
    for (std::vector<Payload2InsertPluginInitFuncs>::const_iterator it(funcs.begin()); it != funcs.end(); it++) {
        int r = it->insert(retClauses, it->env, message, inputFormat, outputFormat, sqlDialect, data,
            tableAliases, fieldAliases, properties, nullValueString);
        if (r >= 0)
            return r;
    }
    return -1;
}

void PayloadInsertPlugins::afterInsert()
{
    for (std::vector<Payload2InsertPluginInitFuncs>::const_iterator it(funcs.begin()); it != funcs.end(); it++) {
        if (it->afterInsert)
            it->afterInsert(it->env);
    }
}

void PayloadInsertPlugins::prepare(
    uint32_t addr,
    const std::string &payload
)
{
    for (std::vector<Payload2InsertPluginInitFuncs>::const_iterator it(funcs.begin()); it != funcs.end(); it++) {
        if (it->prepare != nullptr)
            it->prepare(it->env, addr, payload);
    }
}

std::string PayloadInsertPlugins::create(
    const std::string &message,
    int outputFormat,
    int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::map<std::string, std::string> *tableAliases,
    const std::map<std::string, std::string> *fieldAliases,
    const std::map<std::string, std::string> *properties
) {
    std::stringstream ss;
    for (std::vector<Payload2InsertPluginInitFuncs>::const_iterator it(funcs.begin()); it != funcs.end(); it++) {
        if (it->create != nullptr) {
            ss << it->create(it->env, message, outputFormat, sqlDialect, tableAliases, fieldAliases, properties)
               << std::endl;
        }
    }
    return ss.str();
}

// load plugin by file name
int PayloadInsertPlugins::push(
    const std::string &file
)
{
    HINSTANCE handle;
#ifdef _MSC_VER
    handle = LoadLibraryA(file.c_str());
    if (handle) {
        handles.push_back(handle);

        Payload2InsertPluginInitFuncs fs;
        fs.init = (pluginInitFunc) GetProcAddress(handle, FUNC_NAME_INIT);
        fs.done = (pluginDoneFunc) GetProcAddress(handle, FUNC_NAME_DONE);
        fs.prepare = (payloadPrepareFunc) GetProcAddress(handle, FUNC_NAME_PREPARE);
        fs.insert = (payload2InsertClausesFunc) GetProcAddress(handle, FUNC_NAME_INSERT);
        fs.afterInsert = (payloadAfterInsertFunc) GetProcAddress(handle, FUNC_NAME_AFTER_INSERT);
        fs.create = (payloadCreateFunc) GetProcAddress(handle, FUNC_NAME_CREATE);
        if (fs.insert) {
            funcs.push_back(fs);
            return 0;
        }
    }
#else
    handle = dlopen(file.c_str(), RTLD_LAZY);
    if (handle) {
        handles.push_back(handle);
        Payload2InsertPluginInitFuncs fs;
        fs.init = (pluginInitFunc) dlsym(handle, FUNC_NAME_INIT);
        fs.done = (pluginDoneFunc) dlsym(handle, FUNC_NAME_DONE);
        fs.prepare = (payloadPrepareFunc) dlsym(handle, FUNC_NAME_PREPARE);
        fs.insert = (payload2InsertClausesFunc) dlsym(handle, FUNC_NAME_INSERT);
        fs.afterInsert = (payloadAfterInsertFunc) dlsym(handle, FUNC_NAME_AFTER_INSERT);
        fs.create = (payloadCreateFunc) dlsym(handle, FUNC_NAME_CREATE);
        if (fs.insert) {
            funcs.push_back(fs);
            return 0;
        }
    }
#endif
    return -1;
}

// load plugins
int PayloadInsertPlugins::load(
	const std::string &pluginDirectory,
    const std::string &suffix
)
{
    std::vector<std::string> files;
    // Load .so files. flags: 0- as is file name, 1- full path
    util::filesInPath(pluginDirectory, suffix, 1, &files);
    // sort alphabetically by file name
    std::sort(files.begin(), files.end());
    int count = 0;
    for (std::vector<std::string>::const_iterator it(files.begin()); it != files.end(); it++) {
        if (push(*it) == 0)
            count++;

    }
    return count;   // successfully loaded .so files
}

void PayloadInsertPlugins::unload() {
    for (std::vector<HINSTANCE>::iterator it(handles.begin()); it != handles.end(); it++) {
#ifdef _MSC_VER
        FreeLibrary(*it);
#else
        dlclose(*it);
#endif
    }
    handles.clear();
}
