#ifndef SEMTECH_PULL_RESP_PACKET_H_
#define SEMTECH_PULL_RESP_PACKET_H_	1

#include "utillora.h"
#include "lorawan-mac.h"

#include <string>
#include <vector>

class SemtechPullRespPacket {
private:
    NetworkIdentity identity;
	rfmHeader header;
	std::vector<MacData> macdata;

    std::string toString();
public:	
	// prefix contains gateway identifier
	SEMTECH_DATA_PREFIX prefix;
	rfmMetaData metadata;
	std::string payload;
	// authentication keys
	
	SemtechPullRespPacket(
		const NetworkIdentity &identity,
		const std::vector<MacData> &macdata,
		const std::string &payload,
		rfmMetaData metadata
	);
};

#endif
