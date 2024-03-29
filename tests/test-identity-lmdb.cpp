#include <iostream>
#include "db-identity.h"
#include "identity-service-lmdb.h"
#include "utilstring.h"
#include "errlist.h"

int main(int argc, char **argv) {
	LmdbIdentityService s;
	int r = s.init("db", nullptr);
	if (r) {
		std::cerr << "Error " <<  r << ": " << strerror_lorawan_ns(r) << std::endl;
        if (r == ERR_CODE_LMDB_OPEN) {
            std::cerr << "Trying 'mkdir db'.." << std::endl;
			r = system("mkdir db");
			if (r) {
				std::cerr << "create directory 'db' failed. Manually create directory 'db' and try again" << std::endl;
				exit(r);
			}
			r = s.init("db", nullptr);
			if (r) {
				std::cerr << "Something wrong with directory 'db', database creation failed" << std::endl;
				exit(r);
			}
			std::cerr << "db subdirectory successlully created" << std::endl;
        } else 
        	exit(r);
	}
	DEVADDR a;
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	a[3] = 4;

	DEVICEID id;
	id.devEUI[0] = 1;
	id.devEUI[1] = 2;
	id.devEUI[2] = 3;
	id.devEUI[3] = 4;
	id.devEUI[4] = 5;
	id.devEUI[5] = 6;
	id.devEUI[6] = 7;
	id.devEUI[7] = 8;

	id.nwkSKey[0] = 1;
	id.nwkSKey[1] = 2;
	id.nwkSKey[2] = 3;
	id.nwkSKey[3] = 4;
	id.nwkSKey[4] = 5;
	id.nwkSKey[5] = 6;
	id.nwkSKey[6] = 7;

	id.nwkSKey[10] = 11;
	id.nwkSKey[11] = 12;
	id.nwkSKey[12] = 13;
	id.nwkSKey[13] = 14;
	id.nwkSKey[14] = 15;
	id.nwkSKey[15] = 16;

	id.appSKey[0] = 1;
	id.appSKey[1] = 2;
	id.appSKey[2] = 3;
	id.appSKey[3] = 4;
	id.appSKey[4] = 5;
	id.appSKey[5] = 6;
	id.appSKey[6] = 7;
	id.appSKey[7] = 8;
	id.appSKey[8] = 9;
	id.appSKey[9] = 10;
	id.appSKey[10] = 11;
	id.appSKey[11] = 12;
	id.appSKey[12] = 13;
	id.appSKey[13] = 14;
	id.appSKey[14] = 15;
	id.appSKey[15] = 16;

	s.put(a, id);

	a[0] = 2;
	a[1] = 3;
	a[2] = 4;
	a[3] = 5;
	s.put(a, id);

	a[0] = 3;
	a[1] = 4;
	a[2] = 5;
	a[3] = 6;
	s.put(a, id);

	std::vector<NetworkIdentity> l;
	
	std::cerr << "--" << std::endl;
	s.list(l, 0, 100);
	for (std::vector<NetworkIdentity>::const_iterator it(l.begin()); it != l.end(); it++) {
		std::cout << it->toString() << std::endl;
	}

	a[0] = 2;
	a[1] = 3;
	a[2] = 4;
	a[3] = 5;
	s.rm(a);

	std::cerr << "--" << std::endl;
	l.clear();
	s.list(l, 0, 100);
	for (std::vector<NetworkIdentity>::const_iterator it(l.begin()); it != l.end(); it++) {
		std::cout << it->toString() << std::endl;
	}

}
