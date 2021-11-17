#ifndef REGION_BAND_FILE_JSON_H_
#define REGION_BAND_FILE_JSON_H_ 1

#include <vector>
#include "region-band.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class RegionBandsFileJson {
	private:
		int load();
		int save();
	protected:
        std::string path;
		void clear();		

	public:
        RegionBands storage;
        RegionBandsFileJson();
		~RegionBandsFileJson();

		int init(const std::string &option, void *data);
		void flush();
		void done();

		int errcode;
		std::string errMessage;

};

#endif
