#include <string>
#include <iostream>

#include <google/protobuf/message.h>

#include "pkt2/str-pkt2.h"

#include "db-any.h"

int main(int argc, char **argv) {
	std::string protoPath = "proto";
	void* env = initPkt2(protoPath, 0);
	if (!env) {
		std::cerr << "Init error" << std::endl;
		exit(1);
	}

	ConfigDatabases configDatabases("tests/dbs.js");
	std::cerr << configDatabases.toString() << std::endl;
	DatabaseByConfig dbAny(&configDatabases);
	
	std::string mt = "iridium.IEPacket";
	std::string hexData = "01004e01001c9a0ba5f633303032333430363032333533343000011900005ab8f59303000b003e68a68143d40000000502001e0810003e01b21200004e812b4e160000390000221400829486247a0d1c09";

	// const ConfigDatabase *config;
	std::string dbs[] = {
		"sqlite",
		"postgres"
	};

	for (int d = 1; d < 2; d++)
	{
		DatabaseNConfig *db = dbAny.find(dbs[d]);
		if (!db) {
			std::cerr << "Can not find db " << dbs[d] << std::endl;
			exit(2);
		}

		int r = db->open();
		if (r) {
			std::cerr << "Error open database " << r << ": " << db->db->errmsg << std::endl;
			exit(r);
		}

		r = db->createTable(env, mt);
		if (r) {
			std::cerr << "Error CREATE table " << r << ": " << db->db->errmsg << std::endl;
			std::cerr << "Clause " << db->createClause(env, mt) << std::endl;
		}

		r = db->insert(env, mt, INPUT_FORMAT_HEX, hexData, NULL);
		if (r) {
			std::cerr << "Error INSERT " << r << ": " << db->db->errmsg << std::endl;
			std::cerr << "Clause " << db->insertClause(env, mt, INPUT_FORMAT_HEX, hexData, NULL) << std::endl;
		}

		std::string selectClause = "SELECT * FROM iridium_packet";
		std::vector<std::vector<std::string>> vals;
		r = db->select(vals, selectClause);
		if (r) {
			std::cerr << "Error SELECT exec SQL " << r << ": " << db->db->errmsg << std::endl;
		}

		for (std::vector<std::vector<std::string>>::const_iterator it(vals.begin()); it != vals.end(); it++)
		{
			for (std::vector<std::string>::const_iterator it2(it->begin()); it2 != it->end(); it2++) {
				std::cout << *it2 << "|";
			}
			std::cout << std::endl;
		}

		r = db->close();
		if (r) {
			std::cerr << "Error close database " << r << ": " << db->db->errmsg << std::endl;
			exit(r);
		}
	}

	donePkt2(env);

}
