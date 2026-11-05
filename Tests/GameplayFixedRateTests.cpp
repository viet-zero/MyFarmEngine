#include "Core/Window.h"
#include "Core/Time.h"
#include "Resources/AssetPath.h"
#include "Gameplay/World/DemoScene2D.h"
#include <glad/gl.h>
#include <cmath>
#include <iostream>
namespace
{
    int failures=0;
    void Check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';++failures;}}
    bool Same(glm::vec2 a,glm::vec2 b){return glm::length(a-b)<0.001f;}
    struct Result
    {
        MyGame::SceneSnapshot state;
        MyGame::PresentationSnapshot presented;
        int stock=0;
        std::uint64_t ticks=0;
    };
}
int main()
{
    using namespace MyGame;
    using MyEngine::FixedStepClock;
    MyEngine::Window window;if(!window.Create(640,360,"Fixed-rate integration",false))return 1;
    glViewport(0,0,640,360);
    AssetTextures assets;Check(assets.Initialize(MyEngine::AssetRoot()),"assets failed");
    MyEngine::Canvas2D canvas;Check(canvas.Initialize(),"canvas failed");
    const auto trace=[&](int hz)
    {
        SceneOptions options;options.demoMode=true;options.npcAI=true;options.petFollow=true;
        DemoScene2D scene(options);Check(scene.Initialize(assets,MyEngine::AssetRoot()/"Data"),"scene failed");
        scene.SetViewport(640,360);scene.SynchronizePresentation();FixedStepClock clock;
        for(int frame=0;frame<hz*6;++frame)
        {
            SceneInput input;
            if(frame==0)input.teleport=7;
            if(frame==hz/2 || frame==hz)input.interact=true;
            if(frame>=hz*2 && frame<hz*3)input.movement={1,0};
            if(frame>=hz*3 && frame<hz*4)input.movement={0,1};
            input.running=frame>=hz*5/2 && frame<hz*3;
            if(frame==hz*4)input.zoom=1;
            scene.CaptureInput(input);
            const auto plan=clock.Advance(1.0/hz);
            for(int step=0;step<plan.steps;++step)scene.FixedUpdate(static_cast<float>(FixedStepClock::StepSeconds));
            const auto before=scene.Snapshot();const auto cameraBefore=scene.Presentation(1).camera;
            const auto smooth=scene.Presentation(plan.alpha);
            Check(std::isfinite(smooth.player.x) && std::isfinite(smooth.camera.y),"interpolation returned invalid position");
            if(frame%hz==0 || frame==hz*6-1)scene.Render(canvas,640,360,plan.alpha);
            const auto after=scene.Snapshot();
            Check(Same(before.player,after.player) && Same(before.npc,after.npc) && Same(before.pet,after.pet) &&
                  Same(cameraBefore,scene.Presentation(1).camera) && before.inventoryCount==after.inventoryCount && before.storedItemCount==after.storedItemCount &&
                  before.actionPhase==after.actionPhase && before.playerFrame==after.playerFrame && before.effects==after.effects,
                  "presentation/render advanced simulation, animation or gameplay events");
        }
        Result result{scene.Snapshot(),scene.Presentation(1),scene.TargetState("tree.apple.01")->stock,clock.TotalSteps()};
        Check(result.ticks==360 && result.state.fruitHarvested==2 && result.state.inventoryCount==3 && result.state.storedItemCount==2 && result.stock==1,
              "trace skipped/repeated interaction or simulated time");
        std::cout<<"Virtual render Hz="<<hz<<"; fixed ticks="<<result.ticks<<"; apples="<<result.state.fruitHarvested<<'\n';
        scene.Shutdown();return result;
    };
    const auto reference=trace(60);
    for(const int hz:{30,144})
    {
        const auto result=trace(hz);
        Check(Same(reference.state.player,result.state.player) && Same(reference.state.npc,result.state.npc) &&
              Same(reference.state.pet,result.state.pet) && Same(reference.presented.camera,result.presented.camera) &&
              reference.state.playerFrame==result.state.playerFrame && reference.state.npcFrame==result.state.npcFrame &&
              reference.state.petFrame==result.state.petFrame && reference.state.actionPhase==result.state.actionPhase &&
              reference.state.actionId==result.state.actionId && reference.stock==result.stock,
              "30/60/144 cadence changed actor/camera/animation/action result");
    }
    SceneOptions options;options.demoMode=true;DemoScene2D probe(options);
    probe.Initialize(assets,MyEngine::AssetRoot()/"Data");probe.SetViewport(640,360);probe.SynchronizePresentation();
    const auto original=probe.Snapshot().player;SceneInput move;move.movement={1,0};probe.CaptureInput(move);
    probe.FixedUpdate(static_cast<float>(FixedStepClock::StepSeconds));const auto current=probe.Snapshot().player;
    Check(!Same(original,current) && Same(probe.Presentation(0).player,original) && Same(probe.Presentation(1).player,current) &&
          Same(probe.Presentation(0.5f).player,(original+current)*0.5f),"previous/current actor interpolation incorrect");
    probe.SynchronizePresentation();Check(Same(probe.Presentation(0).player,current),"pause snap interpolated stale actor history");
    SceneInput reset;reset.reset=true;reset.interact=true;reset.movement={1,0};probe.CaptureInput(reset);
    for(int step=0;step<6;++step)probe.FixedUpdate(static_cast<float>(FixedStepClock::StepSeconds));
    Check(Same(probe.Snapshot().player,original) && probe.Snapshot().inventoryCount==3 && probe.Snapshot().actionPhase==ActionPhase::Idle,
          "reset catch-up replayed old movement or interaction command");
    SceneInput teleport;teleport.teleport=7;probe.CaptureInput(teleport);probe.FixedUpdate(static_cast<float>(FixedStepClock::StepSeconds));
    Check(Same(probe.Presentation(0).player,probe.Snapshot().player) && Same(probe.Presentation(0).pet,probe.Snapshot().pet),
          "teleport interpolated across the world");
    SceneInput interact;interact.interact=true;probe.CaptureInput(interact);probe.FixedUpdate(static_cast<float>(FixedStepClock::StepSeconds));
    const auto start=probe.Snapshot();
    for(const float alpha:{0.0f,0.5f,1.0f})probe.Render(canvas,640,360,alpha);
    Check(probe.Snapshot().inventoryCount==3 && probe.Snapshot().effects==0 && start.actionPhase==probe.Snapshot().actionPhase,
          "render/interpolation applied action impact");
    for(int tick=0;tick<6;++tick)probe.FixedUpdate(static_cast<float>(FixedStepClock::StepSeconds));
    Check(probe.Snapshot().inventoryCount==3 && probe.Snapshot().storedItemCount==1 && probe.Snapshot().fruitHarvested==1,"fixed-tick marker impact missing or duplicated");
    probe.SetViewport(240,480);probe.Render(canvas,240,480,0.5f);
    Check(Same(probe.Presentation(0).camera,probe.Presentation(1).camera),"viewport resize retained camera history");
    Check(glGetError()==GL_NO_ERROR,"GL error in fixed-rate rendering");
    probe.Shutdown();assets.Shutdown();canvas.Shutdown();return failures?1:0;
}
