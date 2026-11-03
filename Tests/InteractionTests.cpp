#include "Gameplay/Systems/InteractionSystem.h"
#include <iostream>
#include <limits>

int main()
{
    using namespace MyGame;
    int failures=0;
    const auto check=[&](bool ok,const char* label){if(!ok){std::cerr<<label<<'\n';++failures;}};
    EventBus events;InteractionActor actor;actor.position={0,0};actor.facing={1,0};
    std::vector<InteractionTarget> targets={{"rock",TargetKind::Object,{60,0},2},
                                            {"crop",TargetKind::Crop,{50,10}},
                                            {"seed",TargetKind::Pickup,{40,-10}}};
    check(InteractionSystem::Use(actor,&targets[0],0,events)==InteractionResult::WrongTool,"wrong tool changed object");
    check(targets[0].health==2 && events.Pending()==0,"denied action mutated state");
    actor.uiCapturesInput=true;
    check(InteractionSystem::Use(actor,&targets[0],1,events)==InteractionResult::Busy,"UI input leaked");
    actor.uiCapturesInput=false;actor.facing={-1,0};
    check(InteractionSystem::Use(actor,&targets[0],1,events)==InteractionResult::WrongFacing,"backward hit accepted");
    actor.facing={1,0};actor.position={-200,0};
    check(InteractionSystem::Use(actor,&targets[0],1,events)==InteractionResult::TooFar,"far hit accepted");
    actor.position={0,0};actor.stamina=1;
    check(InteractionSystem::Use(actor,&targets[0],1,events)==InteractionResult::NoStamina,"zero stamina hit accepted");
    actor.stamina=20;
    check(InteractionSystem::Use(actor,&targets[0],1,events)==InteractionResult::Success,"valid hit failed");
    check(targets[0].health==1 && actor.stamina==18 && events.Pending()==2,"hit state/events");
    check(InteractionSystem::Use(actor,&targets[0],1,events)==InteractionResult::Cooldown,"cooldown bypassed");
    InteractionSystem::Tick(actor,targets,std::numeric_limits<float>::quiet_NaN());
    check(actor.cooldown>0,"NaN dt cleared cooldown");
    for(int i=0;i<4;++i) InteractionSystem::Tick(actor,targets,0.1f);
    check(InteractionSystem::Use(actor,&targets[0],1,events)==InteractionResult::Success &&
          !targets[0].active,"object not destroyed after second hit");
    check(InteractionSystem::Use(actor,&targets[0],1,events)==InteractionResult::Inactive,"destroyed object interactive");
    for(int i=0;i<4;++i) InteractionSystem::Tick(actor,targets,0.1f);
    const int beforeSeeds=actor.inventoryCount;
    check(InteractionSystem::Use(actor,&targets[1],2,events)==InteractionResult::Success &&
          targets[1].planted && actor.inventoryCount==beforeSeeds-1,"crop planting failed");
    for(int i=0;i<4;++i) InteractionSystem::Tick(actor,targets,0.1f);
    check(InteractionSystem::Use(actor,&targets[1],2,events)==InteractionResult::NotReady,"early harvest accepted");
    for(int i=0;i<50;++i) InteractionSystem::Tick(actor,targets,0.1f);
    check(targets[1].ready,"crop did not mature");
    actor.inventoryCount=actor.inventoryCapacity;
    check(InteractionSystem::Use(actor,&targets[1],2,events)==InteractionResult::InventoryFull,"full bag harvested crop");
    actor.inventoryCount=3;
    check(InteractionSystem::Use(actor,&targets[1],2,events)==InteractionResult::Success &&
          !targets[1].active && actor.inventoryCount==4,"harvest failed");
    for(int i=0;i<4;++i) InteractionSystem::Tick(actor,targets,0.1f);
    check(InteractionSystem::Use(actor,&targets[2],0,events)==InteractionResult::Success,"pickup failed");
    check(actor.inventoryCount==5 && !targets[2].active,"pickup state failed");
    check(InteractionSystem::Nearest(actor,targets)==nullptr,"inactive target chosen");
    InteractionActor harvester;harvester.position={0,0};harvester.facing={1,0};
    InteractionTarget fruit{"tree.apple.01",TargetKind::FruitTree,{60,0}};
    fruit.stock=fruit.maxStock=2;fruit.regrowSeconds=0.3f;
    EventBus fruitEvents;
    check(InteractionSystem::Use(harvester,&fruit,0,fruitEvents)==InteractionResult::Success &&
          fruit.stock==1 && harvester.inventoryCount==4,"first apple harvest failed");
    check(fruitEvents.Pending()==2,"fruit event missing");
    for(int i=0;i<4;++i) InteractionSystem::Tick(harvester,targets,0.1f);
    check(InteractionSystem::Use(harvester,&fruit,0,fruitEvents)==InteractionResult::Success &&
          fruit.stock==0 && fruit.active,"second harvest removed tree");
    for(int i=0;i<4;++i) InteractionSystem::Tick(harvester,targets,0.1f);
    check(InteractionSystem::Use(harvester,&fruit,0,fruitEvents)==InteractionResult::NotReady,
          "empty tree yielded fruit");
    std::vector<InteractionTarget> forest{fruit};
    for(int i=0;i<4;++i) InteractionSystem::Tick(harvester,forest,0.1f);
    check(forest[0].stock==2,"fruit did not regrow");
    harvester.inventoryCount=harvester.inventoryCapacity;
    check(InteractionSystem::Use(harvester,&forest[0],0,fruitEvents)==InteractionResult::InventoryFull,
          "full inventory harvested fruit");
    return failures?1:0;
}
