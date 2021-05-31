#include <string>
#include <iostream>

#include <google/protobuf/message.h>

#include "pkt2/str-pkt2.h"

#include "db-sqlite.h"

int main(int argc, char **argv) {
	std::string protoPath = "proto";
	void* env = initPkt2(protoPath, 0);
	if (!env) {
		std::cerr << "Init error" << std::endl;
		exit(1);
	}

	ConfigDatabases configDatabases("tests/dbs.js");
	std::cerr << configDatabases.toString() << std::endl;

	ConfigDatabase* config = configDatabases.findByName("sqlite");

	if (!config) {
		std::cerr << "No database definition 'sqlite' in config file" << std::endl;
		exit(2);
	}
	std::string mt = "iridium.IEPacket";

	std::string createClause = createTableSQLClause(env, mt, OUTPUT_FORMAT_SQL, 0, 
		&config->tableAliases, &config->fieldAliases);
	std::cout << "create statement: " << createClause << std::endl;

	std::string hexData = "01004e01001c9a0ba5f633303032333430363032333533343000011900005ab8f59303000b003e68a68143d40000000502001e0810003e01b21200004e812b4e160000390000221400829486247a0d1c09";

	std::string insertClause = parsePacket(env, INPUT_FORMAT_HEX, OUTPUT_FORMAT_SQL, 0, hexData, mt,
		&config->tableAliases, &config->fieldAliases);
	std::cout << "insert statement: " << insertClause << std::endl;

	DatabaseSQLite db;
	int r = db.open(config->connectionString, config->login, config->password);
	if (r) {
		std::cerr << "Error open database " << r << ": " << db.errmsg << std::endl;
		exit(r);
	}

	r = db.exec(createClause);
	if (r) {
		std::cerr << "Error CREATE exec SQL " << r << ": " << db.errmsg << std::endl;
	}

	r = db.exec(insertClause);
	if (r) {
		std::cerr << "Error INSERT exec SQL " << r << ": " << db.errmsg << std::endl;
	}

	std::string selectClause = "SELECT * FROM iridium_packet";
	std::vector<std::vector<std::string>> vals;
	r = db.select(vals, selectClause);
	if (r) {
		std::cerr << "Error SELECT exec SQL " << r << ": " << db.errmsg << std::endl;
	}

	for (std::vector<std::vector<std::string>>::const_iterator it(vals.begin()); it != vals.end(); it++)
	{
		for (std::vector<std::string>::const_iterator it2(it->begin()); it2 != it->end(); it2++) {
			std::cout << *it2 << "|";
		}
		std::cout << std::endl;
	}

	db.close();
		if (r) {
		std::cerr << "Error close database " << r << ": " << db.errmsg << std::endl;
		exit(r);
	}

	donePkt2(env);
}
