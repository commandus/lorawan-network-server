#include "packet-listener.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <syslog.h>
#include <csignal>
#include <cerrno>
#include <sys/select.h>

#include "get_rss/get_rss.h"
#include "utilstring.h"
#include "errlist.h"

#define DEF_BUFFER_SIZE     4096

PacketListener::PacketListener() :
    sysSignalPtr(nullptr), verbosity(0), stopped(false), onLog(nullptr), onGatewayStatDump(nullptr), gwStatEnv(nullptr),
    onDeviceStatDump(nullptr), deviceStatEnv(nullptr),
    handler(nullptr), identityService(nullptr), gatewayList(nullptr), deviceHistoryService(nullptr)
{
	setBufferSize(DEF_BUFFER_SIZE);
}

PacketListener::~PacketListener() {
	clear();
}

void PacketListener::setLogger(
	int averbosity,
	std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message
)> value) 
{
	verbosity = averbosity;
	onLog = value;
}

void PacketListener::setGatewayStatDumper(
	void *gwStatEnvVal,
	std::function<void(
		void *env,
		GatewayStat *gwStat
)> value)
{
	gwStatEnv = gwStatEnvVal;
	onGatewayStatDump = value;
}

void PacketListener::setDeviceStatDumper(
	void *deviceStatEnvVal,
	std::function<void(
		void *env,
		const SemtechUDPPacket &packet
)> value)
{
	deviceStatEnv = deviceStatEnvVal;
	onDeviceStatDump = value;
}

void PacketListener::setHandler(
	LoraPacketHandler *value
) {
	handler = value;
}

void PacketListener::setGatewayList(
	GatewayList *value
)
{
	gatewayList = value;
}

void PacketListener::setIdentityService
(
	IdentityService* value
)
{
	identityService = value;
}

void PacketListener::setDeviceHistoryService
(
        DeviceHistoryService *value
)
{
    deviceHistoryService = value;
}

/**
 * Set system signal last value pointer
 */
void PacketListener::setSysSignalPtr
(
	int *value
)
{
	sysSignalPtr = value;
}

void PacketListener::clear() {
}

std::string PacketListener::toString() const {
    return "";
}

int PacketListener::add(
    const std::vector<std::string> &values,
    int hint
)
{
    int r = 0;
    for (std::vector<std::string>::const_iterator it(values.begin()); it != values.end(); it++) {
        if (!add(*it, hint)) {
            std::stringstream ss;
            ss << ERR_MESSAGE << ERR_CODE_SOCKET_BIND << ": " <<  ERR_SOCKET_BIND << *it << std::endl;
            onLog(this, LOG_ERR, LOG_MAIN_FUNC, ERR_CODE_SOCKET_BIND, ss.str());
            exit(ERR_CODE_SOCKET_BIND);
        }
        r++;
    }
    return r;
}

void PacketListener::setBufferSize(size_t value) {
}

