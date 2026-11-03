#include "DemoScene2D.h"
#include "Gameplay/Player/Movement2D.h"
#include "Input/Input.h"
#include "Resources/AssetPath.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>

namespace
{
    const glm::vec2 WorldSize{1600, 1067};
    constexpr std::array<const char*,4> ItemNames = {"Watering can", "Garden hoe", "Seeds", "Strawberry cake"};
}

namespace MyGame
{
    bool DemoScene2D::Initialize(AssetTextures& assets,const std::filesystem::path& dataRoot)
    {
        if(!m_Visuals.Load(dataRoot/"VisualClips.csv"))
            for(const auto& error:m_Visuals.Errors()) std::cerr<<"Visual catalog: "<<error<<'\n';
        m_Map=assets.Load("world_garden");
        m_Characters[0]=assets.Load("player_linh");
        m_Characters[1]=assets.Load("player_an");
        m_Items=assets.Load("items_farm");
        if(const auto* items=assets.Record("items_farm"))
            for(int i=0;i<4;++i)
            {
                const auto uv=AssetManifest::FrameUv(*items,i);
                if(uv) m_ItemRegions[static_cast<std::size_t>(i)]=*uv;
                else {m_Items=nullptr;std::cerr<<"Asset fallback [items_farm]: four_item_regions_required\n";}
            }
        for(int i=0;i<2;++i)
            if(const auto* character=assets.Record(i==0?"player_linh":"player_an"))
                m_Anchors[static_cast<std::size_t>(i)]={character->anchorX,character->anchorY};
        m_Ready=true; // Missing artwork uses geometric placeholders, not a fatal game error.
        Reset();
        return m_Ready;
    }
    void DemoScene2D::Reset()
    {
        m_Player = {760,620};
        m_WalkTime = 0;
        m_NpcTime=m_DogTime=0;m_GameAge=0;
        m_Moving = false;
        m_Facing=Facing::Down;m_PlayerState=VisualState::Idle;
        m_PlayerAnimation.Select(m_Visuals,m_Character==0?"player_linh":"player_an",m_PlayerState,m_Facing);
        m_NpcAnimation.Select(m_Visuals,"npc_an",VisualState::Idle,Facing::Down);
        m_DogAnimation.Select(m_Visuals,"dog",VisualState::Idle,Facing::Down);
        m_CropAnimation.Select(m_Visuals,"crop_turnip",VisualState::Idle,Facing::Any);
        m_Camera.SetWorldSize(WorldSize);
        m_Camera.SetZoom(0.85f);
        m_Camera.SetPosition(m_Player);
    }
    void DemoScene2D::Update(float deltaTime)
    {
        if(!std::isfinite(deltaTime) || deltaTime<0.0f) deltaTime=0.0f;
        glm::vec2 direction{
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_D) ? 1.0f : 0.0f) -
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_A) ? 1.0f : 0.0f),
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_S) ? 1.0f : 0.0f) -
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_W) ? 1.0f : 0.0f)};
        m_Moving = glm::length(direction) > 0;
        m_Facing=FacingFromVelocity(direction.x,direction.y,m_Facing);
        const bool running=MyEngine::Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT);
        const float speed = running ? 300.0f : 180.0f;
        m_Player = MoveWithinBounds(m_Player, direction, speed, deltaTime, WorldSize);
        if (m_Moving) m_WalkTime += deltaTime;
        else m_WalkTime = 0;
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_Q)) m_Camera.SetZoom(m_Camera.Zoom()/1.15f);
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_E)) m_Camera.SetZoom(m_Camera.Zoom()*1.15f);
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_R)) {Reset();return;}
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_TAB)) m_Character = 1-m_Character;
        VisualState requested=m_Moving?(running?VisualState::Run:VisualState::Walk):VisualState::Idle;
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_SPACE)) requested=VisualState::Action;
        else if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_H)) requested=VisualState::Hurt;
        else if((m_PlayerState==VisualState::Action || m_PlayerState==VisualState::Hurt) &&
                !m_PlayerAnimation.Finished()) requested=m_PlayerState;
        m_PlayerState=requested;
        m_PlayerAnimation.Select(m_Visuals,m_Character==0?"player_linh":"player_an",m_PlayerState,m_Facing);
        m_PlayerAnimation.Update(deltaTime,false);

        m_NpcTime+=deltaTime;
        const auto npcState=std::fmod(m_NpcTime,8.0f)>5.5f?VisualState::Action:
                            (std::fmod(m_NpcTime,8.0f)>2.5f?VisualState::Walk:VisualState::Idle);
        const auto npcFacing=std::fmod(m_NpcTime,8.0f)>4.0f?Facing::Left:Facing::Right;
        if(!m_NpcAnimation.Select(m_Visuals,"npc_an",npcState,npcState==VisualState::Walk?npcFacing:Facing::Down))
            m_NpcAnimation.Select(m_Visuals,"npc_an",VisualState::Idle,Facing::Down);
        m_NpcAnimation.Update(deltaTime,false);

        m_DogTime+=deltaTime;
        const float dogDistance=glm::length(m_Player-m_Dog);
        const auto dogState=dogDistance>180?VisualState::Run:(dogDistance>80?VisualState::Walk:
                            (std::fmod(m_DogTime,12.0f)>8?VisualState::Sleep:VisualState::Idle));
        m_DogAnimation.Select(m_Visuals,"dog",dogState,FacingFromVelocity(m_Player.x-m_Dog.x,m_Player.y-m_Dog.y,Facing::Down));
        m_DogAnimation.Update(deltaTime,false);
        if(dogState==VisualState::Walk || dogState==VisualState::Run)
            m_Dog=MoveWithinBounds(m_Dog,m_Player-m_Dog,dogState==VisualState::Run?210.0f:120.0f,deltaTime,WorldSize);
        m_GameAge+=deltaTime;
        for (int i=0; i<4; ++i)
            if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_1+i))
            {
                m_SelectedItem = i;
                if (auto* window = glfwGetCurrentContext())
                {
                    const std::string title = std::string("My Farm | ") + ItemNames[i] +
                        " | WASD: Move | Q/E: Zoom | Tab: Character | F1/F2: Gallery | Home: Landscape | F3: World";
                    glfwSetWindowTitle(window, title.c_str());
                }
            }
        m_Camera.Follow(m_Player, deltaTime);
    }
    void DemoScene2D::DrawActor(MyEngine::Canvas2D& canvas,int character,glm::vec2 feet,
                                const AnimationController& animation,float bob)
    {
        // Ground marker and a single-pose cutout; bobbing is procedural, not a walk-cycle sheet.
        canvas.Rectangle(feet.x-22, feet.y-4, 44, 9, {0.08f,0.16f,0.09f,0.19f});
        const auto* texture=m_Characters[static_cast<std::size_t>(character)];
        const auto anchor=m_Anchors[static_cast<std::size_t>(character)];
        // Current approved player sheets contain one frame. The controller still owns the
        // state timeline; multi-frame UVs can be enabled when a reviewed sheet is exported.
        (void)animation;
        if(texture) canvas.Sprite(*texture, feet.x-156*anchor.x, feet.y-156*anchor.y-bob, 156,156);
        else canvas.Rectangle(feet.x-22,feet.y-78-bob,44,78,
            character==0?std::array<float,4>{0.85f,0.65f,0.22f,1}:std::array<float,4>{0.28f,0.5f,0.8f,1});
    }
    void DemoScene2D::DrawDog(MyEngine::Canvas2D& canvas) const
    {
        const auto state=m_DogAnimation.State();
        const float height=state==VisualState::Sleep?24.0f:42.0f;
        canvas.Rectangle(m_Dog.x-25,m_Dog.y-height,50,height,{0.55f,0.32f,0.16f,1});
        canvas.Rectangle(m_Dog.x+16,m_Dog.y-height-5,18,18,{0.68f,0.43f,0.22f,1});
    }
    void DemoScene2D::DrawCrop(MyEngine::Canvas2D& canvas) const
    {
        const int stage=GrowthFrame(m_GameAge,{5,8,12,16});
        const float size=14.0f+static_cast<float>(stage)*7.0f;
        const std::array<std::array<float,4>,4> colors={{{0.48f,0.72f,0.22f,1},{0.31f,0.68f,0.18f,1},
            {0.22f,0.58f,0.15f,1},{0.65f,0.82f,0.25f,1}}};
        canvas.Rectangle(570-size*0.5f,735-size,size,size,colors[static_cast<std::size_t>(stage)]);
    }
    void DemoScene2D::Render(MyEngine::Canvas2D& canvas, int width, int height)
    {
        if (!m_Ready) return;
        m_Camera.Resize(width,height);
        canvas.BeginWorld(width,height,m_Camera);
        // Neutral surround when an unusually wide view extends beyond the painted map.
        canvas.Rectangle(-4000,-4000,8000,8000,{0.43f,0.57f,0.28f,1});
        if(m_Map) canvas.Image(*m_Map,0,0,WorldSize.x,WorldSize.y);
        DrawCrop(canvas);
        constexpr std::array<glm::vec2,4> objects = {{{650,700},{1010,700},{1090,770},{970,850}}};
        for (std::size_t i=0; i<objects.size(); ++i)
            if(m_Items) canvas.Sprite(*m_Items,objects[i].x-32,objects[i].y-52,64,64,m_ItemRegions[i]);
            else canvas.Rectangle(objects[i].x-16,objects[i].y-24,32,32,{0.8f,0.5f,0.2f,1});
        const glm::vec2 npc{990,600};
        const float bob = m_Moving ? std::sin(m_WalkTime*12.0f)*2.2f : 0;
        // Painter's ordering gives sprites a consistent overlap by their foot Y.
        if (m_Player.y < npc.y) DrawActor(canvas,m_Character,m_Player,m_PlayerAnimation,bob);
        DrawActor(canvas,1-m_Character,npc,m_NpcAnimation,0);
        DrawDog(canvas);
        if (m_Player.y >= npc.y) DrawActor(canvas,m_Character,m_Player,m_PlayerAnimation,bob);
        canvas.End();

        // Screen UI uses a different projection and therefore does not zoom with the world.
        canvas.Begin(width,height);
        const float size = std::min(78.0f, static_cast<float>(width)/5.5f);
        const float gap = size*0.12f;
        const float start = (static_cast<float>(width) - (size*4+gap*3))*0.5f;
        const float y = std::max(0.0f, static_cast<float>(height)-size-20);
        for (int i=0; i<4; ++i)
        {
            const float x = start+static_cast<float>(i)*(size+gap);
            canvas.Rectangle(x-3,y-3,size+6,size+6,
                             i==m_SelectedItem ? std::array<float,4>{0.95f,0.74f,0.32f,1} : std::array<float,4>{0.18f,0.28f,0.18f,0.7f});
            canvas.Rectangle(x,y,size,size,{0.99f,0.97f,0.88f,0.94f});
            if(m_Items) canvas.Sprite(*m_Items,x+5,y+5,size-10,size-10,m_ItemRegions[static_cast<std::size_t>(i)]);
            else canvas.Rectangle(x+10,y+10,size-20,size-20,{0.65f,0.35f,0.16f,1});
        }
        canvas.Rectangle(16,16,132,76,{0.99f,0.97f,0.88f,0.92f});
        for (int i=0; i<2; ++i)
        {
            const float x=20+static_cast<float>(i)*62;
            const auto* portrait=m_Characters[static_cast<std::size_t>(i)];
            if(portrait) canvas.Image(*portrait,x,20,58,58);
            if (i==m_Character) canvas.Rectangle(x,82,58,3,{0.71f,0.46f,0.14f,1});
        }
        canvas.End();
    }
    void DemoScene2D::Shutdown()
    {
        m_Ready = false;
        m_Map=m_Items=nullptr;
        m_Characters.fill(nullptr);
        m_Visuals={};m_PlayerAnimation={};m_NpcAnimation={};m_DogAnimation={};m_CropAnimation={};
    }
}
