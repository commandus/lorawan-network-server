#include "lora-rejoin.h"

#include <sstream>

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
		ss << "\"netid\": \"" << NETID2string(data.request.reset.netid) << "\", "
			<< "\"deveui\": \"" << DEVEUI2string(data.request.reset.deveui) << "\", "
			<< "\"rjcount0\": \"" << data.request.reset.rjcount0 << "\"";
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
		ss << "\"netid\": \"" << NETID2string(r->request.reset.netid) << "\", "
			<< "\"deveui\": \"" << DEVEUI2string(r->request.reset.deveui) << "\", "
			<< "\"rjcount0\": \"" << r->request.reset.rjcount0 << "\"";
	}
	ss << "}";
	return ss.str();
}

LoraWANJoinAccept::LoraWANJoinAccept(
	const void *buffer,
	size_t size
) {
	if (size < sizeof(LORAWAN_JOIN_ACCEPT) - sizeof(CFLIST))
		return;
	LORAWAN_JOIN_ACCEPT *r = (LORAWAN_JOIN_ACCEPT*) buffer;
	if (size >= sizeof(LORAWAN_JOIN_ACCEPT)) {
		memmove(&data, buffer, sizeof(LORAWAN_JOIN_ACCEPT));
	} else {
		if (size >= sizeof(LORAWAN_JOIN_ACCEPT) - sizeof(CFLIST)) {
			memmove(&data, buffer, sizeof(LORAWAN_REJOIN_REQUEST_0_2) - sizeof(CFLIST));
			memset(&data.cflist, 0, sizeof(CFLIST));
		}
	}
}

std::string LoraWANJoinAccept::toJSONString() const
{
	std::stringstream ss;
	ss << "{\"joinnonce\": \"" << JOINNONCE2string(data.joinNonce) << "\", "
		<< "\"netid\": \"" << NETID2string(data.netid) << "\", "
		<< "\"devaddr\": \"" << DEVADDR2string(data.devaddr) << "\", "
		<< "\"optneg\": " << (int) (data.optneg) << ", "
		<< "\"rx1droffset\": " << (int) (data.rx1droffset) << ", "
		<< "\"rx2datarate\": " << (int) (data.rx2datarate) << ", "
		<< "\"rxdelay\": " << (int) data.rxdelay << "";
	if (FREQUENCY2int(data.cflist.frequency[0]) != 0) {
		ss << ", \"freqCh4\": " << FREQUENCY2int(data.cflist.frequency[0])
			<< ", \"freqCh5\": " << FREQUENCY2int(data.cflist.frequency[1])
			<< ", \"freqCh6\": " << FREQUENCY2int(data.cflist.frequency[2])
			<< ", \"freqCh7\": " << FREQUENCY2int(data.cflist.frequency[3])
			<< ", \"freqCh8\": " << FREQUENCY2int(data.cflist.frequency[4])
			<< ", \"cflisttype\": \"" << (int) (data.cflist.cflisttype);
	}
	ss << "}";
	return ss.str();
}

std::string LoraWANJoinAccept::toJSONString(
	const void *buffer,
	size_t size
) {
	LORAWAN_JOIN_ACCEPT *r = (LORAWAN_JOIN_ACCEPT*) buffer;
	if (size < sizeof(LORAWAN_JOIN_ACCEPT) - sizeof(CFLIST))
		return "";
	std::stringstream ss;
		ss << "{\"joinNonce\": \"" << JOINNONCE2string(r->joinNonce) << "\", "
		<< "\"homeNetID\": \"" << NETID2string(r->netid) << "\", "
		<< "\"devAddr\": \"" << DEVADDR2string(r->devaddr) << "\", "
		<< "\"optneg\": " << (int) (r->optneg) << ", "
		<< "\"rx1droffset\": " << (int) (r->rx1droffset) << ", "
		<< "\"rx2datarate\": " << (int) (r->rx2datarate) << ", "
		<< "\"rxdelay\": " << (int) r->rxdelay << "";
	if (size >= sizeof(LORAWAN_JOIN_ACCEPT)) {
		ss << ", \"freqCh4\": " << FREQUENCY2int(r->cflist.frequency[0])
			<< ", \"freqCh5\": " << FREQUENCY2int(r->cflist.frequency[1])
			<< ", \"freqCh6\": " << FREQUENCY2int(r->cflist.frequency[2])
			<< ", \"freqCh7\": " << FREQUENCY2int(r->cflist.frequency[3])
			<< ", \"freqCh8\": " << FREQUENCY2int(r->cflist.frequency[4])
			<< ", \"cflisttype\": \"" << (int) (r->cflist.cflisttype);
	}
	ss << "}";
	return ss.str();
}
