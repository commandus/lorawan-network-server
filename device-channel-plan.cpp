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

bool DeviceChannelPlan::setDefaultPlanName(const std::string &name)
{
    if (regionalParameterChannelPlans) {
        const RegionalParameterChannelPlan *p = regionalParameterChannelPlans->get(name);
        if (p) {
            defaultRegioanlSettingsBamdChannelPlanIdentifer = p->id;
            return true;
        }
    }
    return false;
}
