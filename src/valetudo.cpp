#include "app-common/zap-generated/cluster-enums.h"
#include "lib/support/TypeTraits.h"
#include "logger.h"
#include "mqtt/valetudo.h"
#include <string_view>
#include <charconv>

using namespace chip::app::Clusters;


namespace MQTT
{

CHIP_ERROR Valetudo::Init()
{
    CHIP_ERROR result;

    if ((result = mBroker.Init(std::bind(&Valetudo::HandlePublish, this, std::placeholders::_1, std::placeholders::_2))) != CHIP_NO_ERROR)
    {
        return result;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Valetudo::SetBatteryLevelCallback(std::function<void(void)> batteryLevelCallback)
{
    if (batteryLevelCallback == nullptr)
    {
        ERROR("batteryLevelCallback is nullptr");
        chipDie();
    }
    mBatteryLevelCallback = batteryLevelCallback;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Valetudo::SetStateCallback(std::function<void(void)> stateCallback)
{
    if (stateCallback == nullptr)
    {
        ERROR("stateCallback is nullptr");
        chipDie();
    }
    mStateCallback = stateCallback;
    return CHIP_NO_ERROR;
}


CHIP_ERROR Valetudo::Locate()
{
    return Publish("LocateCapability/locate/set", "PERFORM");
}

CHIP_ERROR Valetudo::Publish(const std::string & topic, const std::string & message)
{
    std::string homie_topic = mPrefix + "/" + topic;

    return mBroker.Publish(homie_topic, message);
}

void Valetudo::HandlePublish(const std::string & topic, const std::string & message)
{
    // Remove the Valetudo prefix.
    size_t position = topic.find("/") + 1;
    position = topic.find("/", position) + 1;
    std::string_view topic_view = std::string_view(topic).substr(position, topic.size());

    // But keep the prefix for later in case we don't have it.
    if (mPrefix.empty())
    {
        mPrefix = std::string(topic.begin(), topic.begin() + position - 1);
        DEBUG("Prefix is %s", mPrefix.c_str());
    }

    if (topic_view.compare("BatteryStateAttribute/level") == 0)
    {
        uint8_t battery;
        std::from_chars(message.data(), message.data() + message.size(), battery);
        mBatteryLevel = battery;
        return mBatteryLevelCallback();
    }

    if (topic_view.compare("StatusStateAttribute/status") == 0)
    {
        if (message.compare("error") == 0)
            mState = OperationalState::GenericOperationalState(chip::to_underlying(OperationalState::OperationalStateEnum::kError));
        else if (message.compare("docked") == 0)
            mState = OperationalState::GenericOperationalState(chip::to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
        else if (message.compare("returning") == 0)
            mState = OperationalState::GenericOperationalState(chip::to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
        else if (message.compare("cleaning") == 0)
            mState = OperationalState::GenericOperationalState(chip::to_underlying(OperationalState::OperationalStateEnum::kRunning));
        else if (message.compare("paused") == 0)
            mState = OperationalState::GenericOperationalState(chip::to_underlying(OperationalState::OperationalStateEnum::kPaused));
        else
        {
            ERROR("Unknown state: %s", message.c_str());
            chipDie();
        }
        return mStateCallback();
    }

    DEBUG("ValetudoPublish: %s, message: %s", topic_view.data(), message.data());
}

}