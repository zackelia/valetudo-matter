#include "app-common/zap-generated/cluster-enums.h"
#include "clusters/rvc-clean-mode.h"
#include "lib/core/CHIPError.h"
#include "lib/core/ErrorStr.h"
#include "lib/support/TypeTraits.h"
#include "logger.h"
#include "platform/CHIPDeviceLayer.h"
#include "rvc.h"
#include "system/SocketEvents.h"
#include "app-common/zap-generated/attributes/Accessors.h"

using namespace chip::app::Clusters;

CHIP_ERROR RVC::Init()
{
    CHIP_ERROR result;

    if ((result = mCleanModeInstance.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mRvcOperationalStateInstance.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mRunModeInstance.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mServiceAreaInstance.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mValetudo.Init()) != CHIP_NO_ERROR)
    {
        return result;
    }

    return CHIP_NO_ERROR;
}

void RVC::Shutdown()
{
    // TODO: Shutdown instances
}

void RVC::HandleCleanMode(uint8_t cleanMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    mValetudo.SetCleanMode(cleanMode);
}

void RVC::HandleGoHome(OperationalState::GenericOperationalError & err)
{
    switch (mRvcOperationalStateInstance.GetCurrentOperationalState())
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped):
    case to_underlying(OperationalState::OperationalStateEnum::kRunning):
    case to_underlying(OperationalState::OperationalStateEnum::kPaused):
    {
        CHIP_ERROR error = mRvcOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
        if (error != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }

        error = mValetudo.Home();
        err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                            : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }
    default:
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }
}

void RVC::HandlePause(OperationalState::GenericOperationalError & err)
{
    switch (mRvcOperationalStateInstance.GetCurrentOperationalState())
    {
    case to_underlying(OperationalState::OperationalStateEnum::kRunning):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger):
    {
        CHIP_ERROR error = mRvcOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
        if (error != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }

        error = mValetudo.Pause();
        err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                            : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }
    default:
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }
}

void RVC::HandleResume(OperationalState::GenericOperationalError & err)
{
    switch (mRvcOperationalStateInstance.GetCurrentOperationalState())
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped):
    case to_underlying(OperationalState::OperationalStateEnum::kPaused):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kCharging):
    case to_underlying(RvcOperationalState::OperationalStateEnum::kDocked):
    {
        CHIP_ERROR error = mRvcOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
        if (error != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }

        error = mValetudo.Start();
        err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                            : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
        return;
    }
    default:
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
        return;
    }
}

void RVC::HandleIdentify()
{
    DEBUG("Identify RVC");
    mValetudo.Locate();
}

void RVC::UpdateBatteryLevel(const uint8_t battery_level)
{
    DEBUG("Setting BatPercentRemaining to %d", battery_level);
    chip::app::Clusters::PowerSource::Attributes::BatPercentRemaining::Set(ENDPOINT_ID, battery_level);
}

void RVC::UpdateCleanMode(const uint8_t clean_mode)
{
    DEBUG("Setting CleanMode to %d", clean_mode);
    mCleanModeInstance.UpdateCurrentMode(clean_mode);
}

void RVC::UpdateOperationalState(const uint8_t state)
{
    DEBUG("Setting OperationalState to %d", state);
    mRvcOperationalStateInstance.SetOperationalState(state);
}

void RVC::UpdateOperationalError()
{
    detail::Structs::ErrorStateStruct::Type err;

    if (mValetudo.GetDustBinInstalled().has_value() && !mValetudo.GetDustBinInstalled().value())
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kDustBinMissing);
    else if (mValetudo.GetMopInstalled().has_value() && !mValetudo.GetMopInstalled().value())
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kMopCleaningPadMissing);
    else if (mValetudo.GetWaterTankInstalled().has_value() && !mValetudo.GetWaterTankInstalled().value())
        err.errorStateID = to_underlying(RvcOperationalState::ErrorStateEnum::kWaterTankMissing);
    else
    {
        DEBUG("Clearing OperationalErrors");
        mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);
        return;
    }

    DEBUG("Setting OperationalError to %d", err.errorStateID);
    mRvcOperationalStateInstance.OnOperationalErrorDetected(err);
}

void RVC::UpdateSupportedAreas(const std::vector<std::string> & areas)
{
    mServiceAreaInstance.ClearSelectedAreas();

    for (size_t i = 0; i < areas.size(); i++)
    {
        const auto area = areas[i];
        chip::Span<const char> span(area.data(), area.size());
        auto area_wrapper = ServiceArea::AreaStructureWrapper{}
            .SetAreaId(i)
            .SetLocationInfo(span, DataModel::NullNullable, DataModel::NullNullable);
        if (!mServiceAreaInstance.AddSupportedArea(area_wrapper))
        {
            ERROR("Failed to add area");
            chipDie();
        }
    }
}
