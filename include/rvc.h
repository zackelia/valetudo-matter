#pragma once

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

#include "app-common/zap-generated/cluster-enums.h"
#include "clusters/rvc-clean-mode.h"
#include "clusters/rvc-operational-state.h"
#include "clusters/rvc-run-mode.h"
#include "clusters/rvc-service-area.h"
#include "clusters/rvc-service-area-storage.h"
#include "mqtt/valetudo.h"

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
        // TODO: This is for convenience of not having a ton of callback
        // setting. It does break the idea of encapsulation though. Should this
        // be fixed? What about the cluster delegates?
        mValetudo.SetRVC(this);

        mCleanModeDelegate.SetRVC(this);
        mRvcOperationalStateDelegate.SetRVC(this);
        mRunModeDelegate.SetRVC(this);
        mServiceAreaDelegate.SetRVC(this);

        mRunModeInstance.UpdateCurrentMode(RvcRunMode::ModeIdle);
        mRvcOperationalStateInstance.SetOperationalState(to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
    }

    CHIP_ERROR Init();
    void Shutdown();

    // ////////////////////////////////////////////////////////////////////////
    // These are messages from a Matter client to update the state of Valetudo.
    // ////////////////////////////////////////////////////////////////////////

    void HandleCleanMode(uint8_t, ModeBase::Commands::ChangeToModeResponse::Type &);
    void HandleRunMode(uint8_t, ModeBase::Commands::ChangeToModeResponse::Type &);
    void HandleGoHome(OperationalState::GenericOperationalError &);
    void HandlePause(OperationalState::GenericOperationalError &);
    void HandleResume(OperationalState::GenericOperationalError &);
    void HandleIdentify();

    // ////////////////////////////////////////////////////////////////////////
    // These are out-of-band messages from Valetudo to update the state of the
    // Matter server.
    // ////////////////////////////////////////////////////////////////////////

    void UpdateBatteryLevel(uint8_t);
    void UpdateCleanMode(uint8_t);
    void UpdateOperationalState(uint8_t);
    void UpdateOperationalError();
    void UpdateSupportedAreas(const std::map<uint32_t, std::string> &);

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
};

}