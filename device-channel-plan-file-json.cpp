#include "device-channel-plan-file-json.h"

DeviceChannelPlanFileJson::DeviceChannelPlanFileJson(const RegionalParameterChannelPlans *plans, const int defaultPlanId )
{
    setRegionalParameterChannelPlans(plans);
    setDefaultPlanId(defaultPlanId);
}

const RegionalParameterChannelPlan *DeviceChannelPlanFileJson::get(const DEVADDRINT &value) const
{
    return get();
}

const RegionalParameterChannelPlan *DeviceChannelPlanFileJson::get() const
{
    return regionalParameterChannelPlans->get(defaultRegioanlSettingsBamdChannelPlanIdentifer);
}

int DeviceChannelPlanFileJson::init(const std::string &option, void *data)
{

}

void DeviceChannelPlanFileJson::flush()
{

}

void DeviceChannelPlanFileJson::done()
{

}
