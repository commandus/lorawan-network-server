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
        // helper to get()
        std::map<std::string, const RegionBand *> nameIndex;
        // default RegionBand
        const RegionBand *defaultRegionBand;
        void buildIndex();
		int load();
		int save();
	protected:
        std::string path;
		void clear();		

	public:
        RegionBands storage;
        RegionBandsFileJson();
		~RegionBandsFileJson();

        const RegionBand *get(const std::string &name) const;

		int init(const std::string &option, void *data);
		void flush();
		void done();

		int errcode;
		std::string errMessage;

};

#endif
