#include <app-common/zap-generated/cluster-objects.h>

#include "clusters/rvc-operational-state.h"
#include "logger.h"
#include "rvc.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RvcOperationalState;

app::DataModel::Nullable<uint32_t> RvcOperationalStateDelegate::GetCountdownTime()
{
    TRACE;
    return {};
}

CHIP_ERROR RvcOperationalStateDelegate::GetOperationalStateAtIndex(
    size_t index, OperationalState::GenericOperationalState & operationalState)
{
    TRACE;

    if (index >= ArraySize(mOperationalStateList))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    operationalState = mOperationalStateList[index];

    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcOperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    TRACE;

    if (index >= mOperationalPhaseList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
}

void RvcOperationalStateDelegate::HandlePauseStateCallback(OperationalState::GenericOperationalError & err)
{
    TRACE;
    mRvc->HandlePause(err);
}

void RvcOperationalStateDelegate::HandleResumeStateCallback(OperationalState::GenericOperationalError & err)
{
    TRACE;
    mRvc->HandleResume(err);
}

void RvcOperationalStateDelegate::HandleGoHomeCommandCallback(OperationalState::GenericOperationalError & err)
{
    TRACE;
    mRvc->HandleGoHome(err);
}

void RvcOperationalStateDelegate::SetRVC(RVC * rvc)
{
    mRvc = rvc;
}
