#include "lora-rejoin.h"

#include <iostream>
#include <sstream>
#include "system/crypto/aes.h"
#include "system/crypto/cmac.h"

LoraWANRejoinRequest::LoraWANRejoinRequest(
	const void *buffer,
	size_t size
) {
	if (size < sizeof(LORAWAN_REJOIN_REQUEST_0_2))
		return;
	LORAWAN_REJOIN_REQUEST *r = (LORAWAN_REJOIN_REQUEST*) buffer;
	if (r->macheader.f.mtype != MTYPE_REJOIN_REQUEST)
		return;		
	if (r->rejointype == 1) {
		if (size < sizeof(LORAWAN_REJOIN_REQUEST_1))
			return;
		memmove(&data, buffer, sizeof(LORAWAN_REJOIN_REQUEST_1));
	} else {
		memmove(&data, buffer, sizeof(LORAWAN_REJOIN_REQUEST_0_2));
	}
}

std::string LoraWANRejoinRequest::toJSONString() const
{
	std::stringstream ss;
	ss << "{\"rejointype\": " << (int) (data.rejointype) << ", ";
	if (data.rejointype == 1) {
		ss << "\"joineui\": \"" << DEVEUI2string(data.request.restore.joineui) << "\", "
			<< "\"deveui\": \"" << DEVEUI2string(data.request.restore.deveui) << "\", "
			<< "\"rjcount1\": \"" << data.request.restore.rjcount1 << "\"";
	} else {
		ss << "\"netid\": \"" << std::hex << NETID2int(data.request.reset.netid) << "\", "
			<< "\"deveui\": \"" << DEVEUI2string(data.request.reset.deveui) << "\", "
			<< "\"rjcount0\": \"" << std::dec << data.request.reset.rjcount0 << "\"";
	}
	ss << "}";
	return ss.str();
}

std::string LoraWANRejoinRequest::toJSONString(
	const void *buffer,
	size_t size
) {
	LORAWAN_REJOIN_REQUEST *r = (LORAWAN_REJOIN_REQUEST*) buffer;
	if (size < sizeof(LORAWAN_REJOIN_REQUEST_0_2))
		return "";
	if (r->macheader.f.mtype != MTYPE_REJOIN_REQUEST)
		return "";		
	if (r->rejointype == 1) {
		if (size < sizeof(LORAWAN_REJOIN_REQUEST_1))
			return "";
	}

	std::stringstream ss;
	ss << "{\"rejointype\": " << (int) r->rejointype << ", ";
	if (r->rejointype == 1) {
		ss << "\"joineui\": \"" << DEVEUI2string(r->request.restore.joineui) << "\", "
			<< "\"deveui\": \"" << DEVEUI2string(r->request.restore.deveui) << "\", "
			<< "\"rjcount1\": \"" << r->request.restore.rjcount1 << "\"";
	} else {
		ss << "\"netid\": \"" << std::hex << NETID2int(r->request.reset.netid) << "\", "
			<< "\"deveui\": \"" << DEVEUI2string(r->request.reset.deveui) << "\", "
			<< "\"rjcount0\": \"" << std::dec << r->request.reset.rjcount0 << "\"";
	}
	ss << "}";
	return ss.str();
}

LoraWANJoinAccept::LoraWANJoinAccept(
	const void *buffer,
	size_t size
) {
	if (size < sizeof(LORAWAN_JOIN_ACCEPT_SHORT))
		return;
	hasCFList = size >= sizeof(LORAWAN_JOIN_ACCEPT_LONG);
	LORAWAN_JOIN_ACCEPT *r = (LORAWAN_JOIN_ACCEPT*) buffer;
	if (size >= sizeof(LORAWAN_JOIN_ACCEPT_LONG)) {
		memmove(&data, buffer, sizeof(LORAWAN_JOIN_ACCEPT_LONG));
	} else {
		if (size >= sizeof(LORAWAN_JOIN_ACCEPT_SHORT)) {
			memmove(&data, buffer, sizeof(LORAWAN_JOIN_ACCEPT_SHORT));
		}
	}
}

std::string LoraWANJoinAccept::toJSONString() const
{
	std::stringstream ss;
	ss << "{\"joinnonce\": " << JOINNONCE2int(data.s.header.joinNonce) << ", "
		<< "\"netid\": \"" << std::hex << NETID2int(data.s.header.netid) << "\", "
		<< "\"devaddr\": \"" << DEVADDR2string(data.s.header.devaddr) << "\", "
		<< "\"optneg\": " << std::dec << (int) (data.s.header.optneg) << ", "
		<< "\"rx1droffset\": " << (int) (data.s.header.rx1droffset) << ", "
		<< "\"rx2datarate\": " << (int) (data.s.header.rx2datarate) << ", "
		<< "\"rxdelay\": " << (int) data.s.header.rxdelay << "";
	if (hasCFList) {
		ss << ", \"freqCh4\": " << FREQUENCY2int(data.l.cflist.frequency[0])
			<< ", \"freqCh5\": " << FREQUENCY2int(data.l.cflist.frequency[1])
			<< ", \"freqCh6\": " << FREQUENCY2int(data.l.cflist.frequency[2])
			<< ", \"freqCh7\": " << FREQUENCY2int(data.l.cflist.frequency[3])
			<< ", \"freqCh8\": " << FREQUENCY2int(data.l.cflist.frequency[4])
			<< ", \"cflisttype\": \"" << (int) (data.l.cflist.cflisttype)
			<< ", \"mic\": \"" << std::hex << ntohl(data.l.mic) << "\"";
	} else {
		ss << ", \"mic\": \"" << std::hex << ntohl(data.s.mic) << "\"";
	}
	ss << "}";
	return ss.str();
}

