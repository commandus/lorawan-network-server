#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>

#include "pkt2/str-pkt2.h"
#include "receiver-queue-service-file-json.h"
#include "receiver-queue-processor.h"
#include "identity-service-file-json.h"
#include "errlist.h"
#include "utilstring.h"
#include "utillora.h"
#include "utildate.h"

int main(int argc, char** argv)
{
	// create queue
	JsonFileReceiverQueueService *receiverQueueService = new JsonFileReceiverQueueService();
	// create processor to serve queue
	RecieverQueueProcessor *recieverQueueProcessor = new RecieverQueueProcessor();

	void *pkt2env = initPkt2("proto", 0);
	if (!pkt2env) {
		std::cerr << ERR_LOAD_PROTO << std::endl;
		exit(ERR_CODE_LOAD_PROTO);
	}

	recieverQueueProcessor->setPkt2Env(pkt2env);
	ConfigDatabases configDatabases("dbs.js");
	if (configDatabases.dbs.size() == 0) {
		std::cerr << ERR_LOAD_DATABASE_CONFIG << std::endl;
		exit(ERR_CODE_LOAD_DATABASE_CONFIG);
	}
	DatabaseByConfig *dbByConfig = new DatabaseByConfig(&configDatabases);
	recieverQueueProcessor->setDatabaseByConfig(dbByConfig);	

	DeviceId deviceId;
	deviceId.setEUIString("3434383566378112");
	deviceId.setNwkSKeyString("313747123434383535003A0066378888");
	deviceId.setAppSKeyString("35003A003434383531374712656B7F47");
	deviceId.setName("SI-13-232");
	deviceId.setClass(CLASS_B);
	std::string packet = hex2string("0254f80000006cc3743eed467b227278706b223a5b7b22746d7374223a37303335373236302c226368616e223a352c2272666368223a312c2266726571223a3836392e3130303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a362e322c2272737369223a2d3130302c2273697a65223a33372c2264617461223a225144414452514741324155434e77696a662f7836714269344b3958354b326d2b6645794b4963784e537639464f654f686d513d3d227d5d7d");

	std::vector<SemtechUDPPacket> packets;
	// get packets
	SEMTECH_PREFIX_GW dataprefix;
	GatewayStat gatewayStat;
	JsonFileIdentityService identityService;
	identityService.init("identity.json", NULL);

	struct sockaddr_in6 clientAddress;
	int r = SemtechUDPPacket::parse((const struct sockaddr *) &clientAddress, dataprefix, gatewayStat, packets, packet.c_str(), packet.size(), &identityService);
	packets[0].devId = deviceId;
	std::string payload = packets[0].payload;

	timeval timeval;
	
	timeval.tv_usec = 0; 

	for (int i = 0; i < 1; i++) {
		timeval.tv_sec = time(NULL);
		receiverQueueService->push(packets[0], timeval);
	}

	std::vector<ReceiverQueueEntry> entries1;
	receiverQueueService->list(entries1, 0, receiverQueueService->count());
	for (std::vector<ReceiverQueueEntry>::const_iterator it(entries1.begin()); it != entries1.end(); it++ ) {
		std::cerr << it->toJsonString() << std::endl;
	}

	// run processor
	recieverQueueProcessor->start(receiverQueueService);

	sleep(1);
	
	std::vector<ReceiverQueueEntry> entries;
	receiverQueueService->list(entries, 0, receiverQueueService->count());
	for (std::vector<ReceiverQueueEntry>::const_iterator it(entries.begin()); it != entries.end(); it++ ) {
		std::cerr << it->toJsonString() << std::endl;
	}

	// stop processor
	recieverQueueProcessor->stop();
	delete dbByConfig;
	donePkt2(pkt2env);
	return 0;
}            
