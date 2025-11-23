#include "rvc.h"
#include "app-common/zap-generated/attributes/Accessors.h"
#include "app/clusters/mode-base-server/mode-base-cluster-objects.h"
#include "clusters/rvc-clean-mode.h"
#include "clusters/rvc-run-mode.h"
#include "lib/core/CHIPError.h"
#include "lib/support/TypeTraits.h"
#include "logger.h"

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

void RVC::HandleRunMode(uint8_t targetMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    TRACE;
    uint8_t currentState = mRvcOperationalStateInstance.GetCurrentOperationalState();
    uint8_t currentMode = mRunModeInstance.GetCurrentMode();
    DEBUG("CurrentState: %d, CurrentMode: %d, TargetMode: %d", currentState, currentMode, targetMode);

    if (targetMode == RvcRunMode::ModeCleaning)
    {
        if (currentState != to_underlying(OperationalState::OperationalStateEnum::kStopped) &&
            currentState != to_underlying(RvcOperationalState::OperationalStateEnum::kDocked) &&
            currentState != to_underlying(RvcOperationalState::OperationalStateEnum::kCharging))
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue(
                chip::CharSpan::fromCharString("Change to the mapping or cleaning mode is only allowed from idle"));
            return;
        }

        mRunModeInstance.UpdateCurrentMode(targetMode);
        mRvcOperationalStateInstance.SetOperationalState(
            to_underlying(RvcOperationalState::OperationalStateEnum::kRunning));

        // Selecting rooms is only valid when starting, not resuming.
        std::vector<uint32_t> areas;
        if (mRvcOperationalStateInstance.GetCurrentOperationalState() !=
            to_underlying(OperationalState::OperationalStateEnum::kPaused))
        {
            for (uint32_t i = 0; i < mServiceAreaInstance.GetNumberOfSelectedAreas(); i++)
            {
                uint32_t area;
                mServiceAreaInstance.GetSelectedAreaByIndex(i, area);
                DEBUG("Selected room: %s", mValetudo.GetSupportedAreas().value()[area].c_str());
                areas.push_back(area);
            }
        }

        CHIP_ERROR error;
        if (!areas.empty())
            error = mValetudo.Start(areas);
        else
            error = mValetudo.Start();

        if (error != CHIP_NO_ERROR)
        {
            response.status = to_underlying(ModeBase::StatusCode::kGenericFailure);
            response.statusText.SetValue(chip::CharSpan::fromCharString("Failed to start vacuum"));
            return;
        }

        response.status = to_underlying(ModeBase::StatusCode::kSuccess);
    }
    else if (targetMode == RvcRunMode::ModeIdle)
    {
        if (currentState == to_underlying(OperationalState::OperationalStateEnum::kRunning) ||
            currentState == to_underlying(OperationalState::OperationalStateEnum::kPaused))
        {
            mRunModeInstance.UpdateCurrentMode(targetMode);
            mRvcOperationalStateInstance.SetOperationalState(
                to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
            mValetudo.Home();
            response.status = to_underlying(ModeBase::StatusCode::kSuccess);
        }
        else
        {
            response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
            response.statusText.SetValue(
                chip::CharSpan::fromCharString("Change to the mapping or cleaning mode is only allowed from idle"));
            return;
        }
    }
    else
    {
        response.status = to_underlying(ModeBase::StatusCode::kInvalidInMode);
        response.statusText.SetValue(chip::CharSpan::fromCharString("This change is not allowed at this time"));
    }
}

void RVC::HandleGoHome(OperationalState::GenericOperationalError & err)
{
    switch (mRvcOperationalStateInstance.GetCurrentOperationalState())
    {
    case to_underlying(OperationalState::OperationalStateEnum::kStopped):
    case to_underlying(OperationalState::OperationalStateEnum::kRunning):
    case to_underlying(OperationalState::OperationalStateEnum::kPaused): {
        CHIP_ERROR error = mRvcOperationalStateInstance.SetOperationalState(
            to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));
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
    case to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger): {
        CHIP_ERROR error = mRvcOperationalStateInstance.SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kPaused));
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
    case to_underlying(RvcOperationalState::OperationalStateEnum::kDocked): {
        CHIP_ERROR error = mRvcOperationalStateInstance.SetOperationalState(
            to_underlying(OperationalState::OperationalStateEnum::kPaused));
        if (error != CHIP_NO_ERROR)
        {
            err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
            return;
        }

        error = mValetudo.Start();

        err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                         : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));

        // Selected rooms are only valid for one run.
        if (!mServiceAreaInstance.ClearSelectedAreas())
        {
            ERROR("Failed to clear selected areas.");
            chipDie();
        }

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

void RVC::UpdateSupportedAreas(const std::map<uint32_t, std::string> & areas)
{
    if (!mServiceAreaInstance.ClearSelectedAreas())
    {
        ERROR("Failed to clear selected areas.");
        chipDie();
    }

    if (!mServiceAreaInstance.ClearSupportedAreas())
    {
        ERROR("Failed to clear supported areas.");
        chipDie();
    }

    for (const auto & [id, area] : areas)
    {
        chip::Span<const char> span(area.data(), area.size());
        auto area_wrapper = ServiceArea::AreaStructureWrapper{}.SetAreaId(id).SetLocationInfo(
            span, DataModel::NullNullable, DataModel::NullNullable);
        if (!mServiceAreaInstance.AddSupportedArea(area_wrapper))
        {
            ERROR("Failed to add area");
            chipDie();
        }
    }
}
