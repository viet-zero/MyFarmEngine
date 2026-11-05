#pragma once
#include "Gameplay/Inventory/Inventory.h"
#include <cstdint>
namespace MyGame
{
    enum class CropId : std::uint32_t {None=0,Turnip=1,Carrot=2,Potato=3};
    enum class CropState {Untilled,Empty,Planted,Growing,Ready};
    struct CropDefinition {CropId id;ItemId seed,produce;double growthSeconds,waterSeconds;};
    const CropDefinition* FindCrop(CropId id);
    const CropDefinition* CropFromSeed(ItemId seed);
}
