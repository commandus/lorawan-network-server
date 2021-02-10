#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H 1

#include <string>
#include <vector>
#include <functional>
#include "udp-socket.h"
#include "utillora.h"

class UDPListener
{
private:
	std::string buffer;
	int largestSocket();
public:
	std::vector<UDPSocket> sockets;
	bool stopped;
	struct sockaddr_in6 remotePeerAddress;

	std::function<void(
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
	)> onLog;

	std::function<void(
		semtechUDPPacket &packet
	)> onPacket;

	UDPListener();
	~UDPListener();

	void setBufferSize(size_t value);

	std::string toString();

	bool add(
		const std::string &address,
		MODE_FAMILY familyHint
	);
	void clear();

	int listen();

	int peerAddrIndex(
		struct sockaddr *remotePeerAddr);

	void setLastRemoteAddress(
		struct sockaddr *value);

	void clearLogger();
	void setLogger(
		std::function<void(
			int level,
			int modulecode,
			int errorcode,
			const std::string &message
	)> onLog);
	void setHandler(
		std::function<void(
			semtechUDPPacket &value
	)> onHandler);
	
};

#endif
