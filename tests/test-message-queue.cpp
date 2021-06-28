
/**
 * Usage
 * ./test-packet-queue
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>

#include "errlist.h"
#include "receiver-queue-service-file-json.h"
#include "receiver-queue-service-dir-txt.h"
#include "utilstring.h"

void push100(
	int cnt,
	JsonFileReceiverQueueService &svc,
	const std::string &v
)
{
	timeval tm;
	DeviceId deviceid;
	for (int i = 0; i < cnt; i++) {
		gettimeofday(&tm, NULL);
		svc.push(deviceid, v, tm);
	}
}

void pop100(
	int cnt,
	JsonFileReceiverQueueService &svc,
	int dbid
)
{
	for (int i = 0; i < cnt; i++) {
		ReceiverQueueEntry e;
		svc.pop(dbid, e);
	}
	
}

void jsonFileReceiverQueueService()
{
	JsonFileReceiverQueueService svc;
	svc.init("test-message-queue.json", NULL);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

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
	std::string v = hex2string("0100213887c1601c000000004a0000000000000000000000");

	timeval tm;
	gettimeofday(&tm, NULL);

	std::cerr << "Set ids" << std::endl;

	std::cerr << "db #1 #2 #3" << std::endl;
	svc.setDbs(dbids3);

	DeviceId deviceid;

	svc.push(deviceid, v, tm);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

	std::cerr << "db #1 #2" << std::endl;
	svc.setDbs(dbids2);
	svc.push(deviceid, v, tm);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

	std::cerr << "db #1" << std::endl;
	svc.setDbs(dbids1);
	svc.push(deviceid, v, tm);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

	/*
	std::cerr << "Push-pop" << std::endl;
	for(int i = 0; i < 100; i++)
	{
		push100(1000, svc, v);
		pop100(1000, svc, 1);
	}
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;
	*/

	std::cerr << "Remove" << std::endl;
	svc.remove(1);
	svc.remove(0);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

	std::cerr << "List" << std::endl;
	std::vector<ReceiverQueueEntry> entries;
	svc.list(entries, 0, 100);
	for(std::vector<ReceiverQueueEntry>::const_iterator it(entries.begin()); it != entries.end(); it++) {
		std::cerr 
			<< it->toJsonString()
			<< std::endl;
	}
	std::cerr << std::endl;

	svc.flush();
}

void dirReceiverQueueService()
{
	DirTxtReceiverQueueService svc;
	DirTxtReceiverQueueServiceOptions options;
	options.format = DIRTXT_FORMAT_BASE64;
	svc.init("message-queue", (void *) &options);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

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
	std::string v = hex2string("0100213887c1601c000000004a0000000000000000000000");

	timeval tm;
	gettimeofday(&tm, NULL);

	std::cerr << "Set ids" << std::endl;

	std::cerr << "db #1 #2 #3" << std::endl;
	svc.setDbs(dbids3);
	DeviceId deviceid;
	svc.push(deviceid, v, tm);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

	std::cerr << "db #1 #2" << std::endl;
	svc.setDbs(dbids2);
	svc.push(deviceid, v, tm);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

	std::cerr << "db #1" << std::endl;
	svc.setDbs(dbids1);
	svc.push(deviceid, v, tm);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

	std::cerr << "Remove" << std::endl;
	svc.remove(1);
	svc.remove(0);
	std::cerr << "Messages: " << svc.count() << ", " << svc.toJsonString() << std::endl;

	svc.push(deviceid, v, tm);
	svc.push(deviceid, v, tm);
	std::cerr << "List" << std::endl;
	std::vector<ReceiverQueueEntry> entries;
	svc.list(entries, 0, 100);
	for(std::vector<ReceiverQueueEntry>::const_iterator it(entries.begin()); it != entries.end(); it++) {
		std::cerr 
			<< it->toJsonString()
			<< std::endl;
	}
	std::cerr << std::endl;

	svc.flush();
}

int main(int argc, char **argv) {
	// jsonFileReceiverQueueService();
	dirReceiverQueueService();
}
