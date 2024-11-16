#include <app-common/zap-generated/cluster-objects.h>
#include "app-common/zap-generated/callback.h"
#include "app-common/zap-generated/cluster-enums.h"

#include "clusters/rvc-run-mode.h"
#include "lib/core/CHIPError.h"
#include "lib/support/TypeTraits.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RvcRunMode;
using namespace chip::app::Clusters::ModeBase;

#define TRACE printf("[+] Entering %s\n", __func__)

CHIP_ERROR RvcRunModeDelegate::Init()
{
    TRACE;
    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcRunModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    TRACE;

    if (modeIndex > ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR RvcRunModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    TRACE;

    if (modeIndex > ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    value = kModeOptions[modeIndex].mode;

    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcRunModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags)
{
    TRACE;

    if (modeIndex > ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (modeTags.size() < kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), modeTags.begin());
    modeTags.reduce_size(kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}

void RvcRunModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    TRACE;
    response.status = to_underlying(StatusCode::kBatteryLow);
}

static ModeBase::Instance * gRvcRunModeInstance = nullptr;
static RvcRunModeDelegate * gRvcRunModeDelegate = nullptr;

void emberAfRvcRunModeClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(endpoint == 1);
    VerifyOrDie(gRvcRunModeInstance == nullptr && gRvcRunModeDelegate == nullptr);

    gRvcRunModeDelegate = new RvcRunModeDelegate;
    gRvcRunModeInstance = new Instance(gRvcRunModeDelegate, endpoint, RvcRunMode::Id, 0);

    gRvcRunModeInstance->Init();
}