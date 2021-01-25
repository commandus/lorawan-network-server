#ifndef DAEMONIZE_H
#define DAEMONIZE_H	1

#include <string>

typedef void(*TDaemonRunner)();

/**
 * 	run daemon
 * 	\see http://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux
 */
class Daemonize
{
private:
	std::string pidFileName;
	int init();
	bool setPidFile();
	const std::string &working_directory;
	int maxFileDescriptors;
public:
	Daemonize(
		const std::string &daemonName,
		const std::string &working_directory,
		TDaemonRunner runner,					///< function to run as deamon
		TDaemonRunner stopRequest, 				///< function to stop
		TDaemonRunner done,						///< function to clean after runner exit
		const int maxfile_descriptors = 0,		///< 0- default 1024
		const std::string pid_file_name = ""	///< if empty, /var/run/program_name.pid is used

	);
	~Daemonize();
	static int setFdLimit(int value);
};

#endif
