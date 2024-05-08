#include "daemonize.h"
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <iostream>

#ifdef _MSC_VER

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib, "advapi32.lib")

SERVICE_STATUS        gServiceStatus = { 0 };
SERVICE_STATUS_HANDLE gStatusHandle = nullptr;
#define DEF_PID_PATH ""

#define	LOG(msg) {}

#else

#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>

#define	LOG(msg) { syslog (LOG_NOTICE, msg); }

#define DEF_PID_PATH "/var/run/"

#endif

static std::string serviceName;
static TDaemonRunner daemonRun;
static TDaemonRunner daemonStopRequest;
static TDaemonRunner daemonDone;

#define DEF_FD_LIMIT			(1024 * 10)
const char *DEVNULL = "/dev/null";

Daemonize::	Daemonize(
    const std::string &daemonName,
    const std::string &aWorkingDirectory,
    TDaemonRunner runner,					///< function to run as deamon
    TDaemonRunner stopRequest, 				///< function to stop
    TDaemonRunner done,						///< function to clean after runner exit
    const int aMaxFileDescriptors,  		///< 0- default 1024
    const std::string &aPidFileName,     	///< if empty, /var/run/program_name.pid is used
    const bool aCloseFileDescriptors
)
    : workingDirectory(aWorkingDirectory), maxFileDescriptors(aMaxFileDescriptors), closeFileDescriptors(aCloseFileDescriptors)
{
	serviceName = daemonName;
	if (aPidFileName.empty())
		pidFileName = DEF_PID_PATH + daemonName + ".pid";
	else
		pidFileName = aPidFileName;
	daemonRun = runner;
	daemonStopRequest = stopRequest;
	daemonDone = done;
	int r = init();
	if (r)
		std::cerr << "Error daemonize " << r << std::endl;
}

Daemonize::~Daemonize()
{
}

#ifdef _MSC_VER

void statusStartPending()
{
    gServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gServiceStatus.dwControlsAccepted = 0;
    gServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    gServiceStatus.dwWin32ExitCode = 0;
    gServiceStatus.dwServiceSpecificExitCode = 0;
    gServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(gStatusHandle, &gServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"ServiceMain: SetServiceStatus returned error"));
	}
}

void statusStarted()
{
    gServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    gServiceStatus.dwCurrentState = SERVICE_RUNNING;
    gServiceStatus.dwWin32ExitCode = 0;
    gServiceStatus.dwCheckPoint = 0;
	if (SetServiceStatus(gStatusHandle, &gServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"ServiceMain: SetServiceStatus returned error"));
	}
}

void statusStopPending()
{
    gServiceStatus.dwControlsAccepted = 0;
    gServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    gServiceStatus.dwWin32ExitCode = 0;
    gServiceStatus.dwCheckPoint = 4;

	if (SetServiceStatus(gStatusHandle, &gServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"ServiceCtrlHandler: SetServiceStatus returned error"));
	}
}

void statusStopped()
{
    gServiceStatus.dwControlsAccepted = 0;
    gServiceStatus.dwCurrentState = SERVICE_STOPPED;
    gServiceStatus.dwWin32ExitCode = 0;
    gServiceStatus.dwCheckPoint = 3;
	if (SetServiceStatus(gStatusHandle, &gServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"ServiceMain: SetServiceStatus returned error"));
	}
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode) {
	case SERVICE_CONTROL_STOP:
		if (gServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;
		statusStopPending();
		daemonStopRequest();
		daemonDone();
		statusStopped();
		break;
	default:
		break;
	}
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	// Register our service control handler with the SCM
	gStatusHandle = RegisterServiceCtrlHandlerA((LPCSTR) serviceName.c_str(), ServiceCtrlHandler);
	if (gStatusHandle == nullptr)
		goto EXIT;
	// Tell the service controller we are starting
	ZeroMemory(&gServiceStatus, sizeof(gServiceStatus));
	statusStartPending();
	// Tell the service controller we are started
	statusStarted();
	daemonRun();
	daemonDone();
	// Tell the service controller we are stopped
	statusStopped();
EXIT:
	return;
}

// See http://stackoverflow.com/questions/18557325/how-to-create-windows-service-in-c-c
int Daemonize::init()
{
	std::wstring sn(serviceName.begin(), serviceName.end());
	SERVICE_TABLE_ENTRY ServiceTable[] = {
		{(LPSTR) sn.c_str(), (LPSERVICE_MAIN_FUNCTION) ServiceMain},
		{nullptr, nullptr}
	};
	if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
		return GetLastError();
	return 0;
}

int Daemonize::setFdLimit(int value)
{
	return 0;
}

bool Daemonize::setPidFile()
{
	return true;
}

#else
//See http://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux
int Daemonize::init()
{
    pid_t pid;
    // Fork off the parent process
    pid = fork();

    // An error occurred
    if (pid < 0)
        exit(EXIT_FAILURE);

    // Success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // On success: The child process becomes session leader
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    // Catch, ignore and handle signals
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // Fork off for the second time
    pid = fork();

    // An error occurred
    if (pid < 0)
        exit(EXIT_FAILURE);

    // Success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // Set new file permissions
    umask(0);

    int x;

    // Change the working directory to the root directory
    // or another appropriated directory
    x = chdir(workingDirectory.c_str());

    // Close all open file descriptors
    if (closeFileDescriptors) {
        for (x = sysconf(_SC_OPEN_MAX); x>0; x--)	{
            if (x > 2)
                close(x);
        }
        // reopen stdin, stdout, stderr
        /*
        stdin = fopen(DEVNULL, "r");
        stdout = fopen(DEVNULL, "w+");
        stderr = fopen(DEVNULL, "w+");
        */
    }

    if (maxFileDescriptors > 0)
        setFdLimit(maxFileDescriptors);

    setPidFile();
    daemonRun();
    daemonDone();
    return 0;
}

/**
 * Set
 * @param value	file descriptors per process. Default 1024 on Linux
 * @return 0- success
 */
int Daemonize::setFdLimit(int value)
{
    struct rlimit lim;
    int status;

    // current limit
    lim.rlim_cur = value;
    // max limit
    lim.rlim_max = value;
    return setrlimit(RLIMIT_NOFILE, &lim);
}

bool Daemonize::setPidFile()
{
	FILE* f = fopen(pidFileName.c_str(), "w+");
	if (f) {
		fprintf(f, "%u", getpid());
		fclose(f);
		return true;
	}
	return false;
}

#endif
