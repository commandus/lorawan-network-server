#include "device-channel-plan.h"

DeviceChannelPlan::DeviceChannelPlan()
    : defaultChannelPlanIdentifier(0), regionalParameterChannelPlans(nullptr)
{

}

void DeviceChannelPlan::setRegionalParameterChannelPlans(const RegionalParameterChannelPlans *plans)
{
    regionalParameterChannelPlans = plans;
}

void DeviceChannelPlan::setDefaultPlanId(int id)
{
    defaultChannelPlanIdentifier = id;
}

bool DeviceChannelPlan::setDefaultPlanName(const std::string &name)
{
    if (regionalParameterChannelPlans) {
        const RegionalParameterChannelPlan *p = regionalParameterChannelPlans->get(name);
        if (p) {
            defaultChannelPlanIdentifier = p->id;
            return true;
        }
    }
    return false;
}
