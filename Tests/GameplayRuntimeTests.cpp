#include "Core/Window.h"
#include "Input/Input.h"
#include "Resources/AssetPath.h"
#include "Gameplay/World/DemoScene2D.h"
#include "Gameplay/States/GameplayState.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
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
    AssetTextures assets;
    Check(assets.Initialize(MyEngine::AssetRoot()),"asset manifest initialization failed");
    DemoScene2D scene;
    Check(scene.Initialize(assets,MyEngine::AssetRoot()/"Data"),"scene failed");
    auto initial=scene.Snapshot();
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
    Check(glGetError()==GL_NO_ERROR,"OpenGL error in Gameplay11 rendering");
    canvas.Shutdown();scene.Shutdown();assets.Shutdown();

    GameplayState state;state.Enter();
    auto* native=window.GetNativeWindow();glfwPollEvents();glfwFocusWindow(native);glfwPollEvents();
    const auto key=glfwSetKeyCallback(native,nullptr);glfwSetKeyCallback(native,key);
    const auto tap=[&](int code){key(native,code,0,GLFW_PRESS,0);key(native,code,0,GLFW_RELEASE,0);state.Update(0);};
    Check(key!=nullptr,"input callback missing");
    if(key)
    {
        tap(GLFW_KEY_F1);Check(state.Mode()==3,"gallery accessible in default gameplay");
        tap(GLFW_KEY_F9);Check(state.Options().debugOverlay,"F9 did not enable overlay");
        tap(GLFW_KEY_F1);Check(state.Mode()==1,"debug gallery no longer available");
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
