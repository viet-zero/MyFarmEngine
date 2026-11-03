#include "DemoScene2D.h"
#include "Gameplay/Systems/CollisionSystem.h"
#include "Input/Input.h"
#include "Resources/AssetPath.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>

namespace
{
    const glm::vec2 WorldSize=MyGame::WorldMap::Size();
    constexpr std::array<const char*,4> ItemNames = {"Watering can", "Garden hoe", "Seeds", "Strawberry cake"};
    constexpr glm::vec2 NpcPosition{3190,2020};
    constexpr MyGame::ColliderComponent PlayerCollider{{18,12}};
    constexpr MyGame::ColliderComponent DogCollider{{22,13}};
    constexpr MyGame::ColliderComponent NpcCollider{{18,12}};
}

namespace MyGame
{
    bool DemoScene2D::Initialize(AssetTextures& assets,const std::filesystem::path& dataRoot)
    {
        m_Assets=&assets;
        if(!m_World.LoadObjects(dataRoot/"Objects.csv"))
            std::cerr<<"World objects: definitions missing or invalid; object fallback omitted\n";
        if(!m_World.LoadStaticColliders(dataRoot/"StaticColliders.csv"))
            std::cerr<<"World colliders: definitions missing or invalid; static fallback omitted\n";
        if(!m_Trees.Load(dataRoot/"ForegroundTrees.csv",m_World))
            std::cerr<<"Foreground trees: definitions missing or invalid; orchard overlay omitted\n";
        if(!m_Visuals.Load(dataRoot/"VisualClips.csv"))
            for(const auto& error:m_Visuals.Errors()) std::cerr<<"Visual catalog: "<<error<<'\n';
        for(const auto& id:m_Visuals.AssetIds()) assets.Load(id);
        m_Map=assets.Load("world_phase_e");
        m_Characters[0]=assets.Load("player_linh");
        m_Characters[1]=assets.Load("player_an");
        m_DogTexture=assets.Load("dog_walk");
        m_CropTexture=assets.Load("turnip_growth");
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
        m_Player = m_World.Spawn(Zone::Farm);
        m_Dog=m_Player+glm::vec2{-70,30};
        m_Actor={};m_Actor.position=m_Player;
        m_FruitHarvested=0;
        m_Targets.clear();m_Targets.reserve(m_World.Objects().size()+m_Trees.Entries().size()+2);
        for(const auto& object:m_World.Objects())
        {
            m_World.SetObjectActive(object.gameplayId,true);
            if(object.blocked && !m_Trees.Contains(object.gameplayId))
                m_Targets.push_back({object.gameplayId,TargetKind::Object,
                    object.position+object.size*0.5f,2});
        }
        for(const auto& tree:m_Trees.Entries())
        {
            InteractionTarget target{tree.gameplayId,TargetKind::FruitTree,tree.interaction};
            target.stock=target.maxStock=tree.fruitCount;
            m_Targets.push_back(std::move(target));
        }
        m_Targets.push_back({"crop.plot.01",TargetKind::Crop,{3000,1700}});
        m_Targets.push_back({"item.demo.01",TargetKind::Pickup,{3070,2020}});
        m_Events.Clear();m_Effects.Clear();m_FeedbackTime=0;
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
        m_Facing=FacingFromVelocity(direction.x,direction.y,m_Facing);
        const bool running=MyEngine::Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT);
        const float speed = running ? 300.0f : 180.0f;
        const std::array<DynamicCollider,2> playerObstacles={{{NpcPosition,NpcCollider},{m_Dog,DogCollider}}};
        const auto previous=m_Player;
        m_Player=CollisionSystem::MoveAndSlide(m_World,m_Player,direction,speed,deltaTime,
                                               PlayerCollider,playerObstacles);
        m_Moving=glm::length(m_Player-previous)>0.001f;
        if (m_Moving) m_WalkTime += deltaTime;
        else m_WalkTime = 0;
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_Q)) m_Camera.SetZoom(m_Camera.Zoom()/1.15f);
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_E)) m_Camera.SetZoom(m_Camera.Zoom()*1.15f);
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_R)) {Reset();return;}
        bool teleported=false;
        glm::vec2 teleportDogOffset{-70,30};
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F4)){m_Player=m_World.Spawn(Zone::Town);teleported=true;}
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F5)){m_Player=m_World.Spawn(Zone::Farm);teleported=true;}
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F6)){m_Player=m_World.Spawn(Zone::LakeForest);teleported=true;}
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F7))
        {
            m_Player={1990,1740};m_Facing=Facing::Up;teleported=true;
        }
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F8))
        {
            m_Player={2140,1450};m_Facing=Facing::Left;
            teleportDogOffset={100,30};teleported=true;
        }
        if(teleported)
        {
            m_Dog=m_Player+teleportDogOffset;
            m_Camera.SetPosition(m_Player);
        }
        m_Actor.position=m_Player;
        switch(m_Facing)
        {
        case Facing::Left:m_Actor.facing={-1,0};break;
        case Facing::Right:m_Actor.facing={1,0};break;
        case Facing::Up:m_Actor.facing={0,-1};break;
        default:m_Actor.facing={0,1};break;
        }
        InteractionSystem::Tick(m_Actor,m_Targets,deltaTime);
        bool actionTriggered=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_SPACE);
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F))
        {
            auto* target=InteractionSystem::Nearest(m_Actor,m_Targets);
            if(target && !m_World.HasLineOfSight(m_Actor.position,target->position)) target=nullptr;
            m_LastAction=InteractionSystem::Use(m_Actor,target,m_SelectedItem,m_Events);
            if(m_LastAction==InteractionResult::Success) actionTriggered=true;
            m_FeedbackTime=0.6f;
            if(m_LastAction==InteractionResult::Success && target && target->kind==TargetKind::FruitTree)
            {
                ++m_FruitHarvested;
                if(auto* window=glfwGetCurrentContext())
                {
                    const std::string title="My Farm - Phase F | Apples: "+std::to_string(m_FruitHarvested)+
                        " | F: Harvest | F7: Orchard | F8: Depth";
                    glfwSetWindowTitle(window,title.c_str());
                }
            }
            if(m_LastAction==InteractionResult::Success && target &&
               target->kind==TargetKind::Object && !target->active)
                m_World.SetObjectActive(target->id,false);
        }
        m_Effects.Consume(m_Events.Take());m_Effects.Update(deltaTime);
        m_FeedbackTime=std::max(0.0f,m_FeedbackTime-deltaTime);
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_TAB)) m_Character = 1-m_Character;
        const bool hurtTriggered=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_H);
        m_PlayerState=ResolvePlayerState(m_Moving,running,actionTriggered,hurtTriggered,
                                         m_PlayerState,m_PlayerAnimation.Finished());
        m_PlayerAnimation.Select(m_Visuals,m_Character==0?"player_linh":"player_an",m_PlayerState,m_Facing,
                                 actionTriggered && m_PlayerState==VisualState::Action);
        m_PlayerAnimation.Update(deltaTime,false);

        m_NpcTime+=deltaTime;
        const auto npcState=std::fmod(m_NpcTime,8.0f)>5.5f?VisualState::Action:
                            (std::fmod(m_NpcTime,8.0f)>2.5f?VisualState::Walk:VisualState::Idle);
        const auto npcFacing=std::fmod(m_NpcTime,8.0f)>4.0f?Facing::Left:Facing::Right;
        const char* npcProfile=m_Character==0?"npc_an":"npc_linh";
        if(!m_NpcAnimation.Select(m_Visuals,npcProfile,npcState,npcState==VisualState::Walk?npcFacing:Facing::Down))
            m_NpcAnimation.Select(m_Visuals,npcProfile,VisualState::Idle,Facing::Down);
        m_NpcAnimation.Update(deltaTime,false);

        m_DogTime+=deltaTime;
        const float dogDistance=glm::length(m_Player-m_Dog);
        const auto dogState=dogDistance>180?VisualState::Run:(dogDistance>80?VisualState::Walk:
                            (std::fmod(m_DogTime,12.0f)>8?VisualState::Sleep:VisualState::Idle));
        m_DogAnimation.Select(m_Visuals,"dog",dogState,FacingFromVelocity(m_Player.x-m_Dog.x,m_Player.y-m_Dog.y,Facing::Down));
        m_DogAnimation.Update(deltaTime,false);
        if(dogState==VisualState::Walk || dogState==VisualState::Run)
        {
            const std::array<DynamicCollider,2> dogObstacles={{{NpcPosition,NpcCollider},{m_Player,PlayerCollider}}};
            m_Dog=CollisionSystem::MoveAndSlide(m_World,m_Dog,m_Player-m_Dog,
                dogState==VisualState::Run?210.0f:120.0f,deltaTime,DogCollider,dogObstacles);
        }
        m_GameAge+=deltaTime;
        for (int i=0; i<4; ++i)
            if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_1+i))
            {
                m_SelectedItem = i;
                if (auto* window = glfwGetCurrentContext())
                {
                    const std::string title = std::string("My Farm - Phase F | ") + ItemNames[i] +
                        " | Apples: " + std::to_string(m_FruitHarvested) +
                        " | WASD: Move | F: Interact | F7: Orchard | F8: Depth";
                    glfwSetWindowTitle(window, title.c_str());
                }
            }
        m_Camera.Follow(m_Player, deltaTime);
    }
    void DemoScene2D::DrawActor(MyEngine::Canvas2D& canvas,int character,glm::vec2 feet,
                                const AnimationController& animation,float bob)
    {
        // The selected clip owns its texture and UV; gameplay never depends on sprite dimensions.
        canvas.Rectangle(feet.x-22, feet.y-4, 44, 9, {0.08f,0.16f,0.09f,0.19f});
        const auto* texture=m_Characters[static_cast<std::size_t>(character)];
        const auto anchor=m_Anchors[static_cast<std::size_t>(character)];
        std::array<float,4> uv{0,0,1,1};
        if(const auto* clip=animation.Clip();clip && m_Assets)
        {
            if(const auto* loaded=m_Assets->Loaded(clip->assetId))
                if(const auto* record=m_Assets->Record(clip->assetId))
                    if(const auto region=AssetManifest::FrameUv(*record,animation.Frame()-record->firstFrame))
                    {
                        texture=loaded;uv=*region;
                        if(clip->flipX){uv[0]+=uv[2];uv[2]=-uv[2];}
                    }
        }
        if(texture) canvas.Sprite(*texture, feet.x-156*anchor.x, feet.y-156*anchor.y-bob, 156,156,uv);
        else canvas.Rectangle(feet.x-22,feet.y-78-bob,44,78,
            character==0?std::array<float,4>{0.85f,0.65f,0.22f,1}:std::array<float,4>{0.28f,0.5f,0.8f,1});
    }
    void DemoScene2D::DrawDog(MyEngine::Canvas2D& canvas) const
    {
        const auto state=m_DogAnimation.State();
        if(m_DogTexture && m_Assets)
        {
            std::array<float,4> uv{0,0,0.5f,0.5f};
            if(const auto* record=m_Assets->Record("dog_walk"))
                if(const auto region=AssetManifest::FrameUv(*record,
                   state==VisualState::Walk || state==VisualState::Run?m_DogAnimation.Frame():0)) uv=*region;
            canvas.Sprite(*m_DogTexture,m_Dog.x-52,m_Dog.y-87,104,
                          state==VisualState::Sleep?88.0f:104.0f,uv);
            return;
        }
        const float height=state==VisualState::Sleep?24.0f:42.0f;
        canvas.Rectangle(m_Dog.x-25,m_Dog.y-height,50,height,{0.55f,0.32f,0.16f,1});
        canvas.Rectangle(m_Dog.x+16,m_Dog.y-height-5,18,18,{0.68f,0.43f,0.22f,1});
    }
    void DemoScene2D::DrawCrop(MyEngine::Canvas2D& canvas) const
    {
        const auto it=std::find_if(m_Targets.begin(),m_Targets.end(),[](const InteractionTarget& target){
            return target.id=="crop.plot.01";});
        if(it==m_Targets.end() || !it->active) return;
        if(!it->planted) return;
        const int stage=GrowthFrame(it->age,{1,1,1,2});
        if(m_CropTexture && m_Assets)
            if(const auto* record=m_Assets->Record("turnip_growth"))
                if(const auto uv=AssetManifest::FrameUv(*record,stage))
                {
                    canvas.Sprite(*m_CropTexture,it->position.x-56,it->position.y-94,112,112,*uv);
                    return;
                }
        const float size=14.0f+static_cast<float>(stage)*7.0f;
        const std::array<std::array<float,4>,4> colors={{{0.48f,0.72f,0.22f,1},{0.31f,0.68f,0.18f,1},
            {0.22f,0.58f,0.15f,1},{0.65f,0.82f,0.25f,1}}};
        canvas.Rectangle(it->position.x-size*0.5f,it->position.y-size,size,size,colors[static_cast<std::size_t>(stage)]);
    }
    void DemoScene2D::Render(MyEngine::Canvas2D& canvas, int width, int height)
    {
        if (!m_Ready) return;
        m_Camera.Resize(width,height);
        auto renderCamera=m_Camera;
        const float shake=m_Effects.Shake();
        renderCamera.SetPosition(m_Camera.Position()+glm::vec2{
            std::sin(static_cast<float>(m_GameAge)*87.0f)*shake,
            std::cos(static_cast<float>(m_GameAge)*71.0f)*shake});
        canvas.BeginWorld(width,height,renderCamera);
        const auto range=m_World.VisibleTiles(renderCamera.Position(),renderCamera.ViewSize());
        const auto chunks=m_World.VisibleChunks(renderCamera.Position(),renderCamera.ViewSize());
        constexpr std::array<std::array<float,4>,5> colors={{{0.43f,0.65f,0.32f,1},
            {0.78f,0.68f,0.49f,1},{0.55f,0.38f,0.24f,1},{0.21f,0.51f,0.7f,1},
            {0.23f,0.48f,0.26f,1}}};
        for(int cy=chunks.top;cy<chunks.bottom;++cy)
            for(int cx=chunks.left;cx<chunks.right;++cx)
            {
                const auto chunk=m_World.ChunkAt(cx,cy);
                if(m_Map)
                {
                    const float x=static_cast<float>(chunk.tiles.left*WorldMap::TileSize);
                    const float y=static_cast<float>(chunk.tiles.top*WorldMap::TileSize);
                    const float w=static_cast<float>((chunk.tiles.right-chunk.tiles.left)*WorldMap::TileSize);
                    const float h=static_cast<float>((chunk.tiles.bottom-chunk.tiles.top)*WorldMap::TileSize);
                    canvas.Sprite(*m_Map,x,y,w,h,{x/WorldSize.x,y/WorldSize.y,w/WorldSize.x,h/WorldSize.y});
                    continue;
                }
                for(int y=std::max(range.top,chunk.tiles.top);y<std::min(range.bottom,chunk.tiles.bottom);++y)
                    for(int x=std::max(range.left,chunk.tiles.left);x<std::min(range.right,chunk.tiles.right);++x)
                    {
                        const auto* cell=m_World.TryCell(x,y);
                        if(cell) canvas.Rectangle(static_cast<float>(x*WorldMap::TileSize),
                            static_cast<float>(y*WorldMap::TileSize),WorldMap::TileSize+0.5f,
                            WorldMap::TileSize+0.5f,colors[static_cast<std::size_t>(cell->terrain)]);
                    }
            }
        const auto visibleOrigin=renderCamera.Position()-renderCamera.ViewSize()*0.5f;
        m_RenderOrder.Begin(visibleOrigin,renderCamera.ViewSize());
        const auto crop=std::find_if(m_Targets.begin(),m_Targets.end(),[](const InteractionTarget& target){
            return target.id=="crop.plot.01";});
        if(crop!=m_Targets.end() && crop->active && crop->planted)
            m_RenderOrder.Submit(RenderKind::Crop,0,crop->position-glm::vec2{56,94},{112,112},crop->position.y);
        const auto pickup=std::find_if(m_Targets.begin(),m_Targets.end(),[](const InteractionTarget& target){
            return target.id=="item.demo.01";});
        if(pickup!=m_Targets.end() && pickup->active)
            m_RenderOrder.Submit(RenderKind::Pickup,0,pickup->position-glm::vec2{26,40},{52,52},pickup->position.y);
        if(m_Map)
            for(std::size_t i=0;i<m_Trees.Entries().size();++i)
            {
                const auto& tree=m_Trees.Entries()[i];
                m_RenderOrder.Submit(RenderKind::TreeCanopy,i,tree.canopyOrigin,tree.canopySize,tree.footY);
            }
        else
            for(std::size_t i=0;i<m_World.Objects().size();++i)
            {
                const auto& object=m_World.Objects()[i];
                if(object.active) m_RenderOrder.Submit(RenderKind::WorldObject,i,object.position,object.size,
                                                       object.position.y+object.size.y);
            }
        m_RenderOrder.Submit(RenderKind::Actor,0,m_Player-glm::vec2{78,156},{156,156},m_Player.y);
        m_RenderOrder.Submit(RenderKind::Actor,1,NpcPosition-glm::vec2{78,156},{156,156},NpcPosition.y);
        m_RenderOrder.Submit(RenderKind::Animal,0,m_Dog-glm::vec2{52,87},{104,104},m_Dog.y);
        m_RenderOrder.Sort();
        const float bob = m_Moving ? std::sin(m_WalkTime*12.0f)*2.2f : 0;
        for(const auto& entry:m_RenderOrder.Entries())
        {
            switch(entry.kind)
            {
            case RenderKind::Crop:DrawCrop(canvas);break;
            case RenderKind::Pickup:
                if(m_Items) canvas.Sprite(*m_Items,pickup->position.x-26,pickup->position.y-40,52,52,m_ItemRegions[2]);
                else canvas.Rectangle(pickup->position.x-12,pickup->position.y-12,24,24,{0.94f,0.78f,0.3f,1});
                break;
            case RenderKind::Actor:
                if(entry.index==0) DrawActor(canvas,m_Character,m_Player,m_PlayerAnimation,bob);
                else DrawActor(canvas,1-m_Character,NpcPosition,m_NpcAnimation,0);
                break;
            case RenderKind::Animal:DrawDog(canvas);break;
            case RenderKind::TreeCanopy:
            {
                const auto& tree=m_Trees.Entries()[entry.index];
                const auto x=tree.canopyOrigin.x,y=tree.canopyOrigin.y;
                const auto w=tree.canopySize.x,h=tree.canopySize.y;
                canvas.SpriteEllipse(*m_Map,x,y,w,h,{x/WorldSize.x,y/WorldSize.y,w/WorldSize.x,h/WorldSize.y});
                break;
            }
            case RenderKind::WorldObject:
            {
                const auto& object=m_World.Objects()[entry.index];
                const std::array<float,4> color=object.visualId=="visual.water.ripple"?
                    std::array<float,4>{0.65f,0.86f,0.96f,0.6f}:
                    (object.blocked?std::array<float,4>{0.35f,0.37f,0.24f,1}:
                                    std::array<float,4>{0.69f,0.52f,0.23f,1});
                canvas.Rectangle(object.position.x,object.position.y,object.size.x,object.size.y,color);
                break;
            }
            }
        }
        for(const auto& effect:m_Effects.Active())
        {
            if(!WorldMap::Intersects(effect.position-glm::vec2{6,6},{12,12},visibleOrigin,renderCamera.ViewSize())) continue;
            const float alpha=1.0f-effect.age/effect.lifetime;
            const float size=effect.kind==EffectKind::FloatingText?16.0f:8.0f;
            canvas.Rectangle(effect.position.x-size*0.5f,effect.position.y-size*0.5f,size,size,
                effect.kind==EffectKind::FloatingText?std::array<float,4>{0.98f,0.86f,0.3f,alpha}:
                                                      std::array<float,4>{0.95f,0.72f,0.42f,alpha});
        }
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
        canvas.Rectangle(16,105,132,8,{0.17f,0.22f,0.14f,1});
        canvas.Rectangle(16,105,132.0f*std::clamp(m_Actor.stamina/20.0f,0.0f,1.0f),8,{0.3f,0.7f,0.3f,1});
        if(m_FeedbackTime>0)
            canvas.Rectangle(static_cast<float>(width)-44,16,28,28,
                m_LastAction==InteractionResult::Success?std::array<float,4>{0.25f,0.78f,0.35f,m_FeedbackTime}:
                                                         std::array<float,4>{0.85f,0.25f,0.19f,m_FeedbackTime});
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
        m_Map=m_Items=m_DogTexture=m_CropTexture=nullptr;
        m_Assets=nullptr;
        m_Characters.fill(nullptr);
        m_Trees={};m_RenderOrder={};
        m_Visuals={};m_PlayerAnimation={};m_NpcAnimation={};m_DogAnimation={};m_CropAnimation={};
        m_Targets.clear();m_Events.Clear();m_Effects.Clear();
    }
}
