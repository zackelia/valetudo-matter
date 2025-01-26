#pragma once

#include "app-common/zap-generated/cluster-enums.h"
#include "app/clusters/operational-state-server/operational-state-cluster-objects.h"
#include "broker.h"
#include "clusters/rvc-clean-mode.h"
#include "lib/support/Span.h"
#include <functional>
#include <optional>

namespace chip::app::Clusters
{
    class RVC;
}

namespace MQTT
{
class Valetudo
{
public:
    CHIP_ERROR Init();

    [[nodiscard]] std::optional<uint8_t> GetBatteryLevel() const { return mBatteryLevel; }
    [[nodiscard]] std::optional<uint8_t> GetCleanMode() const { return mCleanMode; }
    [[nodiscard]] std::optional<uint8_t> GetOperationalState() const { return mOperationalState; }
    [[nodiscard]] std::optional<std::vector<std::string>> GetSupportedAreas() const { return mSupportedAreas; }

    CHIP_ERROR Locate();
    CHIP_ERROR SetCleanMode(uint8_t);

    void SetRVC(chip::app::Clusters::RVC * rvc);
private:
    Broker mBroker;

    std::string mPrefix;

    std::optional<uint8_t> mBatteryLevel = std::nullopt;
    std::optional<uint8_t> mCleanMode = std::nullopt;
    std::optional<uint8_t> mOperationalState = std::nullopt;
    std::optional<std::vector<std::string>> mSupportedAreas = std::nullopt;

    CHIP_ERROR Publish(const std::string &, const std::string &);

    void HandleSubscribe();
    void HandlePublish(const std::string &, const std::string &);

    chip::app::Clusters::RVC * mRvc = nullptr;
};

}
