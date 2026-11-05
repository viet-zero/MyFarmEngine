#include "Crop.h"
#include <array>
namespace MyGame
{
    namespace
    {
        constexpr std::array<CropDefinition,3> Crops{{
            {CropId::Turnip,ItemId::TurnipSeed,ItemId::Turnip,5.0,6.0},
            {CropId::Carrot,ItemId::CarrotSeed,ItemId::Carrot,8.0,6.0},
            {CropId::Potato,ItemId::PotatoSeed,ItemId::Potato,12.0,6.0}}};
    }
    const CropDefinition* FindCrop(CropId id)
    {for(const auto& crop:Crops) if(crop.id==id) return &crop;return nullptr;}
    const CropDefinition* CropFromSeed(ItemId seed)
    {for(const auto& crop:Crops) if(crop.seed==seed) return &crop;return nullptr;}
}
