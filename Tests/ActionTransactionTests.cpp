#include "Gameplay/Systems/InteractionSystem.h"
#include "Animation/Animator.h"
#include <iostream>
#include <limits>
namespace
{
    int failures=0;
    void Check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';++failures;}}
    MyGame::InteractionActor Actor(){MyGame::InteractionActor a;a.facing={1,0};return a;}
    std::vector<MyGame::InteractionTarget> Fruit()
    {
        MyGame::InteractionTarget t{"tree",MyGame::TargetKind::FruitTree,{60,0}};
        t.stock=t.maxStock=3;return {t};
    }
}
int main()
{
    using namespace MyGame;
    auto actor=Actor();auto targets=Fruit();EventBus events;InteractionAction action;
    Check(InteractionSystem::Start(actor,&targets[0],0,action)==InteractionResult::Success,"start failed");
    const auto id=action.id;auto stale=action;
    Check(id!=0 && actor.activeActionId==id && actor.busy && action.phase==ActionPhase::Start,"reservation missing");
    Check(actor.inventory.ResourceCount()==3 && actor.stamina==20 && targets[0].stock==3 && events.Pending()==0,"Start mutated resources/events");
    Check(InteractionSystem::Start(actor,&targets[0],0,action)==InteractionResult::Busy && action.id==id,"retrigger restarted action");
    InteractionSystem::Activate(actor,action);
    targets.reserve(100);targets.push_back({"unrelated",TargetKind::Pickup,{20,0}});
    MyEngine::AnimationClip clip{0,1,5,false};clip.markers={{"IMPACT",0.1},{"COMPLETE",0.2}};
    MyEngine::Animator animator;animator.Play("harvest",clip,1);
    const auto update=[&](float dt)
    {
        animator.Update(dt);
        for(const auto& hit:animator.MarkerHits())
        {
            if(hit.name=="IMPACT") InteractionSystem::Impact(actor,action,targets,events);
            if(hit.name=="COMPLETE") InteractionSystem::Complete(actor,action);
        }
    };
    update(0.05f);Check(actor.inventory.ResourceCount()==3 && events.Pending()==0,"pre-impact changed model");
    const auto beforeImpact=animator.Save();
    animator.SetPaused(true);update(1);Check(actor.inventory.ResourceCount()==3,"paused timeline committed");
    animator.SetPaused(false);update(0.06f);
    Check(actor.inventory.ResourceCount()==4 && targets[0].stock==2 && action.impactConsumed,"impact did not commit after vector reallocation");
    Check(InteractionSystem::Impact(actor,stale,targets,events)==InteractionResult::Busy && actor.inventory.ResourceCount()==4,"copied/stale action double applied");
    animator.Restore(beforeImpact,clip,1);update(0.06f);
    Check(actor.inventory.ResourceCount()==4 && targets[0].stock==2,"timeline rewind replayed committed impact");
    auto emitted=events.Take();Check(emitted.size()==2 && emitted[0].actionId==id && emitted[1].actionId==id,"FX/gameplay action ID mismatch");
    update(1);Check(action.phase==ActionPhase::Complete && actor.busy && actor.inventory.ResourceCount()==4,"completion skipped or repeated mutation");
    update(1);Check(events.Pending()==0,"finished action replayed events");
    InteractionSystem::TickAction(actor,action,0);Check(action.phase==ActionPhase::Complete,"zero dt advanced recovery");
    InteractionSystem::TickAction(actor,action,0.1f);
    const auto remaining=action.recoveryRemaining;InteractionSystem::Complete(actor,action);
    Check(action.recoveryRemaining==remaining,"duplicate Complete reset recovery");
    InteractionSystem::TickAction(actor,action,0.1f);
    Check(action.phase==ActionPhase::Finished && !actor.busy && actor.activeActionId==0,"recovery did not release owner");
    Check(InteractionSystem::Impact(actor,stale,targets,events)==InteractionResult::Busy,"finished action replayed");

    actor=Actor();targets=Fruit();action={};events.Clear();
    InteractionSystem::Start(actor,&targets[0],0,action);stale=action;InteractionSystem::Cancel(actor,action);
    Check(!actor.busy && action.phase==ActionPhase::Cancelled && actor.inventory.ResourceCount()==3,"pre-impact cancel charged actor");
    Check(InteractionSystem::Impact(actor,stale,targets,events)==InteractionResult::Busy && targets[0].stock==3,"cancelled action replayed");
    InteractionSystem::Start(actor,&targets[0],0,action);Check(action.id>stale.id,"action ID reused");
    InteractionSystem::Impact(actor,action,targets,events);InteractionSystem::Cancel(actor,action);
    Check(actor.inventory.ResourceCount()==4 && targets[0].stock==2 && !actor.busy,"post-impact cancel rolled back committed model");

    const auto denied=[&](auto mutate,InteractionResult expected,bool visible=true)
    {
        auto a=Actor();auto t=Fruit();InteractionAction pending;EventBus bus;
        InteractionSystem::Start(a,&t[0],0,pending);mutate(a,t);
        const auto inventory=a.inventory;const auto stamina=a.stamina;
        Check(InteractionSystem::Impact(a,pending,t,bus,visible)==expected,"impact revalidation result incorrect");
        Check(a.inventory==inventory && a.stamina==stamina && bus.Pending()==0,"denied impact changed resources/events");
        t=Fruit();Check(InteractionSystem::Impact(a,pending,t,bus)==InteractionResult::Busy,"failed impact retried same action ID");
        InteractionSystem::Cancel(a,pending);
    };
    denied([](auto&,auto& t){t.clear();},InteractionResult::NoTarget);
    denied([](auto&,auto& t){t[0].active=false;},InteractionResult::Inactive);
    denied([](auto& a,auto&){a.inventory=Inventory(1);a.inventory.TryAdd(ItemId::Cake,10);},InteractionResult::InventoryFull);
    denied([](auto& a,auto&){a.stamina=1;},InteractionResult::NoStamina);
    denied([](auto& a,auto&){a.position={-200,0};},InteractionResult::TooFar);
    denied([](auto& a,auto&){a.facing={-1,0};},InteractionResult::WrongFacing);
    denied([](auto&,auto& t){t[0].stock=0;},InteractionResult::NotReady);
    denied([](auto&,auto& t){t[0].kind=TargetKind::Pickup;},InteractionResult::TargetChanged);
    denied([](auto&,auto& t){t.push_back(t[0]);},InteractionResult::TargetChanged);
    denied([](auto&,auto&){},InteractionResult::Blocked,false);

    actor=Actor();targets={{"crop",TargetKind::Crop,{60,0}}};action={};events.Clear();
    InteractionSystem::Start(actor,&targets[0],2,action);targets[0].planted=true;targets[0].ready=true;
    Check(InteractionSystem::Impact(actor,action,targets,events)==InteractionResult::TargetChanged && actor.inventory.ResourceCount()==3,"plant action changed into harvest");
    InteractionSystem::Cancel(actor,action);
    actor=Actor();targets={{"crop",TargetKind::Crop,{60,0}}};action={};events.Clear();
    InteractionSystem::Start(actor,&targets[0],2,action);actor.inventory.TryRemove(ItemId::TurnipSeed,3);
    Check(InteractionSystem::Impact(actor,action,targets,events)==InteractionResult::NoSeeds && !targets[0].planted &&
          events.Pending()==0,"impact consumed missing seeds or planted free crop");
    InteractionSystem::Cancel(actor,action);
    actor=Actor();targets=Fruit();action={};events.Clear();InteractionSystem::Start(actor,&targets[0],0,action);
    for(std::size_t i=0;i<EventBus::MaxPending;++i)events.Publish({GameEventKind::ToolUsed,"fill",{0,0},1});
    Check(InteractionSystem::Impact(actor,action,targets,events)==InteractionResult::EventQueueFull && actor.inventory.ResourceCount()==3 && actor.stamina==20,"full event queue lost sync after mutation");
    InteractionSystem::Cancel(actor,action);
    actor=Actor();action={};actor.nextActionId=std::numeric_limits<std::uint64_t>::max();
    Check(InteractionSystem::Start(actor,&targets[0],0,action)!=InteractionResult::Success && !actor.busy,"action ID overflow accepted");
    return failures?1:0;
}
