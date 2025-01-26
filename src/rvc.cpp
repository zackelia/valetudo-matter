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

    if ((result = mValetudo.SetBatteryLevelCallback(std::bind(&RVC::BatteryLevelCallback, this))) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mValetudo.SetCleanModeCallback(std::bind(&RVC::CleanModeCallback, this))) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mValetudo.SetStateCallback(std::bind(&RVC::StateCallback, this))) != CHIP_NO_ERROR)
    {
        return result;
    }

    if ((result = mValetudo.SetSupportedAreasCallback(std::bind(&RVC::SupportedAreasCallback, this))) != CHIP_NO_ERROR)
    {
        return result;
    }

    return CHIP_NO_ERROR;
}

void RVC::Shutdown()
{
    // TODO: Shutdown instances
}

void RVC::BatteryLevelCallback()
{
    DEBUG("Setting BatPercentRemaining to %d", mValetudo.GetBatteryLevel());
    chip::app::Clusters::PowerSource::Attributes::BatPercentRemaining::Set(1, mValetudo.GetBatteryLevel());
}

void RVC::CleanModeCallback()
{
    DEBUG("Setting CleanMode to %d", mValetudo.GetCleanMode());
    mCleanModeInstance.UpdateCurrentMode(mValetudo.GetCleanMode());
}

void RVC::IdentifyCallback()
{
    DEBUG("Identify RVC");
    mValetudo.Locate();
}

void RVC::StateCallback()
{
    DEBUG("Setting OperationalState to %d", mValetudo.GetState().operationalStateID);
    mRvcOperationalStateInstance.SetOperationalState(mValetudo.GetState().operationalStateID);
}

void RVC::SetCleanMode(uint8_t cleanMode)
{
    mValetudo.SetCleanMode(cleanMode);
}

void RVC::SupportedAreasCallback()
{
    mServiceAreaInstance.ClearSelectedAreas();

    for (size_t i = 0; i < mValetudo.GetSupportedAreas().size(); i++)
    {
        const auto area = mValetudo.GetSupportedAreas()[i];
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
