#include "Core/Window.h"
#include "Input/Input.h"
#include "Resources/AssetPath.h"
#include "Gameplay/World/DemoScene2D.h"
#include "Gameplay/States/GameplayState.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <iostream>
namespace
{
    int failures=0;
    void Check(bool ok,const char* text){if(!ok){std::cerr<<text<<'\n';++failures;}}
    bool Same(glm::vec2 a,glm::vec2 b){return glm::length(a-b)<0.001f;}
}
int main()
{
    using namespace MyGame;
    MyEngine::Window window;
    if(!window.Create(640,360,"Gameplay11 integration",false)) return 1;
    MyEngine::Input::Initialize(window.GetNativeWindow());
    if(std::filesystem::exists(MyEngine::AssetRoot()/"LocalV4"/"sprites.csv"))
    {
        WorldArt art;
        Check(art.Load(MyEngine::AssetRoot()/"LocalV4") && art.Ready(),"staged v4 atlas failed to load");
        for(int direction=0;direction<8;++direction)
            for(int frame=0;frame<2;++frame)
                Check(art.Actor(direction,true,frame)!=nullptr && art.Actor(direction,false,0)!=nullptr,
                      "v4 direction/frame missing");
        for(int season=0;season<4;++season)
            Check(art.Terrain(season).Find("land_255") && art.Terrain(season).Find("water_000"),
                  "v4 seasonal terrain missing");
        if(std::filesystem::exists(MyEngine::AssetRoot()/"Cozy"/"props.csv"))
            Check(art.LoadCozy(MyEngine::AssetRoot()/"Cozy") && art.CozyReady(),"cozy atlases failed to load");
        art.Reset();
        Check(!art.Ready(),"v4 reset left atlas enabled");
    }
    AssetTextures assets;
    Check(assets.Initialize(MyEngine::AssetRoot()),"asset manifest initialization failed");
    VisualScale scales;
    Check(scales.Load(MyEngine::AssetRoot()/"Data"/"VisualScale.csv") && scales.human==112 && scales.tree==336,
          "editable visual scale configuration did not load");
    DemoScene2D scene;
    Check(scene.Initialize(assets,MyEngine::AssetRoot()/"Data"),"scene failed");
    auto alignment=scene.CaptureSave();
    alignment.position={2848,2310};
    Check(scene.RestoreSave(alignment),"front-wall approach start rejected");
    SceneInput approachWall;approachWall.movement={0,-1};
    for(int i=0;i<30;++i) scene.UpdateSimulation(0.1f,approachWall);
    // MoveAndSlide stops within its bounded 8-unit substep of the wall, without penetration.
    std::cout<<"Wall contact y="<<scene.Snapshot().player.y<<'\n';
    Check(scene.Snapshot().player.y>=2272 && scene.Snapshot().player.y<2280 &&
          scene.Snapshot().playerState==VisualState::Idle,"character walked through wall or animated while blocked");
    alignment.position={1990,1640};
    Check(scene.RestoreSave(alignment),"trunk approach start rejected");
    for(int i=0;i<20;++i) scene.UpdateSimulation(0.1f,approachWall);
    std::cout<<"Trunk contact y="<<scene.Snapshot().player.y<<'\n';
    Check(scene.Snapshot().player.y>=1602 && scene.Snapshot().player.y<1610,"character passed through orchard trunk");
    alignment.position={3104,1320};
    Check(scene.RestoreSave(alignment),"north entrance start rejected");
    for(int i=0;i<10;++i) scene.UpdateSimulation(0.1f,approachWall);
    Check(scene.Snapshot().player.y<1240,"character could not pass north opening");
    scene.Reset();
    const auto managerStart=scene.Snapshot().player;
    SceneInput manager;manager.toggleGardenControl=true;manager.selectedItem=2;
    scene.UpdateSimulation(1.0f/60,manager);
    Check(scene.Snapshot().gardenControl,"B did not enable garden control");
    manager={};manager.pointerWorld=glm::vec2{2336,2080};
    scene.CaptureInput(manager);
    for(int tick=0;tick<12;++tick) scene.FixedUpdate(1.0f/60);
    Check(scene.TargetState("crop.home.6")->planted && scene.Bag().Count(ItemId::TurnipSeed)==2,
          "pointer did not select exact remote plot or replayed click");
    manager={};manager.movement={1,0};
    const auto cameraBefore=scene.Presentation(1).camera;
    for(int tick=0;tick<30;++tick) scene.UpdateSimulation(1.0f/60,manager);
    Check(Same(managerStart,scene.Snapshot().player) && !Same(cameraBefore,scene.Presentation(1).camera),
          "manager camera moved avatar or failed to pan");
    manager={};manager.toggleGardenControl=true;scene.UpdateSimulation(1.0f/60,manager);
    Check(!scene.Snapshot().gardenControl,"B did not restore character mode");
    scene.Reset();
    SceneInput sowAtSpawn;
    sowAtSpawn.selectedItem=2;
    sowAtSpawn.interact=true;
    scene.CaptureInput(sowAtSpawn);
    for(int tick=0;tick<12;++tick) scene.FixedUpdate(1.0f/60);
    const auto starterPlot=scene.TargetState("crop.home.1");
    Check(starterPlot && starterPlot->planted && scene.Bag().Count(ItemId::TurnipSeed)==2,
          "starter grid did not accept 3/F without walking");
    Check(scene.TargetState("crop.home.2") && !scene.TargetState("crop.home.2")->planted,
          "adjacent tile was planted instead of the front tile");
    scene.Reset();
    SceneInput faceSide;
    faceSide.movement={1,0};
    faceSide.selectedItem=2;
    faceSide.interact=true;
    scene.UpdateSimulation(0,faceSide);
    Check(scene.Snapshot().actionResult==InteractionResult::NoTarget &&
          scene.Bag().Count(ItemId::TurnipSeed)==3,"interact selected a plot off the facing row");
    scene.Reset();
    auto initial=scene.Snapshot();
    Check(scene.Bag().Count(ItemId::TurnipSeed)==3 && scene.Bag().Count(ItemId::Axe)==1 &&
          scene.Bag().Count(ItemId::Apple)==0,"scene starter typed inventory invalid");
    for(int i=0;i<180;++i) scene.UpdateSimulation(1.0f/60,{});
    auto idle=scene.Snapshot();
    Check(Same(initial.player,idle.player) && Same(initial.npc,idle.npc) && Same(initial.pet,idle.pet),"default scene moved without input/AI/follow");
    Check(idle.playerState==VisualState::Idle && idle.npcState==VisualState::Idle && idle.petState==VisualState::Idle,"default actor not idle");
    SceneInput move;move.movement={1,0};
    for(int i=0;i<35;++i) scene.UpdateSimulation(1.0f/60,move);
    auto walking=scene.Snapshot();
    Check(!Same(idle.player,walking.player) && walking.playerState==VisualState::Walk,"player input did not produce actual walk");
    Check(Same(idle.pet,walking.pet),"disabled pet followed player");
    scene.UpdateSimulation(1.0f/60,{});
    Check(scene.Snapshot().playerState==VisualState::Idle,"player did not stop on input release");
    auto options=scene.Options();options.petFollow=true;scene.SetOptions(options);
    for(int i=0;i<30;++i) scene.UpdateSimulation(1.0f/60,{});
    Check(!Same(walking.pet,scene.Snapshot().pet),"enabled pet did not follow");
    options.petFollow=false;scene.SetOptions(options);const auto stoppedPet=scene.Snapshot().pet;
    for(int i=0;i<30;++i) scene.UpdateSimulation(1.0f/60,move);
    Check(Same(stoppedPet,scene.Snapshot().pet) && scene.Snapshot().petState==VisualState::Idle,"disabling pet did not stop motion/state");
    scene.Reset();options.demoMode=true;scene.SetOptions(options);
    const auto stillNpc=scene.Snapshot().npc;
    for(int i=0;i<180;++i) scene.UpdateSimulation(1.0f/60,{});
    Check(Same(stillNpc,scene.Snapshot().npc) && scene.Snapshot().npcState==VisualState::Idle,"demo NPC autoplay while AI off");
    options.npcAI=true;scene.SetOptions(options);
    for(int i=0;i<30;++i) scene.UpdateSimulation(1.0f/60,{});
    Check(!Same(stillNpc,scene.Snapshot().npc) && scene.Snapshot().npcState==VisualState::Walk,"enabled demo patrol not walking");
    Check(Same(initial.player,scene.Snapshot().player),"NPC update moved player");
    options.npcAI=false;scene.SetOptions(options);const auto npcStopped=scene.Snapshot().npc;
    for(int i=0;i<30;++i) scene.UpdateSimulation(1.0f/60,move);
    Check(Same(npcStopped,scene.Snapshot().npc) && scene.Snapshot().npcState==VisualState::Idle,"NPC did not stop or shared player animation state");
    options.npcAI=true;options.demoMode=false;scene.SetOptions(options);
    for(int i=0;i<30;++i) scene.UpdateSimulation(1.0f/60,{});
    Check(Same(npcStopped,scene.Snapshot().npc),"hidden demo left an active NPC AI");
    scene.Reset();SceneInput debugTeleport;debugTeleport.teleport=7;
    scene.UpdateSimulation(0,debugTeleport);
    Check(Same(scene.Snapshot().player,initial.player),"developer teleport active in normal gameplay");
    options.demoMode=true;options.npcAI=false;scene.SetOptions(options);
    scene.UpdateSimulation(0,debugTeleport);SceneInput wall;wall.movement={0,-1};
    for(int i=0;i<120;++i) scene.UpdateSimulation(1.0f/60,wall);
    const auto blocked=scene.Snapshot();
    for(int i=0;i<10;++i) scene.UpdateSimulation(1.0f/60,wall);
    Check(Same(blocked.player,scene.Snapshot().player) && scene.Snapshot().playerState==VisualState::Idle,"blocked input animated walk without motion");
    const auto beforeInvalid=scene.Snapshot();scene.UpdateSimulation(std::nanf(""),move);
    Check(Same(beforeInvalid.player,scene.Snapshot().player),"invalid dt moved player");

    // Real F -> animation marker -> model/FX path, including pause, cancellation and spam.
    scene.Reset();options.demoMode=true;options.npcAI=false;options.petFollow=false;scene.SetOptions(options);
    SceneInput orchard;orchard.teleport=7;scene.UpdateSimulation(0,orchard);
    SceneInput interact;interact.interact=true;
    scene.UpdateSimulation(0,interact);const auto started=scene.Snapshot();
    Check(started.actionPhase==ActionPhase::Start && started.inventoryCount==3 && started.effects==0 &&
          scene.TargetState("tree.apple.01")->stock==3,"F press changed model before impact");
    SceneInput pause;pause.paused=true;
    scene.UpdateSimulation(10,pause);
    Check(scene.Snapshot().actionPhase==ActionPhase::Start && scene.Snapshot().inventoryCount==3,"paused action progressed");
    scene.UpdateSimulation(0.049f,{});
    SceneInput spam;spam.interact=true;spam.movement={1,0};scene.UpdateSimulation(0.04f,spam);
    Check(scene.Snapshot().actionId==started.actionId && Same(scene.Snapshot().player,started.player) &&
          scene.Snapshot().inventoryCount==3,"spam/movement restarted pending action");
    scene.UpdateSimulation(0.02f,{});const auto impacted=scene.Snapshot();
    Check(scene.Storage().items.Count(ItemId::Apple)==1 && scene.Bag().Count(ItemId::Apple)==0 && scene.Bag().Count(ItemId::TurnipSeed)==3,
          "live fruit impact changed seed quantity instead of adding apple");
    Check(impacted.inventoryCount==3 && impacted.storedItemCount==1 && impacted.fruitHarvested==1 && impacted.effects>0 &&
          scene.TargetState("tree.apple.01")->stock==2,"marker did not apply fruit/model/FX together");
    for(int i=0;i<10;++i)scene.UpdateSimulation(0.1f,{});
    Check(scene.Snapshot().inventoryCount==3 && scene.Snapshot().storedItemCount==1 && scene.TargetState("tree.apple.01")->stock==2 &&
          scene.Snapshot().actionPhase==ActionPhase::Finished,"fruit impact repeated or recovery stuck");
    scene.Reset();scene.UpdateSimulation(0,orchard);scene.UpdateSimulation(0,interact);
    const auto cancelledId=scene.Snapshot().actionId;SceneInput cancel;cancel.cancel=true;
    scene.UpdateSimulation(0.1f,cancel);
    Check(scene.Snapshot().inventoryCount==3 && scene.TargetState("tree.apple.01")->stock==3 &&
          scene.Snapshot().actionPhase==ActionPhase::Cancelled,"C did not cancel before impact");
    scene.UpdateSimulation(0,interact);
    Check(scene.Snapshot().actionId>cancelledId,"cancel/retrigger reused ID");
    SceneInput hurt;hurt.hurt=true;scene.UpdateSimulation(0.1f,hurt);
    Check(scene.Snapshot().inventoryCount==3 && scene.Snapshot().actionPhase==ActionPhase::Cancelled,"hurt failed to cancel transaction");
    const auto preResetId=scene.Snapshot().actionId;scene.Reset();scene.UpdateSimulation(0,orchard);scene.UpdateSimulation(0,interact);
    Check(scene.Snapshot().actionId>preResetId,"scene reset reused stale action ID");
    SceneInput warp;warp.teleport=8;scene.UpdateSimulation(0.1f,warp);
    Check(scene.Snapshot().inventoryCount==3 && scene.Snapshot().actionPhase==ActionPhase::Cancelled,"debug warp allowed ghost impact");

    // Reach the real plot through the open south entrance; no test-only target setters.
    scene.Reset();
    const auto travel=[&](glm::vec2 destination)
    {
        for(int i=0;i<1500;++i)
        {
            const auto delta=destination-scene.Snapshot().player;
            const float distance=glm::length(delta);if(distance<0.5f)return;
            SceneInput walkingInput;walkingInput.movement=delta;
            scene.UpdateSimulation(std::min(0.05f,distance/180.0f),walkingInput);
        }
        Check(false,"integration route blocked");
    };
    travel({2200,2020});travel({3000,2020});travel({3000,1780});
    SceneInput facePlot;facePlot.movement={0,-1};facePlot.selectedItem=2;scene.UpdateSimulation(0,facePlot);
    scene.UpdateSimulation(0,interact);
    Check(scene.Snapshot().inventoryCount==3 && !scene.TargetState("crop.plot.01")->planted,"plant mutated at Start");
    scene.UpdateSimulation(0.1f,{});
    Check(scene.Snapshot().inventoryCount==2 && scene.TargetState("crop.plot.01")->planted,"plant impact missing");
    for(int i=0;i<10;++i)scene.UpdateSimulation(0.1f,{});
    Check(scene.TargetState("crop.plot.01")->age==0,"dry crop grew without water");
    SceneInput water;water.selectedItem=0;water.interact=true;scene.UpdateSimulation(0,water);
    Check(scene.TargetState("crop.plot.01")->moisture==0,"watering mutated before impact");
    scene.UpdateSimulation(0.1f,{});
    Check(scene.TargetState("crop.plot.01")->moisture>0,"watering impact missing");
    const auto wetPlot=scene.TargetState("crop.plot.01");SceneInput pauseCrop;pauseCrop.paused=true;
    scene.UpdateSimulation(10,pauseCrop);
    Check(scene.TargetState("crop.plot.01")->age==wetPlot->age && scene.TargetState("crop.plot.01")->moisture==wetPlot->moisture,
          "paused crop grew or lost moisture");
    for(int i=0;i<65;++i)scene.UpdateSimulation(0.1f,{});
    Check(scene.TargetState("crop.plot.01")->ready,"crop did not grow during simulation");
    scene.UpdateSimulation(0,interact);
    Check(scene.Snapshot().inventoryCount==2 && scene.TargetState("crop.plot.01")->active,"harvest mutated at Start");
    scene.UpdateSimulation(0.1f,{});
    Check(scene.Snapshot().inventoryCount==2 && scene.Snapshot().storedItemCount==1 && scene.TargetState("crop.plot.01")->active && !scene.TargetState("crop.plot.01")->planted,"harvest impact not applied once");
    Check(scene.Bag().Count(ItemId::TurnipSeed)==2 && scene.Bag().Count(ItemId::Turnip)==0 && scene.Storage().items.Count(ItemId::Turnip)==1,
          "live crop did not preserve separate seed/produce quantities");
    for(int i=0;i<6;++i)scene.UpdateSimulation(0.1f,{});
    SceneInput replant;replant.selectedItem=2;replant.interact=true;scene.UpdateSimulation(0,replant);
    Check(scene.Bag().Count(ItemId::TurnipSeed)==2,"replant charged before impact");
    scene.UpdateSimulation(0.1f,{});
    Check(scene.Bag().Count(ItemId::TurnipSeed)==1 && scene.TargetState("crop.plot.01")->planted &&
          scene.TargetState("crop.plot.01")->age==0 && scene.Storage().items.Count(ItemId::Turnip)==1,"live replant reused old growth or lost stored produce");
    for(int i=0;i<6;++i)scene.UpdateSimulation(0.1f,{});
    travel({3000,2020});travel({2990,2020});
    SceneInput facePickup;facePickup.movement={1,0};scene.UpdateSimulation(0,facePickup);
    scene.UpdateSimulation(0,interact);Check(scene.Snapshot().inventoryCount==1,"pickup mutated at Start");
    scene.UpdateSimulation(0.1f,{});
    Check(scene.Snapshot().inventoryCount==2 && !scene.TargetState("item.demo.01")->active,"pickup impact not applied");
    for(int i=0;i<10;++i)scene.UpdateSimulation(0.1f,{});
    Check(scene.Snapshot().inventoryCount==2,"pickup replayed after completion");

    // Validate the real scene draw path, including the opt-in corner portraits.
    MyEngine::Canvas2D canvas;Check(canvas.Initialize(),"canvas failed");glViewport(0,0,640,360);
    options.debugOverlay=false;scene.SetOptions(options);scene.Render(canvas,640,360);
    unsigned char clean[4]={};glReadPixels(30,330,1,1,GL_RGBA,GL_UNSIGNED_BYTE,clean);
    options.debugOverlay=true;scene.SetOptions(options);scene.Render(canvas,640,360);
    unsigned char overlay[4]={};glReadPixels(30,330,1,1,GL_RGBA,GL_UNSIGNED_BYTE,overlay);
    Check(clean[0]!=overlay[0] || clean[1]!=overlay[1] || clean[2]!=overlay[2],"overlay toggle did not affect corner rendering");
    options.debugOverlay=false;scene.SetOptions(options);scene.Render(canvas,640,360);
    unsigned char hidden[4]={};glReadPixels(30,330,1,1,GL_RGBA,GL_UNSIGNED_BYTE,hidden);
    Check(clean[0]==hidden[0] && clean[1]==hidden[1] && clean[2]==hidden[2],"debug portraits leaked into gameplay");
    SceneInput quiet;
    for(int i=0;i<51;++i)scene.UpdateSimulation(0.1f,quiet);
    Check(scene.HotbarAlpha()==0,"live idle hotbar not hidden");
    scene.Render(canvas,640,360);
    const auto hotbar=HotbarLayout::ForViewport(640,360);
    unsigned char hiddenBar[4]={};glReadPixels(static_cast<int>(hotbar.x+3),360-static_cast<int>(hotbar.y+3),1,1,GL_RGBA,GL_UNSIGNED_BYTE,hiddenBar);
    SceneInput reveal;reveal.selectedItem=0;scene.UpdateSimulation(0,reveal);
    Check(scene.HotbarAlpha()==1,"live selection did not reveal hotbar");scene.Render(canvas,640,360);
    unsigned char shownBar[4]={};glReadPixels(static_cast<int>(hotbar.x+3),360-static_cast<int>(hotbar.y+3),1,1,GL_RGBA,GL_UNSIGNED_BYTE,shownBar);
    Check(hiddenBar[0]!=shownBar[0] || hiddenBar[1]!=shownBar[1] || hiddenBar[2]!=shownBar[2],"hidden hotbar still rendered");
    unsigned char closedPanel[4]={};glReadPixels(415,340,1,1,GL_RGBA,GL_UNSIGNED_BYTE,closedPanel);
    SceneInput openStorage;openStorage.toggleStorage=true;scene.CaptureInput(openStorage);scene.FixedUpdate(1.0f/60);
    Check(scene.Snapshot().storageOpen,"K did not open storage in own garden");
    scene.FixedUpdate(1.0f/60);Check(scene.Snapshot().storageOpen,"catch-up replayed K and closed storage");
    scene.Render(canvas,640,360);unsigned char openedPanel[4]={};glReadPixels(415,340,1,1,GL_RGBA,GL_UNSIGNED_BYTE,openedPanel);
    Check(closedPanel[0]!=openedPanel[0] || closedPanel[1]!=openedPanel[1] || closedPanel[2]!=openedPanel[2],"on-demand warehouse panel not rendered");
    SceneInput withdraw;
    withdraw.selectedItem=0;
    withdraw.interact=true;
    scene.CaptureInput(withdraw);
    scene.FixedUpdate(1.0f/60);
    scene.FixedUpdate(1.0f/60);
    Check(scene.Bag().Count(ItemId::Turnip)==1 && scene.Storage().items.Count(ItemId::Turnip)==0,
          "warehouse withdrawal did not transfer exactly once");
    SceneInput deposit;
    deposit.cancel=true;
    scene.UpdateSimulation(0,deposit);
    Check(scene.Bag().Count(ItemId::Turnip)==0 && scene.Storage().items.Count(ItemId::Turnip)==1,
          "warehouse deposit did not restore both counts");
    scene.UpdateSimulation(0,openStorage);Check(!scene.Snapshot().storageOpen,"K did not close storage");
    scene.UpdateSimulation(0,openStorage);SceneInput leave;leave.teleport=6;
    options.demoMode=true;scene.SetOptions(options);scene.UpdateSimulation(0,leave);
    Check(!scene.Snapshot().storageOpen,"leaving home did not close storage");
    scene.UpdateSimulation(0,openStorage);Check(!scene.Snapshot().storageOpen,"storage opened outside own property");
    SceneInput market;
    market.toggleMarket=true;
    scene.CaptureInput(market);
    scene.FixedUpdate(1.0f/60);
    Check(scene.Snapshot().marketOpen,"M did not open market");
    SceneInput buy;
    buy.selectedItem=1;
    buy.interact=true;
    scene.CaptureInput(buy);
    scene.FixedUpdate(1.0f/60);
    scene.FixedUpdate(1.0f/60);
    Check(scene.Bag().Count(ItemId::CarrotSeed)==1 && scene.SelectedItem()==ItemId::CarrotSeed &&
          scene.Snapshot().money==15,"buffered purchase repeated or did not bind seed");
    scene.Render(canvas,640,360);
    std::cout<<"Market screen GL draw calls: "<<canvas.DrawCalls()<<'\n';
    Check(canvas.DrawCalls()>0 && canvas.DrawCalls()<=2,"market text did not batch in screen pass");
    scene.UpdateSimulation(0,market);
    Check(!scene.Snapshot().marketOpen,"M did not close market");
    const auto snapshot=DecodeSave(EncodeSave(scene.CaptureSave()));
    Check(snapshot.has_value(),"live farm snapshot failed validation");
    if(snapshot)
    {
        scene.Reset();
        auto legacy=*snapshot;
        std::erase_if(legacy.targets,[](const auto& target){return target.id.starts_with("crop.home.");});
        Check(scene.RestoreSave(legacy) && scene.TargetState("crop.home.1").has_value() &&
              !scene.TargetState("crop.home.1")->planted,"pre-grid save did not initialize new empty plots");
        scene.Reset();
        auto bad=*snapshot;
        bad.targets.pop_back();
        Check(!scene.RestoreSave(bad) && scene.Snapshot().money==20,"invalid save changed live scene");
        Check(scene.RestoreSave(*snapshot) && scene.Snapshot().money==15 &&
              scene.SelectedItem()==ItemId::CarrotSeed &&
              scene.Storage().items.Count(ItemId::Turnip)==1 &&
              scene.TargetState("crop.plot.01")->planted,"scene did not restore inventory, money, storage and planted crop");
        if(std::filesystem::is_regular_file(MyEngine::AssetRoot()/"LocalReview/soil_wet.png"))
        {
            auto soil=*snapshot;
            soil.position={2200,1950};
            for(auto& target:soil.targets)
                if(target.id=="crop.home.1")
                {
                    target.planted=true;
                    target.crop=CropId::Turnip;
                    target.ready=false;
                    target.age=0;
                    target.moisture=0;
                }
            MyEngine::Camera2D probeCamera;
            probeCamera.SetWorldSize(WorldMap::Size());
            probeCamera.Resize(640,360);
            probeCamera.SetZoom(0.85f);
            probeCamera.SetPosition(soil.position);
            const auto sample=probeCamera.WorldToScreen({2185,2030});
            std::array<unsigned char,4> dry{},wet{};
            Check(scene.RestoreSave(soil),"dry soil review snapshot failed");
            scene.Render(canvas,640,360);
            glReadPixels(static_cast<int>(sample.x),359-static_cast<int>(sample.y),1,1,GL_RGBA,GL_UNSIGNED_BYTE,dry.data());
            for(auto& target:soil.targets)
                if(target.id=="crop.home.1") target.moisture=6;
            Check(scene.RestoreSave(soil),"wet soil review snapshot failed");
            scene.Render(canvas,640,360);
            glReadPixels(static_cast<int>(sample.x),359-static_cast<int>(sample.y),1,1,GL_RGBA,GL_UNSIGNED_BYTE,wet.data());
            Check(dry!=wet && glGetError()==GL_NO_ERROR,"local soil did not display dry/wet distinction");
        }
        if(std::filesystem::is_regular_file(MyEngine::AssetRoot()/"LocalReview/carrot_mature.png"))
        {
            auto carrot=*snapshot;
            carrot.position={3000,1780};
            for(auto& target:carrot.targets)
            {
                if(target.id!="crop.plot.01") continue;
                target.crop=CropId::Carrot;
                target.planted=true;
                target.ready=false;
                target.age=0;
                target.moisture=0;
            }
            Check(scene.RestoreSave(carrot),"carrot review scene failed to restore");
            scene.Render(canvas,640,360);
            std::array<unsigned char,64*80*4> sprout{},ripe{};
            glReadPixels(288,230,64,80,GL_RGBA,GL_UNSIGNED_BYTE,sprout.data());
            for(auto& target:carrot.targets)
                if(target.id=="crop.plot.01"){target.age=8;target.ready=true;}
            Check(scene.RestoreSave(carrot),"ripe carrot scene failed to restore");
            scene.Render(canvas,640,360);
            glReadPixels(288,230,64,80,GL_RGBA,GL_UNSIGNED_BYTE,ripe.data());
            Check(sprout!=ripe && glGetError()==GL_NO_ERROR,"carrot growth stages did not render differently");
        }

    }


    Check(glGetError()==GL_NO_ERROR,"OpenGL error in Gameplay11 rendering");
    canvas.Shutdown();scene.Shutdown();assets.Shutdown();

    GameplayState state;state.Enter();
    auto* native=window.GetNativeWindow();glfwPollEvents();glfwFocusWindow(native);glfwPollEvents();
    const auto key=glfwSetKeyCallback(native,nullptr);glfwSetKeyCallback(native,key);
    const auto mouse=glfwSetMouseButtonCallback(native,nullptr);glfwSetMouseButtonCallback(native,mouse);
    Check(mouse!=nullptr,"mouse callback missing");
    if(mouse)
    {
        mouse(native,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);mouse(native,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);
        Check(MyEngine::Input::ConsumeMousePress(GLFW_MOUSE_BUTTON_LEFT),"short mouse click lost");
        Check(!MyEngine::Input::ConsumeMousePress(GLFW_MOUSE_BUTTON_LEFT),"mouse click consumed twice");
    }
    const auto tap=[&](int code){key(native,code,0,GLFW_PRESS,0);key(native,code,0,GLFW_RELEASE,0);state.ProcessInput();state.Update(0);};
    Check(key!=nullptr,"input callback missing");
    if(key)
    {
        if(mouse)
        {
            state.Render(1);
            const auto click=[&](double x,double y)
            {
                glfwSetCursorPos(native,x,y);
                mouse(native,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
                mouse(native,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);
                state.ProcessInput();state.Update(1.0f/60);state.Render(1);
            };
            click(40,18);
            Check(state.Snapshot().gardenControl,"clickable mode control failed");
            const auto toolbar=HotbarLayout::ForViewport(640,360);
            click(toolbar.x+toolbar.padding+2*(toolbar.slot+toolbar.gap)+toolbar.slot/2,
                  toolbar.y+toolbar.padding+toolbar.slot/2);
            Check(state.Snapshot().inventoryCount==3,"hotbar click leaked to world");
            MyEngine::Camera2D pick;
            pick.SetWorldSize(WorldMap::Size());pick.Resize(640,360);pick.SetZoom(0.85f);
            pick.SetPosition(state.Snapshot().player);
            const auto plot=pick.WorldToScreen({2336,2080});
            click(plot.x,plot.y);
            Check(state.Snapshot().inventoryCount==2,"live cursor/camera mapping failed to plant remote plot");
            tap(GLFW_KEY_M);click(plot.x,plot.y);
            Check(state.Snapshot().inventoryCount==2 && state.Snapshot().marketOpen,"menu click leaked to world");
            tap(GLFW_KEY_M);tap(GLFW_KEY_R);
        }
        tap(GLFW_KEY_F1);Check(state.Mode()==3,"gallery accessible in default gameplay");
        tap(GLFW_KEY_F9);Check(state.Options().debugOverlay,"F9 did not enable overlay");
        tap(GLFW_KEY_F7);tap(GLFW_KEY_F);
        const auto previewAction=state.Snapshot();
        Check(previewAction.actionPhase==ActionPhase::Start,"keyboard F did not begin transaction");
        tap(GLFW_KEY_F1);Check(state.Mode()==1,"debug gallery no longer available");
        state.Update(10);
        Check(state.Snapshot().inventoryCount==3 && state.Snapshot().actionId==previewAction.actionId &&
              state.Snapshot().actionPhase==ActionPhase::Start,"preview advanced pending action");
        tap(GLFW_KEY_F3);tap(GLFW_KEY_C);
        const auto cancelledPreview=state.Snapshot();
        Check(cancelledPreview.actionPhase==ActionPhase::Cancelled,"keyboard C failed to cancel resumed action");
        tap(GLFW_KEY_F1);tap(GLFW_KEY_F);tap(GLFW_KEY_F3);
        Check(state.Snapshot().actionId==cancelledPreview.actionId && state.Snapshot().inventoryCount==3,
              "preview key tap replayed into a new interaction");
        tap(GLFW_KEY_F1);
        tap(GLFW_KEY_F9);Check(state.Mode()==3 && !state.Options().debugOverlay,"overlay off did not dismiss gallery");
        tap(GLFW_KEY_F10);tap(GLFW_KEY_F11);tap(GLFW_KEY_F12);
        Check(state.Options().demoMode && state.Options().npcAI && state.Options().petFollow,"independent runtime toggles failed");
        key(native,GLFW_KEY_F,0,GLFW_PRESS,0);
        const auto focus=glfwSetWindowFocusCallback(native,nullptr);glfwSetWindowFocusCallback(native,focus);
        Check(focus!=nullptr,"focus-loss handler missing");if(focus) focus(native,GLFW_FALSE);
        Check(!MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F),"focus loss left stale input");
    }
    state.Exit();MyEngine::Input::Initialize(nullptr);
    return failures?1:0;
}
