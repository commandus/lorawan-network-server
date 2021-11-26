#include "device-channel-plan.h"

DeviceChannelPlan::DeviceChannelPlan()
    : defaultRegioanlSettingsBamdChannelPlanIdentifer(0), regionalParameterChannelPlans(nullptr)
{

}

void DeviceChannelPlan::setRegionalParameterChannelPlans(const RegionalParameterChannelPlans *plans)
{
    regionalParameterChannelPlans = plans;
}

void DeviceChannelPlan::setDefaultPlanId(int id)
{
    defaultRegioanlSettingsBamdChannelPlanIdentifer = id;
}
