#pragma once

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

#include "clusters/rvc-clean-mode.h"
#include "clusters/rvc-operational-state.h"
#include "clusters/rvc-run-mode.h"

namespace chip::app::Clusters
{

class RVC
{
public:
    RVC() :
        mCleanModeInstance(&mCleanModeDelegate, ENDPOINT_ID, RvcCleanMode::Id, 0),
        mRvcOperationalStateInstance(&mRvcOperationalStateDelegate, ENDPOINT_ID),
        mRunModeInstance(&mRunModeDelegate, ENDPOINT_ID, RvcRunMode::Id, 0)
    {
    }

    CHIP_ERROR Init();
    void Shutdown();

private:
    static constexpr EndpointId ENDPOINT_ID = 1;

    RvcCleanMode::RvcCleanModeDelegate mCleanModeDelegate;
    RvcOperationalState::RvcOperationalStateDelegate mRvcOperationalStateDelegate;
    RvcRunMode::RvcRunModeDelegate mRunModeDelegate;

    ModeBase::Instance mCleanModeInstance;
    RvcOperationalState::Instance mRvcOperationalStateInstance;
    ModeBase::Instance mRunModeInstance;
};

}