int PacketListener::parseBuffer
(
	const std::string &buffer,
	size_t bytesReceived,
	int socket,
	const struct timeval &receivedTime,
	const struct sockaddr_in6 &gwAddress
) {
	std::vector<SemtechUDPPacket> packets;
	// get packets
	SEMTECH_PREFIX_GW dataPrefix;
	GatewayStat gatewayStat;
	int pr = LORA_OK;
	if (bytesReceived < sizeof(SEMTECH_PREFIX)) 
		pr = ERR_CODE_PACKET_TOO_SHORT;
	else {
		SEMTECH_PREFIX *prefix = (SEMTECH_PREFIX *) buffer.c_str();
		switch (prefix->tag) {
			case SEMTECH_GW_PUSH_DATA:
				pr = SemtechUDPPacket::parse((const struct sockaddr *) &gwAddress, dataPrefix,
                        gatewayStat, packets, buffer.c_str(), bytesReceived, identityService);
                if (handler) {
                    if (pr == LORA_OK) {
                        // send ACK immediately
                        handler->ack(socket, (const sockaddr_in *) &gwAddress, dataPrefix);
                    }
                    if (pr == ERR_CODE_IS_JOIN) {
                        // send ACK immediately too
                        handler->ack(socket, (const sockaddr_in *) &gwAddress, dataPrefix);
                        if (packets.size() > 0) {
                            // enqueue JOIN request packet
                            handler->join(receivedTime, socket, (const sockaddr_in *) &gwAddress, packets[0]);
                        }
                    }
                }
				break;
			case SEMTECH_GW_PULL_DATA:	// PULL_DATA
				gatewayStat.errcode = ERR_CODE_NO_GATEWAY_STAT;
				pr = SemtechUDPPacket::parsePrefixGw(dataPrefix, buffer.c_str(), bytesReceived);
				if (pr != LORA_OK)
					break;
				// check is gateway in service
				{
					std::stringstream sse;
					sse << strerror_lorawan_ns(pr)
                        << " " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
                        << ", token: " << std::hex << dataPrefix.token;
					onLog(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, sse.str());
					// send PULL ACK immediately
					if (handler) {
						handler->ack(socket, (const sockaddr_in *) &gwAddress, dataPrefix);
					}
					if (gatewayList) {
						if (!gatewayList->setSocketAddress(dataPrefix.mac, socket, (const struct sockaddr_in *) &gwAddress)) {
							std::stringstream ss;
							ss << ERR_MESSAGE << ERR_CODE_INVALID_GATEWAY_ID << ": "
								<< ERR_INVALID_GATEWAY_ID
								<< " from " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
								<< " gateway: " << DEVEUI2string(dataPrefix.mac);
								;
							onLog(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_SEND_ACK, ss.str());
							break;
						}
					}
				}
				break;
			case SEMTECH_GW_TX_ACK:	// TX_ACK
				//
				gatewayStat.errcode = ERR_CODE_NO_GATEWAY_STAT;
				{
					pr = SemtechUDPPacket::parsePrefixGw(dataPrefix, buffer.c_str(), bytesReceived);
					if (pr != LORA_OK)
						break;
					ERR_CODE_TX r = extractTXAckCode(buffer.c_str(), bytesReceived);
					std::stringstream ss;
                    if (r) {
                        ss << ERR_MESSAGE << " " << r << ": ";
                        // TODO re-send in second window or later if device is class C
                    }
					ss << "TX ACK " << getTXAckCodeName(r)
						<< " from " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
						<< " gateway: " << DEVEUI2string(dataPrefix.mac);
					onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
					pr = LORA_OK;
				}
				break;
			default:
				pr = ERR_CODE_INVALID_PACKET;
				break;
		}
	}

    if (onDeviceStatDump) {
        for (std::vector<SemtechUDPPacket>::iterator itp(packets.begin()); itp != packets.end(); itp++) {
            onDeviceStatDump(deviceStatEnv, *itp);
        }
    }
	switch (pr) {
        case ERR_CODE_IS_JOIN:
            break;
		case ERR_CODE_PACKET_TOO_SHORT:
		case ERR_CODE_INVALID_PROTOCOL_VERSION:
		case ERR_CODE_NO_GATEWAY_STAT:
		case ERR_CODE_INVALID_STAT:
		case ERR_CODE_INVALID_PACKET:
		case ERR_CODE_INVALID_JSON:
		// ignore invalid mic, inconsistent packet removed, json can contain other valid packets
		// case ERR_CODE_INVALID_MIC: 
			{
				std::stringstream sse;
				sse << strerror_lorawan_ns(pr)
					<< " " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
					<< " (" << bytesReceived
					<< " bytes): " << hexString(buffer.c_str(), bytesReceived);
				onLog(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, sse.str());
			}
			break;
		case ERR_CODE_PULLOUT:
			{
			}
			break;
		default: // including ERR_CODE_INVALID_PACKET, it will contain some valid packets in the JSON, continue
			// process data packets if exists
            for (std::vector<SemtechUDPPacket>::iterator itp(packets.begin()); itp != packets.end(); itp++) {
				if (itp->errcode) {
					std::string v = std::string(buffer.c_str(), bytesReceived);
					std::stringstream ss;
					ss << ERR_MESSAGE << ERR_CODE_INVALID_PACKET << " "
						<< ": " << ERR_INVALID_PACKET
                        << ", gateway address: " << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
                        << ", packet: " << hexString(v);
					onLog(this, LOG_ERR, LOG_UDP_LISTENER, ERR_CODE_INVALID_PACKET, ss.str());
					continue;
				} else {
					if (onLog) {
						std::stringstream ss;
						ss << MSG_RXPK
							<< itp->toDebugString();
						onLog(this, LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
					}

					if (handler) {
						handler->put(receivedTime, *itp);
					} else {
						if (onLog) {
							std::stringstream ss;
							ss << MSG_READ_BYTES
								<< UDPSocket::addrString((const struct sockaddr *) &gwAddress) << ": "
								<< itp->toString();
							onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
						}
					}

				}
			}
			// reflect stat
			if (gatewayStat.errcode == 0) {
				if (gatewayList)
					gatewayList->copyId(gatewayStat, (const sockaddr *) &gwAddress);
				std::stringstream ss;
				ss << MSG_GATEWAY_STAT
					<< gatewayStat.toString() << ". Server memory " << getCurrentRSS()/ 1024 << "K";
				onLog(this, LOG_INFO, LOG_UDP_LISTENER, 0, ss.str());
				if (onGatewayStatDump)
					onGatewayStatDump(gwStatEnv, &gatewayStat);
			}
			break;
	}
	return pr;
}
