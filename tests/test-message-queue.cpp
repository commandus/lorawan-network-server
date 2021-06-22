
/**
 * Usage
 * ./test-packet-queue
 */

#include <iostream>
#include <iomanip>
#include <vector>

#include "errlist.h"
#include "receiver-queue-service-file-json.h"


void t1(
	JsonFileReceiverQueueService &svc
)
{
}

int main(int argc, char **argv) {
	JsonFileReceiverQueueService svc;
	svc.init("test-message-queue.json", NULL);

	std::vector<int> dbids;
	dbids.push_back(1);
	dbids.push_back(2);
	dbids.push_back(3);
	svc.setDbs(dbids);

	ReceiverQueueEntry e;
	std::string payload("asdf");
	timeval tm;
	gettimeofday(&tm, NULL);

	svc.push(payload, tm);
	svc.push(payload, tm);
	svc.push(payload, tm);
	svc.flush();

	std::cerr << svc.toJsonString() << std::endl;
	t1(svc);
}
