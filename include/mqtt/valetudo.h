#pragma once

#include "broker.h"
#include "clusters/rvc-clean-mode.h"
#include <optional>
#include <map>

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
    [[nodiscard]] std::optional<std::map<uint32_t, std::string>> GetSupportedAreas() const { return mSupportedAreas; }
    [[nodiscard]] std::optional<bool> GetDustBinInstalled() const { return mDustBinInstalled; }
    [[nodiscard]] std::optional<bool> GetMopInstalled() const { return mMopInstalled; }
    [[nodiscard]] std::optional<bool> GetWaterTankInstalled() const { return mWaterTankInstalled; }

    CHIP_ERROR Locate();
    CHIP_ERROR SetCleanMode(uint8_t);

    CHIP_ERROR Start(std::optional<std::vector<uint32_t>> = std::nullopt);
    CHIP_ERROR Pause();
    CHIP_ERROR Home();

    void SetRVC(chip::app::Clusters::RVC * rvc);
private:
    Broker mBroker;

    std::string mPrefix;

    std::optional<uint8_t> mBatteryLevel = std::nullopt;
    std::optional<uint8_t> mCleanMode = std::nullopt;
    std::optional<uint8_t> mOperationalState = std::nullopt;
    std::optional<std::map<uint32_t, std::string>> mSupportedAreas = std::nullopt;
    std::optional<bool> mDustBinInstalled = std::nullopt;
    std::optional<bool> mMopInstalled = std::nullopt;
    std::optional<bool> mWaterTankInstalled = std::nullopt;

    CHIP_ERROR Publish(const std::string &, const std::string &);

    void HandleSubscribe();
    void HandlePublish(const std::string &, const std::string &);

    chip::app::Clusters::RVC * mRvc = nullptr;
};

}
