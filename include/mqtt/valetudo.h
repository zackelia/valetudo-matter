#pragma once

#include "app-common/zap-generated/cluster-enums.h"
#include "app/clusters/operational-state-server/operational-state-cluster-objects.h"
#include "broker.h"
#include "clusters/rvc-clean-mode.h"
#include "lib/support/Span.h"
#include <functional>
#include <optional>

namespace MQTT
{
class Valetudo
{
public:
    CHIP_ERROR Init();

    CHIP_ERROR SetBatteryLevelCallback(std::function<void(void)>);
    CHIP_ERROR SetCleanModeCallback(std::function<void(void)>);
    CHIP_ERROR SetStateCallback(std::function<void(void)>);

    // Public getters, only to be called when callbacks say they are ready.
    [[nodiscard]] uint8_t GetBatteryLevel() const { return mBatteryLevel.value(); }
    [[nodiscard]] uint8_t GetCleanMode() const { return mCleanMode.value(); }
    [[nodiscard]] chip::app::Clusters::OperationalState::GenericOperationalState GetState() const { return mState.value(); }

    CHIP_ERROR Locate();
    CHIP_ERROR SetCleanMode(uint8_t);
private:
    Broker mBroker;

    std::string mPrefix;

    std::function<void(void)> mBatteryLevelCallback = nullptr;
    std::optional<uint8_t> mBatteryLevel = std::nullopt;

    std::function<void(void)> mCleanModeCallback = nullptr;
    std::optional<uint8_t> mCleanMode = std::nullopt;

    std::function<void(void)> mStateCallback = nullptr;
    std::optional<chip::app::Clusters::OperationalState::GenericOperationalState> mState;

    CHIP_ERROR Publish(const std::string &, const std::string &);

    void HandleSubscribe();
    void HandlePublish(const std::string &, const std::string &);
};

}
