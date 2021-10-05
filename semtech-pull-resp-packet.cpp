#include "semtech-pull-resp-packet.h"

#include "lora-encrypt.h"

SemtechPullRespPacket::SemtechPullRespPacket(
    const NetworkIdentity &aidentity,
    const std::vector<MacData> &aMacData,
    const std::string &apayload,
    rfmMetaData ametadata
)
    : identity(aidentity), macdata(aMacData), payload(apayload), metadata(ametadata)
{

}

std::string SemtechPullRespPacket::toString() {
    uint16_t token;
    // radio prefix
    SEMTECH_PREFIX_GW prefix = { 2, token, 2 };
    memmove(&prefix.mac, identity.deviceEUI, sizeof(DEVEUI));

	// direction of frame is down
	unsigned char direction = 0x01;

	// MAC header byte: message type, RFU, Major
    RFM_HEADER h;
    h.macheader.i = 0x60; // unconfirmed downlink
    memmove(&h.devaddr, identity.devaddr, sizeof(DEVADDR));
    // frame control
    h.fctrl.f.foptslen = 0;
	h.fctrl.f.fpending = 0;
	h.fctrl.f.ack = 0;
    h.fctrl.f.rfu = 0;
    h.fctrl.f.adr = 0;
	// frame counter 0..15
	h.fcnt = 0;	


	// build radio packet, unconfirmed data up macHeader = 0x40;
	// RFM header 8 bytes
    std::stringstream ss;
	ss << header.toBinary() << header.fport;

	// load data
	// encrypt data
    std::string p(payload);
	encryptPayload(p, header.header.fcnt, direction, header.header.devaddr, identity.appSKey);
	ss << p;

	std::string rs = ss.str();
	// calc MIC
	uint32_t mic = calculateMIC((const unsigned char*) rs.c_str(), rs.size(), header.header.fcnt, direction, header.header.devaddr, identity.nwkSKey);	// nwkSKey
	// load MIC in package
	// mic = ntoh4(mic);
	ss << std::string((char *) &mic, 4);
    std::string d(ss.str());

    std::stringstream ss2;
	ss2 << "{\"txpk\":{";
	ss2 << metadata.toJsonString(d);
	ss2 << "}}";
	return ss2.str();
}
