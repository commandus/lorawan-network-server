#ifndef DEVICE_CHANNEL_PLAN_FILE_JSON_H_
#define DEVICE_CHANNEL_PLAN_FILE_JSON_H_	1

#include "device-channel-plan.h"

class DeviceChannelPlanFileJson : public DeviceChannelPlan {
    public:
        DeviceChannelPlanFileJson(const RegionalParameterChannelPlans *plans, const int defaultPlanId = 0);
        virtual const RegionalParameterChannelPlan *get(const DEVEUI *value) const override;
        virtual const RegionalParameterChannelPlan *getByAddr(const DEVADDRINT &value) const override;

        virtual int init(const std::string &option, void *data) override;
        virtual void flush() override;
        virtual void done() override;
};

#endif