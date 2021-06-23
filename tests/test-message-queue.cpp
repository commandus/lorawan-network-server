
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

	std::vector<int> dbids1;
	dbids1.push_back(1);

	std::vector<int> dbids2;
	dbids2.push_back(2);
	dbids2.push_back(3);
	
	std::vector<int> dbids3;
	dbids3.push_back(1);
	dbids3.push_back(2);
	dbids3.push_back(3);
	
	ReceiverQueueEntry e;
	std::string payload("asdf");
	timeval tm;
	gettimeofday(&tm, NULL);

	std::cerr << "db #1 #2 #3" << std::endl;
	svc.setDbs(dbids3);
	svc.push(payload, tm);
	std::cerr << svc.toJsonString() << std::endl;

	std::cerr << "db #1 #2" << std::endl;
	svc.setDbs(dbids2);
	svc.push(payload, tm);
	std::cerr << svc.toJsonString() << std::endl;

	std::cerr << "db #1" << std::endl;
	svc.setDbs(dbids1);
	svc.push(payload, tm);
	std::cerr << svc.toJsonString() << std::endl;

	svc.flush();

	t1(svc);
}
