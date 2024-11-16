#include "clusters/rvc-clean-mode.h"
#include "app-common/zap-generated/callback.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RvcCleanMode;
using namespace chip::app::Clusters::ModeBase;

#define TRACE printf("[+] Entering %s\n", __func__)

CHIP_ERROR RvcCleanModeDelegate::Init()
{
    TRACE;
    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcCleanModeDelegate::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    TRACE;

    if (modeIndex > ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR RvcCleanModeDelegate::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    TRACE;

    if (modeIndex > ArraySize(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    value = kModeOptions[modeIndex].mode;

    return CHIP_NO_ERROR;
}

CHIP_ERROR RvcCleanModeDelegate::GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags)
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

void RvcCleanModeDelegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    TRACE;
    response.status = to_underlying(StatusCode::kCleaningInProgress);
}

static ModeBase::Instance * gRvcCleanModeInstance = nullptr;
static RvcCleanModeDelegate * gRvcCleanModeDelegate = nullptr;

void emberAfRvcCleanModeClusterInitCallback(chip::EndpointId endpoint)
{
    VerifyOrDie(endpoint == 1);
    VerifyOrDie(gRvcCleanModeInstance == nullptr && gRvcCleanModeDelegate == nullptr);

    gRvcCleanModeDelegate = new RvcCleanModeDelegate;
    gRvcCleanModeInstance = new Instance(gRvcCleanModeDelegate, endpoint, RvcCleanMode::Id, 0);

    gRvcCleanModeInstance->Init();
}