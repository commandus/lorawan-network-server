#include "daemonize.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>

#ifdef _MSC_VER
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib, "advapi32.lib")

SERVICE_STATUS        g_ServiceStatus = {0}; 
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
#define DEF_PID_PATH ""

#else
#include <sys/resource.h>
#include <unistd.h>
#include <syslog.h>
#define DEF_PID_PATH "/var/run/"
#endif

static std::string serviceName;
static TDaemonRunner daemonRun;
static TDaemonRunner daemonStopRequest;
static TDaemonRunner daemonDone;

#define DEF_FD_LIMIT			1024*10

#ifdef _MSC_VER
#define	LOG(msg) {}
#else
#define	LOG(msg) { syslog (LOG_NOTICE, msg); }
#endif

Daemonize::Daemonize(
		const std::string &daemonName,
		const std::string &aworking_directory,
		TDaemonRunner runner,
		TDaemonRunner stopRequest,
		TDaemonRunner done,
		const int maxfile_descriptors,
		const std::string pid_file_name
)
	: working_directory(aworking_directory), maxFileDescriptors(maxfile_descriptors)
{
	serviceName = daemonName;
	if (pidFileName.empty())
		pidFileName = DEF_PID_PATH + daemonName + ".pid";
	else
		pidFileName = pid_file_name;
	daemonRun = runner;
	daemonStopRequest = stopRequest;
	daemonDone = done;
	int r = init();
	if (r)
	{
		std::cerr << "Error daemonize " << r << std::endl;
	}
}

Daemonize::~Daemonize()
{
}

#ifdef _MSC_VER

void statusStartPending()
{
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"ServiceMain: SetServiceStatus returned error"));
	}
}

void statusStarted()
{
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;
	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"ServiceMain: SetServiceStatus returned error"));
	}
}

void statusStopPending()
{
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 4;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"ServiceCtrlHandler: SetServiceStatus returned error"));
	}
}

void statusStopped()
{
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;
	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"ServiceMain: SetServiceStatus returned error"));
	}
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:
		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
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
	g_StatusHandle = RegisterServiceCtrlHandler((LPWSTR) serviceName.c_str(), ServiceCtrlHandler);
	if (g_StatusHandle == NULL)
		goto EXIT;
	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
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
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{(LPWSTR)sn.c_str(), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};
	if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
		return GetLastError ();
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
	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* On success: The child process becomes session leader */
	if (setsid() < 0)
		exit(EXIT_FAILURE);

	/* Catch, ignore and handle signals */
	//TODO: Implement a working signal handler */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* Set new file permissions */
	umask(0);

	int x;

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	x = chdir(working_directory.c_str());

	/* Close all open file descriptors */
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
	{
		close(x);
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
	if (f)
	{
		fprintf(f, "%u", getpid());
		fclose(f);
		return true;
	}
	return false;
}


#endif
