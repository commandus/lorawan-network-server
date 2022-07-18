#include <fstream>
#include <regex>

#include "identity-service-dir-txt.h"
#include "utilstring.h"
#include "utilfile.h"
#include "errlist.h"

/**
 * Looking for a end-device properties by regex string
 * devAddr  devAppSKey                       devNwkSKey                       devEUI           devAppKey                        AppEUI           devAESKey				                                          (mode confirm class_dev) Kosa
 * 1        2                                3                                4                5                                6                7                                                                8 9 10                                                
 * 00000A15 89AA2CDF2D06F9147A424257AB476616 135932B6013CE5FE53C3639BFAB4C26C 3434343500000A15 89AA2CDF2D06F9147A424257AB476616 696B6669616C6734 89AA2CDF2D06F9147A424257AB476616135932B6013CE5FE53C3639BFAB4C26C 0 0 0 NONE
 * 4 16 16 8  16 8  32 1 1 1 NONE
 * 8 32 32 16 32 16 64 1 1 1 NONE
 * grep -E "^[0-9A-Fa-f]{8}\s[0-9A-Fa-f]{32}\s[0-9A-Fa-f]{32}\s[0-9A-Fa-f]{16}\s[0-9A-Fa-f]{32}\s[0-9A-Fa-f]{16}\s[0-9A-Fa-f]{64}\s" doc/lora_devices.txt 
 */ 
int DirTxtIdentityService::loadTxtFile(
	const std::string &filePath
)
{
	std::string res = "^([0-9A-Fa-f]{8})\\s([0-9A-Fa-f]{32})\\s([0-9A-Fa-f]{32})\\s([0-9A-Fa-f]{16})\\s[0-9A-Fa-f]{32}\\s[0-9A-Fa-f]{16}\\s[0-9A-Fa-f]{64}\\s([0-9]{1,2})\\s[0-9]{1,2}\\s([0-9]{1,2})";
	std::regex re(res, std::regex_constants::ECMAScript);
	std::string s;
	std::ifstream f(filePath, std::ios::in);
	std::smatch matches;
	int r = 0;
	std::string name = "";
	while (std::getline(f, s)) {
		if (std::regex_search(s, matches, re)) {
			DEVADDR k;
			DEVICEID v;
			string2DEVADDR(k, matches[1].str());
			string2KEY(v.appSKey, hex2string(matches[2].str()));
			string2KEY(v.nwkSKey, hex2string(matches[3].str()));
			string2DEVEUI(v.devEUI, matches[4].str());
			v.activation = (ACTIVATION) strtoull(matches[5].str().c_str(), NULL, 10);
			v.deviceclass = (DEVICECLASS) strtoull(matches[6].str().c_str(), NULL, 10);
			strncpy(v.name, name.c_str(), sizeof(DEVICENAME));
			put(k, v);
			r++;
		}
	}
	f.close();
	return r;
}

int DirTxtIdentityService::load()
{
	clear();
	std::vector<std::string> files;
	util::filesInPath(path, ".txt", 0, &files);
	int r = 0;
	for (int f = 0; f < files.size(); f++) {
        int rr = loadTxtFile(files[f]);
		if (rr < 0)
            r = rr;
	}
	return r;
}

// Entries count
size_t DirTxtIdentityService::size()
{
    return storage.size();
}

int DirTxtIdentityService::save()
{
	return 0;
} 

int DirTxtIdentityService::startListen(
	OnIdentitiesUpdate value
) {
	onIdentitiesUpdate = value;
	fileWatcher = new filewatch::FileWatch<std::string>(path,
		std::regex(".*\\.txt"), 
		[this](const std::string& path, const filewatch::Event event) {
			// reload
			load();
			onIdentitiesUpdate(this, path, event);
	});
	return 0;
}

int DirTxtIdentityService::stopListen() {
	// onIdentitiesUpdate = NULL;
	if (fileWatcher) {
		delete fileWatcher;
		fileWatcher = NULL;
	}
	return 0;
}
