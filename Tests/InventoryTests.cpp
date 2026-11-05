#include "Gameplay/Systems/InteractionSystem.h"
#include <iostream>
#include <limits>
namespace {int failures=0;void Check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';++failures;}}}
int main()
{
    using namespace MyGame;
    const auto unknown=static_cast<ItemId>(999999);
    Check(FindItem(ItemId::TurnipSeed)->key=="seed.turnip" && !FindItem(unknown),"stable item lookup failed");
    Inventory bag(2);
    Check(bag.TryAdd(ItemId::Apple,25) && bag.UsedSlots()==2 && bag.Count(ItemId::Apple)==25,
          "stack splitting failed");
    const auto saved=bag;
    Check(!bag.TryAdd(ItemId::Turnip,1) && bag==saved,"full bag partially added different item");
    Check(bag.TryAdd(ItemId::Apple,15) && bag.Count(ItemId::Apple)==40,"existing stack capacity ignored");
    const auto full=bag;
    Check(!bag.TryAdd(ItemId::Apple,1) && bag==full,"overflow add mutated full bag");
    Check(!bag.TryRemove(ItemId::Apple,41) && bag==full,"failed remove partially charged inventory");
    Check(bag.TryRemove(ItemId::Apple,21) && bag.Count(ItemId::Apple)==19 && bag.UsedSlots()==1,
          "remove across stacks did not free slot");
    Check(bag.TryAdd(ItemId::Turnip,1) && bag.Count(ItemId::Apple)==19 && bag.Count(ItemId::Turnip)==1,
          "typed items merged");
    const auto valid=bag;
    for(const int quantity:{-1,0,std::numeric_limits<int>::max()})
        Check(!bag.TryAdd(ItemId::Apple,quantity) && !bag.TryRemove(ItemId::Apple,quantity) && bag==valid,
              "invalid/huge quantity mutated inventory");
    Check(!bag.TryAdd(unknown,1) && !bag.TryAdd(ItemId::None,1) && bag==valid,"unknown item accepted");
    Inventory tools(2);Check(tools.TryAdd(ItemId::Axe,2) && tools.UsedSlots()==2 && !tools.TryAdd(ItemId::Axe,1),
                             "tools stacked or exceeded capacity");
    Check(Inventory(-1).SlotCount()==0 && Inventory(1000).SlotCount()==Inventory::MaxSlots,"slot bounds invalid");
    const auto starter=Inventory::Starter();
    Check(starter.TotalCount()==5 && starter.ResourceCount()==3 && starter.UsedSlots()==3 &&
          starter.Count(ItemId::TurnipSeed)==3 && starter.Count(ItemId::Axe)==1 && starter.Count(ItemId::WateringCan)==1,
          "starter loadout incorrect");

    InteractionActor actor;actor.facing={1,0};EventBus events;InteractionAction action;
    std::vector<InteractionTarget> targets{{"plot",TargetKind::Crop,{60,0}}};
    actor.inventory.TryRemove(ItemId::TurnipSeed,3);actor.inventory.TryAdd(ItemId::Apple,3);
    Check(InteractionSystem::Start(actor,&targets[0],2,action)==InteractionResult::NoSeeds && !targets[0].planted,
          "apples counted as seeds");
    actor.inventory.TryAdd(ItemId::TurnipSeed,1);actor.hotbar[0]=ItemId::TurnipSeed;
    Check(InteractionSystem::Start(actor,&targets[0],0,action)==InteractionResult::Success &&
          actor.inventory.Count(ItemId::TurnipSeed)==1,"hotbar used atlas index or Start charged seeds");
    Check(InteractionSystem::Impact(actor,action,targets,events)==InteractionResult::Success &&
          actor.inventory.Count(ItemId::TurnipSeed)==0 && actor.inventory.Count(ItemId::Apple)==3 && targets[0].planted,
          "impact removed wrong item");
    InteractionSystem::Cancel(actor,action);actor.cooldown=0;events.Clear();
    targets[0].ready=true;
    Check(InteractionSystem::Use(actor,&targets[0],0,events)==InteractionResult::Success &&
          actor.inventory.Count(ItemId::Turnip)==1 && actor.inventory.Count(ItemId::TurnipSeed)==0,
          "harvest added seeds instead of crop");

    actor=InteractionActor{};actor.facing={1,0};targets={{"rock",TargetKind::Object,{60,0}}};action={};events.Clear();
    actor.inventory.TryRemove(ItemId::Axe,1);
    Check(InteractionSystem::Start(actor,&targets[0],1,action)==InteractionResult::WrongTool,"unowned axe accepted");
    actor.inventory.TryAdd(ItemId::Axe,1);
    Check(InteractionSystem::Start(actor,&targets[0],1,action)==InteractionResult::Success,"owned tool failed");
    actor.inventory.TryRemove(ItemId::Axe,1);
    Check(InteractionSystem::Impact(actor,action,targets,events)==InteractionResult::WrongTool &&
          targets[0].health==2 && events.Pending()==0 && actor.stamina==20,"impact did not revalidate tool ownership");

    actor=InteractionActor{};actor.facing={1,0};targets={{"plot",TargetKind::Crop,{60,0}}};action={};events.Clear();
    InteractionSystem::Start(actor,&targets[0],2,action);actor.hotbar[2]=ItemId::Apple;
    const auto before=actor.inventory;
    Check(InteractionSystem::Impact(actor,action,targets,events)==InteractionResult::TargetChanged &&
          actor.inventory==before && !targets[0].planted && events.Pending()==0,"hotbar rebind changed reserved action");

    actor=InteractionActor{};actor.facing={1,0};actor.inventory=Inventory(1);actor.inventory.TryAdd(ItemId::Apple,19);
    InteractionTarget fruit{"tree",TargetKind::FruitTree,{60,0}};fruit.stock=2;events.Clear();
    Check(InteractionSystem::Use(actor,&fruit,0,events)==InteractionResult::Success && actor.inventory.Count(ItemId::Apple)==20,
          "harvest should top up stack with no free slot");
    actor.cooldown=0;const auto stamina=actor.stamina;const auto pending=events.Pending();
    Check(InteractionSystem::Use(actor,&fruit,0,events)==InteractionResult::InventoryFull && fruit.stock==1 &&
          actor.stamina==stamina && events.Pending()==pending,"full inventory changed fruit/stamina/events");
    actor=InteractionActor{};actor.facing={1,0};targets={{"pickup",TargetKind::Pickup,{60,0}}};action={};events.Clear();
    actor.hotbar[0]=unknown;
    Check(InteractionSystem::Start(actor,&targets[0],0,action)==InteractionResult::WrongTool,"unknown hotbar item accepted");
    actor.hotbar=DefaultHotbar;
    targets[0].item=ItemId::Wood;
    Check(InteractionSystem::Start(actor,&targets[0],-1,action)==InteractionResult::WrongTool &&
          InteractionSystem::Start(actor,&targets[0],4,action)==InteractionResult::WrongTool,"invalid hotbar slot accepted");
    Check(InteractionSystem::Start(actor,&targets[0],0,action)==InteractionResult::Success,"typed pickup start failed");
    targets[0].item=ItemId::Apple;const auto oldBag=actor.inventory;
    Check(InteractionSystem::Impact(actor,action,targets,events)==InteractionResult::TargetChanged &&
          actor.inventory==oldBag && targets[0].active && events.Pending()==0,"reward changed during pending pickup");
    InteractionSystem::Cancel(actor,action);targets[0].item=ItemId::Wood;
    Check(InteractionSystem::Use(actor,&targets[0],0,events)==InteractionResult::Success &&
          actor.inventory.Count(ItemId::Wood)==1 && actor.inventory.Count(ItemId::Apple)==0,"pickup ignored declared item ID");
    return failures?1:0;
}
