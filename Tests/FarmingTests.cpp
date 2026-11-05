#include "Gameplay/Systems/InteractionSystem.h"
#include "Gameplay/Graphics/HotbarVisibility.h"
#include "Core/Time.h"
#include <cmath>
#include <iostream>
#include <limits>
namespace
{
    int failures=0;
    void Check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';++failures;}}
    MyGame::InteractionActor Actor(){MyGame::InteractionActor actor;actor.facing={1,0};return actor;}
    MyGame::InteractionTarget Plot(){return {"plot",MyGame::TargetKind::Crop,{60,0}};}
    void Home(MyGame::InteractionActor& actor)
    {actor.home.enabled=true;actor.home.ownerId=1;actor.home.origin={-100,-100};actor.home.size={300,200};}
    void Ripe(MyGame::InteractionTarget& plot)
    {plot.planted=true;plot.ready=true;plot.crop=MyGame::CropId::Turnip;plot.age=5;}
}
int main()
{
    using namespace MyGame;
    Check(!FindCrop(CropId::None) && !CropFromSeed(ItemId::Apple),"invalid crop/seed accepted");
    for(const auto id:{CropId::Turnip,CropId::Carrot,CropId::Potato})
    {
        auto actor=Actor();const auto* definition=FindCrop(id);EventBus events;
        std::vector<InteractionTarget> plots{Plot()};auto& plot=plots[0];
        actor.hotbar[2]=definition->seed;actor.inventory.TryAdd(definition->seed,2);
        const int seeds=actor.inventory.Count(definition->seed);
        Check(plot.State()==CropState::Empty,"prepared plot not empty");
        plot.tilled=false;
        Check(InteractionSystem::Use(actor,&plot,2,events)==InteractionResult::NeedsTilling,"unprepared soil planted");
        plot.tilled=true;
        Check(InteractionSystem::Use(actor,&plot,2,events)==InteractionResult::Success && plot.crop==id &&
              plot.State()==CropState::Planted && actor.inventory.Count(definition->seed)==seeds-1,"typed planting failed");
        for(int i=0;i<100;++i)InteractionSystem::Tick(actor,plots,0.1f);
        Check(plot.age==0 && plot.moisture==0 && !plot.ready,"dry crop grew");
        Check(InteractionSystem::Use(actor,&plot,2,events)==InteractionResult::NotReady,"unready crop harvested");
        actor.inventory.TryRemove(ItemId::WateringCan,1);
        Check(InteractionSystem::Use(actor,&plot,0,events)==InteractionResult::WrongTool,"unowned can watered");
        actor.inventory.TryAdd(ItemId::WateringCan,1);
        Check(InteractionSystem::Use(actor,&plot,0,events)==InteractionResult::Success && plot.moisture==definition->waterSeconds,
              "watering did not set moisture");
        const auto waterEvents=events.Take();
        Check(!waterEvents.empty() && waterEvents.back().kind==GameEventKind::CropWatered,"watering event missing");
        actor.cooldown=0;
        Check(InteractionSystem::Use(actor,&plot,0,events)==InteractionResult::AlreadyWatered,"full moisture charged again");
        for(int i=0;i<60;++i)InteractionSystem::Tick(actor,plots,0.1f);
        if(id!=CropId::Turnip)
        {
            const double dryAge=plot.age;
            for(int i=0;i<30;++i)InteractionSystem::Tick(actor,plots,0.1f);
            Check(!plot.ready && std::abs(plot.age-dryAge)<1e-6,"crop grew after moisture ran out");
            Check(InteractionSystem::Use(actor,&plot,0,events)==InteractionResult::Success,"rewater failed");
            for(int i=0;i<65;++i)InteractionSystem::Tick(actor,plots,0.1f);
        }
        Check(plot.State()==CropState::Ready && plot.age==definition->growthSeconds,"growth duration boundary wrong");
        const auto revision=plot.revision;
        Check(InteractionSystem::Use(actor,&plot,0,events)==InteractionResult::Success && plot.active && !plot.planted &&
              plot.State()==CropState::Empty && plot.moisture==0 && plot.revision>revision &&
              actor.inventory.Count(definition->produce)==1,"harvest did not return reusable plot/typed produce");
        actor.cooldown=0;
        Check(InteractionSystem::Use(actor,&plot,0,events)==InteractionResult::WrongTool &&
              actor.inventory.Count(definition->produce)==1,"repeated harvest duplicated produce");
        Check(InteractionSystem::Use(actor,&plot,2,events)==InteractionResult::Success && plot.crop==id &&
              plot.age==0 && plot.moisture==0 && actor.inventory.Count(definition->seed)==seeds-2,"replant failed/reset old growth");
        const auto age=plot.age;InteractionSystem::Tick(actor,plots,std::numeric_limits<float>::quiet_NaN());
        Check(plot.age==age,"invalid dt changed growth");
    }

    auto actor=Actor();Home(actor);std::vector<InteractionTarget> plots{Plot()};Ripe(plots[0]);
    EventBus events;InteractionAction action;
    actor.inventory=Inventory(1);actor.inventory.TryAdd(ItemId::Cake,10);const auto fullBag=actor.inventory;
    Check(InteractionSystem::Start(actor,&plots[0],0,action)==InteractionResult::Success && action.toStorage,"home harvest selected bag capacity");
    Check(actor.home.items.Count(ItemId::Turnip)==0,"Start deposited early");
    auto stale=action;
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::Success && actor.inventory==fullBag &&
          actor.home.items.Count(ItemId::Turnip)==1 && !plots[0].planted,"home harvest did not bypass full backpack safely");
    Check(InteractionSystem::Impact(actor,stale,plots,events)==InteractionResult::Busy && actor.home.items.Count(ItemId::Turnip)==1,
          "copied action duplicated warehouse deposit");
    InteractionSystem::Cancel(actor,action);actor.cooldown=0;Ripe(plots[0]);action={};events.Clear();
    actor.home.items=Inventory(1);actor.home.items.TryAdd(ItemId::Cake,10);
    const auto oldStorage=actor.home.items;const auto stamina=actor.stamina;
    Check(InteractionSystem::Start(actor,&plots[0],0,action)==InteractionResult::StorageFull &&
          actor.inventory==fullBag && actor.home.items==oldStorage && plots[0].ready && actor.stamina==stamina,
          "full warehouse lost crop or silently fell back to bag");
    actor.inventory=Inventory::Starter();
    Check(InteractionSystem::Start(actor,&plots[0],0,action)==InteractionResult::StorageFull,"full warehouse fell back to nonfull bag");

    actor=Actor();Home(actor);Ripe(plots[0]);action={};events.Clear();
    Check(InteractionSystem::Start(actor,&plots[0],0,action)==InteractionResult::Success,"warehouse capacity recheck setup failed");
    actor.home.items=Inventory(1);actor.home.items.TryAdd(ItemId::Cake,10);const auto bagBefore=actor.inventory;
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::StorageFull && plots[0].ready &&
          actor.inventory==bagBefore && events.Pending()==0 && actor.stamina==20,"warehouse filled before impact partially committed");

    for(const int scenario:{0,1,2,3})
    {
        actor=Actor();Home(actor);actor.position={-1,0};plots={Plot()};Ripe(plots[0]);action={};events.Clear();
        if(scenario==0) actor.ownerId=2;
        if(scenario==1) actor.home.enabled=false;
        if(scenario==2){actor.home.origin={0,-100};actor.home.size={200,200};} // actor just outside, target inside
        if(scenario==3){actor.home.origin={-100,-100};actor.home.size={150,200};} // target outside, actor inside
        Check(InteractionSystem::Use(actor,&plots[0],0,events)==InteractionResult::Success &&
              actor.inventory.Count(ItemId::Turnip)==1 && actor.home.items.Count(ItemId::Turnip)==0,
              "foreign/disabled/outside property routed to warehouse");
    }
    HomeStorage boundary;boundary.origin={0,0};boundary.size={100,100};
    Check(boundary.Contains({0,0}) && boundary.Contains({99.99f,99.99f}) && !boundary.Contains({100,1}) &&
          !boundary.Contains({1,100}) && !boundary.Contains({std::numeric_limits<float>::infinity(),0}),"property boundaries unsafe");

    actor=Actor();Home(actor);plots={Plot()};Ripe(plots[0]);action={};events.Clear();
    InteractionSystem::Start(actor,&plots[0],0,action);actor.home.ownerId=2;
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::TargetChanged && plots[0].ready && events.Pending()==0,
          "property ownership changed during action");
    actor=Actor();Home(actor);plots={Plot()};Ripe(plots[0]);action={};events.Clear();
    InteractionSystem::Start(actor,&plots[0],0,action);++plots[0].revision;
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::TargetChanged && actor.home.items.TotalCount()==0,
          "old action harvested a new plot generation");
    actor=Actor();Home(actor);plots={Plot()};Ripe(plots[0]);action={};events.Clear();
    InteractionSystem::Start(actor,&plots[0],0,action);InteractionSystem::Cancel(actor,action);
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::Busy && actor.home.items.TotalCount()==0 && plots[0].ready,
          "cancelled harvest deposited");
    actor=Actor();Home(actor);plots={Plot()};Ripe(plots[0]);action={};events.Clear();
    InteractionSystem::Start(actor,&plots[0],0,action);
    for(std::size_t i=0;i<EventBus::MaxPending;++i)events.Publish({GameEventKind::ToolUsed,"fill",{0,0},1});
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::EventQueueFull &&
          actor.home.items.TotalCount()==0 && plots[0].ready && actor.stamina==20,"queue failure changed warehouse/crop");

    actor=Actor();Home(actor);actor.home.origin={0,-100};actor.home.size={200,200};
    plots={Plot()};Ripe(plots[0]);action={};events.Clear();InteractionSystem::Start(actor,&plots[0],0,action);
    actor.position={-1,0};
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::TargetChanged && plots[0].ready &&
          actor.home.items.TotalCount()==0 && actor.inventory.Count(ItemId::Turnip)==0,"crossing home boundary silently rerouted pending harvest");

    actor=Actor();plots={Plot()};action={};events.Clear();InteractionSystem::Use(actor,&plots[0],2,events);
    actor.cooldown=0;events.Clear();const auto beforeWater=actor.stamina;
    Check(InteractionSystem::Start(actor,&plots[0],0,action)==InteractionResult::Success && action.operation==FarmOperation::Water,
          "water transaction start failed");
    InteractionSystem::Cancel(actor,action);
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::Busy && plots[0].moisture==0 &&
          actor.stamina==beforeWater && events.Pending()==0,"cancelled watering spent resources or wetted plot");
    InteractionSystem::Start(actor,&plots[0],0,action);actor.inventory.TryRemove(ItemId::WateringCan,1);
    Check(InteractionSystem::Impact(actor,action,plots,events)==InteractionResult::WrongTool && plots[0].moisture==0 &&
          actor.stamina==beforeWater && events.Pending()==0,"missing can at impact still watered plot");
    plots[0].moisture=0.025;const auto ageBefore=plots[0].age;
    InteractionSystem::Tick(actor,plots,0.1f);
    Check(std::abs(plots[0].age-ageBefore-0.025)<1e-8 && plots[0].moisture==0,"partial wet tick grew for entire frame");

    actor=Actor();Home(actor);events.Clear();InteractionTarget pickup{"loose",TargetKind::Pickup,{60,0}};
    Check(InteractionSystem::Use(actor,&pickup,0,events)==InteractionResult::Success && actor.inventory.Count(ItemId::Apple)==1 &&
          actor.home.items.TotalCount()==0,"loose pickup unexpectedly auto-stored");

    struct TraceResult {double age,moisture;CropId crop;int seed,stored;std::uint64_t ticks;};
    const auto trace=[&](int hz)
    {
        auto farmer=Actor();Home(farmer);farmer.hotbar[2]=ItemId::CarrotSeed;farmer.inventory.TryAdd(ItemId::CarrotSeed,2);
        std::vector<InteractionTarget> garden{Plot()};EventBus bus;MyEngine::FixedStepClock clock;
        for(int frame=0;frame<hz*14;++frame)
        {
            if(frame==0 || frame==hz*12)Check(InteractionSystem::Use(farmer,&garden[0],2,bus)==InteractionResult::Success,"trace plant failed");
            if(frame==hz || frame==hz*8)Check(InteractionSystem::Use(farmer,&garden[0],0,bus)==InteractionResult::Success,"trace water failed");
            if(frame==hz*11)Check(InteractionSystem::Use(farmer,&garden[0],0,bus)==InteractionResult::Success,"trace harvest failed");
            const auto plan=clock.Advance(1.0/hz);
            for(int i=0;i<plan.steps;++i)InteractionSystem::Tick(farmer,garden,static_cast<float>(MyEngine::FixedStepClock::StepSeconds));
            bus.Clear();
        }
        return TraceResult{garden[0].age,garden[0].moisture,garden[0].crop,farmer.inventory.Count(ItemId::CarrotSeed),
                           farmer.home.items.Count(ItemId::Carrot),clock.TotalSteps()};
    };
    const auto reference=trace(60);
    Check(reference.crop==CropId::Carrot && reference.age==0 && reference.moisture==0 && reference.seed==0 &&
          reference.stored==1 && reference.ticks==840,"trace did not harvest and replant dry plot");
    for(int hz:{30,144})
    {
        const auto other=trace(hz);
        Check(other.age==reference.age && other.moisture==reference.moisture && other.crop==reference.crop &&
              other.seed==reference.seed && other.stored==reference.stored && other.ticks==reference.ticks,"render cadence changed crop/storage state");
    }
    HotbarVisibility hud;
    Check(hud.Alpha()==1,"hotbar not initially visible");hud.Update(3.7,false);
    Check(hud.Alpha()>0 && hud.Alpha()<1,"hotbar did not fade");hud.Update(1,false);
    Check(hud.Alpha()==0,"idle hotbar not hidden");hud.Update(0,true);Check(hud.Alpha()==1,"selection did not reveal four-slot hotbar");
    hud.Update(std::numeric_limits<double>::quiet_NaN(),false);Check(hud.Alpha()==1,"invalid dt corrupted UI timer");
    for(int hz:{30,60,144}){HotbarVisibility h;for(int i=0;i<hz*5;++i)h.Update(1.0/hz,false);Check(h.Alpha()==0,"HUD hide cadence mismatch");}
    return failures?1:0;
}
