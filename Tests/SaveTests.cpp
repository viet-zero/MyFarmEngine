#include "Gameplay/Save/Save.h"
#include <iostream>

namespace
{
    MyGame::FarmSave Sample()
    {
        using namespace MyGame;
        FarmSave state;
        state.position={3000,1800};
        state.money=37;
        state.selectedSlot=2;
        state.seed=ItemId::CarrotSeed;
        state.bag.TryAdd(ItemId::CarrotSeed,21);
        state.home=HomeStorage::PlayerGarden();
        state.home.items.TryAdd(ItemId::Apple,5);
        InteractionTarget crop{"crop.plot.01",TargetKind::Crop};
        crop.planted=true;
        crop.crop=CropId::Carrot;
        crop.age=4.25;
        crop.moisture=1.75;
        crop.revision=7;
        state.targets.push_back(crop);
        return state;
    }
}

int main(int argc,char** argv)
{
    using namespace MyGame;
    const auto expected=EncodeSave(Sample());
    const auto restored=DecodeSave(expected);
    if(!restored || EncodeSave(*restored)!=expected ||
       DecodeSave(expected+"extra") || DecodeSave(expected.substr(0,expected.size()/2)) ||
       DecodeSave("MYFARM 99 "+expected.substr(9)))
    {
        std::cerr<<"Save codec validation failed\n";
        return 1;
    }
    auto portalSample=Sample();portalSample.portals={{"gate.town.01",true},{"door.farm.home",false}};
    const auto portalPayload=EncodeSave(portalSample);
    const auto portalDecoded=DecodeSave(portalPayload);
    if(!portalDecoded || portalDecoded->portals.size()!=2 || !portalDecoded->portals[0].open ||
       portalDecoded->portals[1].open || EncodeSave(*portalDecoded)!=portalPayload) return 1;
    portalSample.portals.push_back(portalSample.portals[0]);
    if(DecodeSave(EncodeSave(portalSample))) return 1;
    portalSample.portals.pop_back();
    if(argc==1) return 0;
    const std::string_view mode=argv[1];
    if(mode!="write" && mode!="read") return 1;
    const auto result=mode=="write"?SaveManager::Write(portalSample,true):SaveManager::Read(true);
    std::cout<<result.message<<'\n';
    if(!result.success || (mode=="read" && (!result.state || EncodeSave(*result.state)!=portalPayload))) return 1;
    return 0;
}
