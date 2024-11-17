#pragma once

#include <app/clusters/operational-state-server/operational-state-server.h>

namespace chip::app::Clusters
{
    class RVC;
}

namespace chip::app::Clusters::RvcOperationalState
{

class RvcOperationalStateDelegate : public Delegate
{
public:
    app::DataModel::Nullable<uint32_t> GetCountdownTime() override;

    CHIP_ERROR GetOperationalStateAtIndex(size_t index, OperationalState::GenericOperationalState & operationalState) override;

    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;

    void HandlePauseStateCallback(OperationalState::GenericOperationalError & err) override;

    void HandleResumeStateCallback(OperationalState::GenericOperationalError & err) override;

    void HandleGoHomeCommandCallback(OperationalState::GenericOperationalError & err) override;

    void SetRVC(RVC * rvc);

private:
    const Clusters::OperationalState::GenericOperationalState mOperationalStateList[7] = {
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kError)),
        OperationalState::GenericOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kSeekingCharger)),
        OperationalState::GenericOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kCharging)),
        OperationalState::GenericOperationalState(to_underlying(Clusters::RvcOperationalState::OperationalStateEnum::kDocked)),
    };
    const Span<const CharSpan> mOperationalPhaseList;

    RVC * mRvc = nullptr;
};

}