std::string LoraWANJoinAccept::toJSONString(
	const void *buffer,
	size_t size
) {
	if (size < sizeof(LORAWAN_JOIN_ACCEPT_SHORT))
		return "";
	LORAWAN_JOIN_ACCEPT *r = (LORAWAN_JOIN_ACCEPT*) buffer;
	bool hasCFList = size >= sizeof(LORAWAN_JOIN_ACCEPT_LONG);
	std::stringstream ss;
		ss << "{\"joinnonce\": " << JOINNONCE2int(r->s.header.joinNonce) << ", "
		<< "\"netid\": \"" << std::hex << NETID2int(r->s.header.netid) << "\", "
		<< "\"devaddr\": \"" << DEVADDR2string(r->s.header.devaddr) << "\", "
		<< "\"optneg\": " << std::dec << (int) (r->s.header.optneg) << ", "
		<< "\"rx1droffset\": " << (int) (r->s.header.rx1droffset) << ", "
		<< "\"rx2datarate\": " << (int) (r->s.header.rx2datarate) << ", "
		<< "\"rxdelay\": " << (int) r->s.header.rxdelay;
	if (hasCFList) {
		ss << ", \"freqCh4\": " << FREQUENCY2int(r->l.cflist.frequency[0])
			<< ", \"freqCh5\": " << FREQUENCY2int(r->l.cflist.frequency[1])
			<< ", \"freqCh6\": " << FREQUENCY2int(r->l.cflist.frequency[2])
			<< ", \"freqCh7\": " << FREQUENCY2int(r->l.cflist.frequency[3])
			<< ", \"freqCh8\": " << FREQUENCY2int(r->l.cflist.frequency[4])
			<< ", \"cflisttype\": \"" << (int) (r->l.cflist.cflisttype)
			<< ", \"mic\": \"" << std::hex << ntohl(r->l.mic) << "\"";
	} else {
		ss << ", \"mic\": \"" << std::hex << ntohl(r->s.mic) << "\"";
	}
	ss << "}";
	return ss.str();
}

typedef PACK( struct {
	uint8_t joinType;					// 1
	DEVEUI eui;							// 8
	JOINNONCE devnonce;				    // 3
} ) JOINACCEPT4MIC_NEG1_HEADER;	// 12 bytes

typedef PACK( struct {
	MHDR macheader;
	JOINNONCE joinnonce;
	NETID netid;
	DEVADDR devAddr;
	uint8_t dlSettings;
	uint8_t rxDelay;
	CFLIST cfList;
} ) JOINACCEPT4MIC_NEG0;

typedef PACK( struct {
	JOINACCEPT4MIC_NEG1_HEADER n;
	JOINACCEPT4MIC_NEG0 v;
} ) JOINACCEPT4MIC_NEG1;

uint32_t calcJoinAcceptMIC(
	const DEVEUI &joinEUI,
	const JOINNONCE &devNonce,
	const KEY128 &key,
	const LORAWAN_JOIN_ACCEPT *data,
	bool hasCFList
) {
	uint32_t r;
	JOINACCEPT4MIC_NEG1 a;
	size_t sz = 0;
	if (data->s.header.optneg) {
		// JoinReqType | JoinEUI | DevNonce 12 bytes
		a.n.joinType = (uint8_t) JOINREQUEST;
		memmove(a.n.eui, joinEUI, sizeof(DEVEUI));
		memmove(a.n.devnonce, devNonce, sizeof(JOINNONCE));
		sz += sizeof(JOINACCEPT4MIC_NEG1_HEADER);
	}
	//  MHDR | JoinNonce | NetID | DevAddr | DLSettings | RxDelay: 13 bytes | CFList: 16 bytes
	a.v.macheader = data->s.header.macheader;
	sz += sizeof(LORAWAN_JOIN_ACCEPT_HEADER);
	if (hasCFList)
		sz += sizeof(CFLIST);
	std::cerr << "size of " << sz << std::endl;		
	memmove(a.v.joinnonce, data->s.header.joinNonce, sz);

	unsigned char blockB[16];
	memset(blockB, 0, sizeof(blockB));
	aes_context aesContext;
	memset(aesContext.ksch, '\0', 240);
	AES_CMAC_CTX aesCmacCtx;
    aes_set_key(key, sizeof(KEY128), &aesContext);
	AES_CMAC_Init(&aesCmacCtx);
	AES_CMAC_SetKey(&aesCmacCtx, key);
	AES_CMAC_Update(&aesCmacCtx, blockB, sizeof(blockB));
	void *p;
	if (data->s.header.optneg) {
		p = &a.n;
	} else {
		p = &a.v;
	}
	std::cerr << "size of " << sz << std::endl;

	AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) p, (uint32_t) sz);
	uint8_t mic[16];
	AES_CMAC_Final(mic, &aesCmacCtx);
    r = (uint32_t) ((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
	return r;
}

uint32_t LoraWANJoinAccept::mic(
	const DEVEUI &joinEUI,
	const JOINNONCE &devNonce,
	const KEY128 &key

) {
	return calcJoinAcceptMIC(
		joinEUI, devNonce, key, &data, hasCFList);
}