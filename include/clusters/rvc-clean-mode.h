#pragma once

#include <app/clusters/mode-base-server/mode-base-server.h>

namespace chip::app::Clusters
{
class RVC;
}

namespace chip::app::Clusters::RvcCleanMode
{

const uint8_t ModeMop = 0;
const uint8_t ModeVacuum = 1;
const uint8_t ModeVacuumAndMop = 2;

class RvcCleanModeDelegate : public ModeBase::Delegate
{
  public:
    CHIP_ERROR Init() override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;

    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags) override;

    void HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    void SetRVC(RVC * rvc);

  private:
    using ModeTagStructType = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType ModeTagsMop[1] = {{.value = to_underlying(ModeTag::kMop)}};
    ModeTagStructType ModeTagsVacuum[1] = {{.value = to_underlying(ModeTag::kVacuum)}};
    ModeTagStructType ModeTagsVacuumAndMop[2] = {{.value = to_underlying(ModeTag::kVacuum)},
                                                 {.value = to_underlying(ModeTag::kMop)}};

    const detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        detail::Structs::ModeOptionStruct::Type{.label = CharSpan::fromCharString("Mop"),
                                                .mode = ModeMop,
                                                .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsMop)},
        detail::Structs::ModeOptionStruct::Type{.label = CharSpan::fromCharString("Vacuum"),
                                                .mode = ModeVacuum,
                                                .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsVacuum)},
        detail::Structs::ModeOptionStruct::Type{.label = CharSpan::fromCharString("Vacuum and Mop"),
                                                .mode = ModeVacuumAndMop,
                                                .modeTags =
                                                    DataModel::List<const ModeTagStructType>(ModeTagsVacuumAndMop)},
    };

    RVC * mRvc = nullptr;
};

}
