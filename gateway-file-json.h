#ifndef GATEWAY_FILE_JSON_H_
#define GATEWAY_FILE_JSON_H_ 1

#include <vector>
#include "regional-parameter-channel-plan.h"
#include "regional-parameter-channel-plans.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#include "rapidjson/document.h"
#pragma clang diagnostic pop

class GatewayConfigFileJson : public RegionalParameterChannelPlans {
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
        GatewayConfigFileJson();
		~GatewayConfigFileJson();
        int errCode;
        std::string errDescription;

        virtual const RegionalParameterChannelPlan *get(const std::string &name) const override;
        virtual const RegionalParameterChannelPlan *get(int id) const override;

        virtual int init(const std::string &option, void *data) override;
        virtual void flush() override;
        virtual void done() override;

        virtual std::string toJsonString() const override;

        virtual std::string getErrorDescription(int &subCode) const override;
};

#endif
