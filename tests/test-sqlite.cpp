#include <string>
#include <iostream>

#include <google/protobuf/message.h>

#include "pkt2/str-pkt2.h"

int main(int argc, char **argv) {
	std::string protoPath = "proto";
	void* env = initPkt2(protoPath, 0);
	if (!env) {
		std::cerr << "Init error" << std::endl;
		exit(1);
	}

	ConfigDatabases configDatabases("tests/dbs.js");
	std::cerr << configDatabases.toString() << std::endl;

	if (!configDatabases.dbs.size()) {
		std::cerr << "No database definitions in config file" << std::endl;
		exit(2);
	}
	std::string mt = "iridium.IEPacket";

	std::string createClause = createTableSQLClause(env, mt, OUTPUT_FORMAT_SQL, 0, 
		&configDatabases.dbs[0].tableAliases, &configDatabases.dbs[0].fieldAliases);
	std::cout << "create statement: " << createClause << std::endl;

	std::string hexData = "01004e01001c9a0ba5f633303032333430363032333533343000011900005ab8f59303000b003e68a68143d40000000502001e0810003e01b21200004e812b4e160000390000221400829486247a0d1c09";

	donePkt2(env);
}
