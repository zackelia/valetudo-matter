#include "app-common/zap-generated/cluster-enums.h"
#include "clusters/rvc-clean-mode.h"
#include "lib/core/CHIPError.h"
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
    CHIP_ERROR error = mRvcOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));

    err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                        : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));

    // TODO: Control Valetudo
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
