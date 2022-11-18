#include "payload-insert-plugin.h"

#include "utilfile.h"

#define FUNC_NAME   "payload2InsertClauses"
#ifdef _MSC_VER
#else
#include <dlfcn.h>
#include <algorithm>

#endif

PayloadInsertPlugins::PayloadInsertPlugins()
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

int PayloadInsertPlugins::callChain(
    std::vector<std::string> &retClauses,
    void *env,
    const std::string &message,
    int inputFormat,
    int sqlDialect, ///< SQL_POSTGRESQL = 0 SQL_MYSQL = 1 SQL_FIREBIRD = 2 SQL_SQLITE = 3
    const std::string &data,
    const std::map<std::string, std::string> *properties,
    const std::string &nullValueString
)
{
    for (std::vector<payload2InsertClausesFunc>::const_iterator it(funcs.begin()); it != funcs.end(); it++) {
        int r = (*it)(retClauses, env, message, inputFormat, sqlDialect, data, properties, nullValueString);
        if (r >= 0)
            return r;
    }
    return -1;
}

// load plugin by file name
int PayloadInsertPlugins::push(
    const std::string &file
)
{
    void *handle;
#ifdef _MSC_VER
#else
    handle = dlopen(file.c_str(), RTLD_LAZY);
    if (handle) {
        handles.push_back(handle);
        payload2InsertClausesFunc f = (payload2InsertClausesFunc) dlsym(handle, FUNC_NAME);
        if (f) {
            funcs.push_back(f);
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
    size_t count = 0;
    for (std::vector<std::string>::const_iterator it(files.begin()); it != files.end(); it++) {
        if (push(*it) == 0)
            count++;

    }
    return count;   // successfully loaded .so files
}

void PayloadInsertPlugins::unload() {
    for (std::vector<void *>::iterator it(handles.begin()); it != handles.end(); it++) {
#ifdef _MSC_VER
#else
        dlclose(*it);
#endif
    }
    handles.clear();
}
