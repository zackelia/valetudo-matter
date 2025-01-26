#pragma once

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

#include "clusters/rvc-clean-mode.h"
#include "clusters/rvc-operational-state.h"
#include "clusters/rvc-run-mode.h"
#include "clusters/rvc-service-area.h"
#include "clusters/rvc-service-area-storage.h"
#include "mqtt/valetudo.h"
#include "logger.h"
#include "socket.h"

namespace chip::app::Clusters
{

class RVC
{
public:
    RVC() :
        mCleanModeInstance(&mCleanModeDelegate, ENDPOINT_ID, RvcCleanMode::Id, 0),
        mRvcOperationalStateInstance(&mRvcOperationalStateDelegate, ENDPOINT_ID),
        mRunModeInstance(&mRunModeDelegate, ENDPOINT_ID, RvcRunMode::Id, 0),
        mServiceAreaInstance(&mServiceAreaStorageDelegate, &mServiceAreaDelegate, ENDPOINT_ID, BitMask<ServiceArea::Feature>())
    {
        mCleanModeDelegate.SetRVC(this);
        mRvcOperationalStateDelegate.SetRVC(this);
        mRunModeDelegate.SetRVC(this);
        mServiceAreaDelegate.SetRVC(this);

        mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);
    }

    CHIP_ERROR Init();
    void Shutdown();

    void GoHome(OperationalState::GenericOperationalError & err)
    {
        TRACE;

        CHIP_ERROR error = mRvcOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger));

        err.Set((error == CHIP_NO_ERROR) ? to_underlying(OperationalState::ErrorStateEnum::kNoError)
                                         : to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation));
    }

    void SetCleanMode(uint8_t);

private:
    static constexpr EndpointId ENDPOINT_ID = 1;

    RvcCleanMode::RvcCleanModeDelegate mCleanModeDelegate;
    RvcOperationalState::RvcOperationalStateDelegate mRvcOperationalStateDelegate;
    RvcRunMode::RvcRunModeDelegate mRunModeDelegate;
    ServiceArea::RvcServiceAreaDelegate mServiceAreaDelegate;
    ServiceArea::RvcServiceAreaStorageDelegate mServiceAreaStorageDelegate;

    ModeBase::Instance mCleanModeInstance;
    RvcOperationalState::Instance mRvcOperationalStateInstance;
    ModeBase::Instance mRunModeInstance;
    ServiceArea::Instance mServiceAreaInstance;

    MQTT::Valetudo mValetudo;

    void BatteryLevelCallback();
    void CleanModeCallback();
    void IdentifyCallback();
    void StateCallback();
    void SupportedAreasCallback();

};

}