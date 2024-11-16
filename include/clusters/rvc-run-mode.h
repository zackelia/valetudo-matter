#pragma once

#include <app/clusters/mode-base-server/mode-base-server.h>

namespace chip::app::Clusters::RvcRunMode
{

const uint8_t ModeIdle     = 0;
const uint8_t ModeCleaning = 1;
const uint8_t ModeMapping  = 2;

class RvcRunModeDelegate : public ModeBase::Delegate
{
    CHIP_ERROR Init() override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;

    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags) override;

    void HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

private:
    using ModeTagStructType               = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType ModeTagsIdle[1]     = { { .value = to_underlying(ModeTag::kIdle) } };
    ModeTagStructType ModeTagsCleaning[1] = { { .value = to_underlying(ModeTag::kCleaning) } };
    ModeTagStructType ModeTagsMapping[1]  = { { .value = to_underlying(ModeTag::kMapping) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Idle"),
                                                 .mode     = ModeIdle,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsIdle) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Cleaning"),
                                                 .mode     = ModeCleaning,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsCleaning) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Mapping"),
                                                 .mode     = ModeMapping,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsMapping) },
    };
};

}