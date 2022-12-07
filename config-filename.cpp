#include <sys/types.h>
#ifdef _MSC_VER
#include "Userenv.h"
#else
#include <pwd.h>
#include <unistd.h>
#include <dlfcn.h>
#include <climits>

#endif

#include "config-filename.h"
#include "utilfile.h"

#ifdef _MSC_VER
std::string getDefaultConfigFileName(
    char *programPath,
    const std::string &filename
)
{
	std::string r = filename;
	// Need a process with query permission set
	HANDLE hToken = 0;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		// Returns a path like C:/Documents and Settings/nibu if my user name is nibu
		char homedir[MAX_PATH];
		DWORD size = sizeof(homedir);
		if (GetUserProfileDirectoryA(hToken, homedir, &size) && (size > 0)) {
			r = std::string(homedir, size - 1).append("\\").append(filename);
		}
		CloseHandle(hToken);
	}
	return r;
}
#else
/**
* https://stackoverflow.com/questions/2910377/get-home-directory-in-linux-c
*/
std::string getDefaultConfigFileName(
    char *programPath,
    const std::string &filename
)
{
    // try to get in the current directory
    if (util::fileExists(filename))
        return filename;

    // try to get in the /etc directory
    std::string r("/etc/" + filename);
    if (util::fileExists(r))
        return r;

    // try to get in the $HOME directory
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    r = std::string(homedir) + "/" + filename;
    if (util::fileExists(r))
        return r ;

    // try executable file folder
    if (programPath) {
        std::string p = programPath;
        size_t last_slash_idx = p.rfind('/');
        if (last_slash_idx == std::string::npos)
            last_slash_idx = p.rfind('\\');
        if (last_slash_idx != std::string::npos) {
            r = p.substr(0, last_slash_idx + 1) + filename;
            if (util::fileExists(r)) {
                return r;
            }
        }
    }
    return filename;
}

#endif
