#pragma once

#include <app/clusters/service-area-server/service-area-server.h>

namespace chip::app::Clusters
{
class RVC;
}

namespace chip::app::Clusters::ServiceArea
{

class RvcServiceAreaDelegate : public Delegate
{
  public:
    bool IsSetSelectedAreasAllowed(MutableCharSpan & statusText) override;

    bool IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas, SelectAreasStatus & locationStatus,
                               MutableCharSpan & statusText) override;

    bool IsSupportedAreasChangeAllowed() override;

    bool IsSupportedMapChangeAllowed() override;

    void SetRVC(RVC * rvc);

  private:
    RVC * mRvc = nullptr;
};

}
