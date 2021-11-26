#ifndef REGION_BAND_FILE_JSON_H_
#define REGION_BAND_FILE_JSON_H_ 1

#include <vector>
#include "regional-parameter-channel-plan.h"
#include "regional-parameter-channel-plans.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

/**
 * RegionalParameterChannelPlanFileJson class load LoRaWAN region settings from the
 * regional-parameters.json JSON file. This file looks like:
 * {
	"regionalParametersVersion": "1.0.3",
	"RegionBands": [{
        "id": 14,
		"name": "RU864-870",
		"supportsExtraChannels": true,
		"bandDefaults": {
			"RX2Frequency": 869100000,
			"RX2DataRate": 0,
			"ReceiveDelay1": 1,
			"ReceiveDelay2": 2,
			"JoinAcceptDelay1": 5,
			"JoinAcceptDelay2": 6
		},
		"uplinkChannels": [{
			"frequency": 868900000,
			"minDR": 0,
			"maxDR": 5,
			"enabled": true,
			"custom": false
		}, ...],
		"downlinkChannels": [...],
		"maxPayloadSizePerDataRate": [{
			"m": 59,
			"n": 51
		},...],
		"maxPayloadSizePerDataRateRepeater": [{
			"m": 59,
			"n": 51
		}, ...],
		"rx1DataRateOffsets": [
			[0, 0, 0, 0, 0, 0],...
		],
		"txPowerOffsets": [0, -2, -4, -6, -8, -10, -12, -14]
	}]
}
 */
class RegionalParameterChannelPlanFileJson : public RegionalParameterChannelPlans {
	private:
        // helper hash map to get() region by the name
        std::map<std::string, const RegionalParameterChannelPlan *> nameIndex;
        std::map<int, const RegionalParameterChannelPlan *> idIndex;
        // default RegionalParameterChannelPlan
        // last region marked as default ("defaultRegion": true) is used
        const RegionalParameterChannelPlan *defaultRegionBand;
        int buildIndex();
        int loadFile(const std::string &fileName);
        int saveFile(const std::string &fileName) const;
		int load();
		int save();
	protected:
        std::string path;
		void clear();		

	public:
        RegionBands storage;
        RegionalParameterChannelPlanFileJson();
		~RegionalParameterChannelPlanFileJson();

        virtual const RegionalParameterChannelPlan *get(const std::string &name) const override;
        virtual const RegionalParameterChannelPlan *get(int id) const override;

        virtual int init(const std::string &option, void *data) override;
        virtual void flush() override;
        virtual void done() override;

        virtual std::string toJsonString() const override;

        int errcode;
		std::string errMessage;
};

#endif
