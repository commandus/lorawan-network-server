#include <string>
#include "udp-socket.h"

class UDPEmitter
{
private:
	std::string buffer;

public:
	UDPSocket mSocket;
	bool stopped;
	bool daemonize;
	int verbosity;
	std::string logfilename;
	std::ostream *logstream;
	struct sockaddr_in remotePeerAddress;

	UDPEmitter();
	~UDPEmitter();

	void setBufferSize(size_t value);

	std::string toString();

	void closeSocket();

	int openSocket(
		UDPSocket &retval,
		const char *address,
		const char *port);

	int listenSocket(
		UDPSocket &retval,
		const char *address,
		const char *port);

	bool setAddress(
		const std::string &address);

	int receive(
		struct sockaddr_in *remotePeerAddr,
		int max_wait_s);

	bool isPeerAddr(
		struct sockaddr_in *remotePeerAddr);

	int sendDown(
		size_t size);

	int sendUp(
		size_t size);

	void setLastRemoteAddress(
		struct sockaddr_in *value);
};
