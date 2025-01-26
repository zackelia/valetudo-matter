#include "app-common/zap-generated/cluster-enums.h"
#include "clusters/rvc-clean-mode.h"
#include "json/json.h"
#include "lib/support/CodeUtils.h"
#include "lib/support/TypeTraits.h"
#include "logger.h"
#include <memory>
#include "mqtt/valetudo.h"
#include <string_view>
#include <charconv>
#include "rvc.h"

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

CHIP_ERROR Valetudo::Locate()
{
    return Publish("LocateCapability/locate/set", "PERFORM");
}

CHIP_ERROR Valetudo::SetCleanMode(uint8_t cleanMode)
{
    std::string data;
    if (cleanMode == RvcCleanMode::ModeMop)
        data = "mop";
    else if (cleanMode == RvcCleanMode::ModeVacuum)
        data = "vacuum";
    else if (cleanMode == RvcCleanMode::ModeVacuumAndMop)
        data = "vacuum_and_mop";
    else
    {
        ERROR("Unknown mode: %d", cleanMode);
        chipDie();
    }
    return Publish("OperationModeControlCapability/preset/set", data);
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
        return mRvc->UpdateBatteryLevel(mBatteryLevel.value());
    }

    if (topic_view.compare("OperationModeControlCapability/preset") == 0)
    {
        if (message.compare("mop") == 0)
            mCleanMode = RvcCleanMode::ModeMop;
        else if (message.compare("vacuum") == 0)
            mCleanMode = RvcCleanMode::ModeVacuum;
        else if (message.compare("vacuum_and_mop") == 0)
            mCleanMode = RvcCleanMode::ModeVacuumAndMop;
        else
        {
            ERROR("Unknown mode: %s", message.c_str());
            chipDie();
        }
        return mRvc->UpdateCleanMode(mCleanMode.value());
    }

    if (topic_view.compare("StatusStateAttribute/status") == 0)
    {
        if (message.compare("error") == 0)
            mOperationalState = OperationalState::GenericOperationalState(chip::to_underlying(OperationalState::OperationalStateEnum::kError)).operationalStateID;
        else if (message.compare("docked") == 0)
            mOperationalState = OperationalState::GenericOperationalState(chip::to_underlying(RvcOperationalState::OperationalStateEnum::kDocked)).operationalStateID;
        else if (message.compare("returning") == 0)
            mOperationalState = OperationalState::GenericOperationalState(chip::to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger)).operationalStateID;
        else if (message.compare("cleaning") == 0)
            mOperationalState = OperationalState::GenericOperationalState(chip::to_underlying(OperationalState::OperationalStateEnum::kRunning)).operationalStateID;
        else if (message.compare("paused") == 0)
            mOperationalState = OperationalState::GenericOperationalState(chip::to_underlying(OperationalState::OperationalStateEnum::kPaused)).operationalStateID;
        else
        {
            ERROR("Unknown state: %s", message.c_str());
            chipDie();
        }
        return mRvc->UpdateOperationalState(mOperationalState.value());
    }

    if (topic_view.compare("MapData/segments") == 0)
    {
        JSONCPP_STRING err;
        Json::Value root;
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(message.c_str(), message.c_str() + message.length(), &root,
                       &err))
        {
            ERROR("%s", err.c_str());
            chipDie();
        }

        std::vector<std::string> keys = root.getMemberNames();
        std::vector<std::string> values;
        for (const auto& key : keys)
        {
            values.push_back(root[key].asCString());
        }
        mSupportedAreas = values;
        return mRvc->UpdateSupportedAreas(mSupportedAreas.value());
    }

    DEBUG("ValetudoPublish: %s, message: %s", topic_view.data(), message.data());
}

void Valetudo::SetRVC(RVC * rvc)
{
    mRvc = rvc;
}

}