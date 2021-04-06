#include <sys/types.h>
#ifdef _MSC_VER
#include "Userenv.h"
#else
#include <pwd.h>
#include <unistd.h>
#include <dlfcn.h>
#endif

#include "config-filename.h"

#ifdef _MSC_VER
std::string getDefaultConfigFileName(const std::string &filename)
{
	std::string r = filename;
	// Need a process with query permission set
	HANDLE hToken = 0;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		// Returns a path like C:/Documents and Settings/nibu if my user name is nibu
		char homedir[MAX_PATH];
		DWORD size = sizeof(homedir);
		if (GetUserProfileDirectoryA(hToken, homedir, &size) && (size > 0))
		{
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
std::string getDefaultConfigFileName(const std::string &filename)
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	std::string r(homedir);
	return r + "/" + filename;
}
#endif
