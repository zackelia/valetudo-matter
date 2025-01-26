#include <app-common/zap-generated/cluster-objects.h>
#include "app-common/zap-generated/cluster-enums.h"
#include "lib/support/TypeTraits.h"

#include "clusters/rvc-service-area.h"
#include "logger.h"
#include "rvc.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ServiceArea;

bool RvcServiceAreaDelegate::IsSetSelectedAreasAllowed(MutableCharSpan & statusText)
{
    // TODO: Verify?
    return true;
}

bool RvcServiceAreaDelegate::IsValidSelectAreasSet(const Span<const uint32_t> &selectedAreas, SelectAreasStatus &locationStatus, MutableCharSpan &statusText)
{
    // TODO: Verify?
    return true;
}

bool RvcServiceAreaDelegate::IsSupportedAreasChangeAllowed()
{
    // TODO: Verify?
    return true;
}

bool RvcServiceAreaDelegate::IsSupportedMapChangeAllowed()
{
    ERROR("IsSupportedMapChangeAllowed");
    chipDie();
}

void RvcServiceAreaDelegate::SetRVC(RVC * rvc)
{
    mRvc = rvc;
}
