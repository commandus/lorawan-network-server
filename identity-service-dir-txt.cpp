#include <fstream>
#include <regex>

#include "identity-service-dir-txt.h"
#include "utilstring.h"
#include "errlist.h"

int DirTxtIdentityService::load()
{
	clear();
	/*
    IdentityJsonHandler handler(this);
    rapidjson::Reader reader;
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp)
		return ERR_CODE_INVALID_JSON;
 	char readBuffer[4096];
	rapidjson::FileReadStream istrm(fp, readBuffer, sizeof(readBuffer));
    rapidjson::ParseResult r = reader.Parse(istrm, handler);
	if (r.IsError()) {
		errcode = r.Code();
		std::stringstream ss;
		ss << rapidjson::GetParseError_En(r.Code()) << " at " << r.Offset();
		errmessage = ss.str();
	} else {
		errcode = 0;
		errmessage = "";
	}
	fclose(fp);
	return r.IsError() ? ERR_CODE_INVALID_JSON : 0;
	*/
} 

int DirTxtIdentityService::save()
{
	return 0;
} 
