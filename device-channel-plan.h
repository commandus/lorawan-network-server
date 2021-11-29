#ifndef DEVICE_CHANNEL_PLAN_H_
#define DEVICE_CHANNEL_PLAN_H_ 1

#include <string>
#include "utillora.h"
#include "regional-parameter-channel-plans.h"

// Abstract class return device's channel plan
class DeviceChannelPlan {
    protected:
        int defaultRegioanlSettingsBamdChannelPlanIdentifer;
        const RegionalParameterChannelPlans *regionalParameterChannelPlans;
    public:
        DeviceChannelPlan();
        virtual const RegionalParameterChannelPlan *get(const DEVEUI *value) const = 0;

        virtual int init(const std::string &option, void *data) = 0;
        virtual void flush() = 0;
        virtual void done() = 0;

        void setRegionalParameterChannelPlans(const RegionalParameterChannelPlans *plans);
        void setDefaultPlanId(int id);
        bool setDefaultPlanName(const std::string &name);
};

#endif
