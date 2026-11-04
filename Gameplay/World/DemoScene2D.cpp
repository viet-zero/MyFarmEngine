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

    constexpr glm::vec2 NpcPosition{2360,1950};
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
        m_Ready=true; // Missing artwork uses geometric placeholders, not a fatal game error.
        Reset();
        return m_Ready;
    }
    void DemoScene2D::Reset()
    {
        m_Player = m_World.Spawn(Zone::Farm);
        m_Dog=m_Player+glm::vec2{-70,30};
        CancelInteraction();
        const auto nextAction=m_Actor.nextActionId;
        m_Actor={};m_Actor.nextActionId=nextAction;m_Actor.position=m_Player;m_Action={};
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
        m_Events.Clear();m_Effects.Clear();m_FeedbackTime=0;m_LastAction=InteractionResult::NoTarget;
        m_Npc=NpcPosition;m_NpcFacing=Facing::Down;m_NpcPatrolRight=true;
        m_GameAge=0;
        m_Moving = false;
        m_Facing=Facing::Down;m_PlayerState=VisualState::Idle;
        m_PlayerAnimation={};m_NpcAnimation={};m_DogAnimation={};m_CropAnimation={};
        m_PlayerAnimation.Select(m_Visuals,m_Character==0?"player_linh":"player_an",m_PlayerState,m_Facing);
        m_NpcAnimation.Select(m_Visuals,"npc_an",VisualState::Idle,Facing::Down);
        m_DogAnimation.Select(m_Visuals,"dog",VisualState::Idle,Facing::Down);
        m_CropAnimation.Select(m_Visuals,"crop_turnip",VisualState::Idle,Facing::Any);
        m_Camera.SetWorldSize(WorldSize);
        m_Camera.SetZoom(0.85f);
        m_Camera.SetPosition(m_Player);
    }
    void DemoScene2D::SetOptions(SceneOptions options)
    {
        m_Options=options;
        std::cout<<"Scene options: demo="<<options.demoMode<<" npcAI="<<options.npcAI
                 <<" petFollow="<<options.petFollow<<" overlay="<<options.debugOverlay<<'\n';
    }
    SceneSnapshot DemoScene2D::Snapshot() const
    {
        return {m_Player,m_Npc,m_Dog,m_PlayerState,m_NpcAnimation.State(),m_DogAnimation.State(),
                m_PlayerAnimation.Frame(),m_NpcAnimation.Frame(),m_DogAnimation.Frame(),
                m_Actor.inventoryCount,m_FruitHarvested,m_Action.id,m_Action.phase,m_LastAction,m_Effects.Active().size()};
    }
    std::optional<InteractionTarget> DemoScene2D::TargetState(std::string_view id) const
    {
        for(const auto& target:m_Targets) if(target.id==id) return target;
        return std::nullopt;
    }
    void DemoScene2D::CancelInteraction(InteractionResult reason)
    {
        if(!m_Action.Pending()) return;
        InteractionSystem::Cancel(m_Actor,m_Action,reason);
        m_LastAction=m_Action.result;m_FeedbackTime=0.6f;m_PlayerState=VisualState::Idle;
        m_PlayerAnimation.Select(m_Visuals,m_Character==0?"player_linh":"player_an",VisualState::Idle,m_Facing,true);
    }
    void DemoScene2D::Update(float deltaTime)
    {
        SceneInput input;
        input.movement={
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_D)?1.0f:0.0f)-(MyEngine::Input::IsKeyPressed(GLFW_KEY_A)?1.0f:0.0f),
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_S)?1.0f:0.0f)-(MyEngine::Input::IsKeyPressed(GLFW_KEY_W)?1.0f:0.0f)};
        input.running=MyEngine::Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT);
        input.interact=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F);
        input.cancel=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_C);
        if(auto* window=glfwGetCurrentContext()) input.paused=!glfwGetWindowAttrib(window,GLFW_FOCUSED);
        input.reset=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_R);
        input.action=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_SPACE);
        input.hurt=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_H);
        input.switchCharacter=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_TAB);
        for(int key=GLFW_KEY_F4;key<=GLFW_KEY_F8;++key)
            if(MyEngine::Input::ConsumeKeyPress(key)) input.teleport=key-GLFW_KEY_F4+4;
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_Q)) input.zoom=-1;
        if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_E)) input.zoom=1;
        for(int i=0;i<4;++i) if(MyEngine::Input::ConsumeKeyPress(GLFW_KEY_1+i)) input.selectedItem=i;
        UpdateSimulation(deltaTime,input);
    }
    void DemoScene2D::UpdateSimulation(float deltaTime,const SceneInput& input)
    {
        if(!m_Ready) return;
        const float dt=std::isfinite(deltaTime)?std::clamp(deltaTime,0.0f,0.1f):0.0f;
        if(input.reset){Reset();return;}
        m_Actor.uiCapturesInput=input.paused;
        if(input.paused)
        {
            m_PlayerAnimation.Update(0,true);m_NpcAnimation.Update(0,true);m_DogAnimation.Update(0,true);
            return;
        }
        const bool developer=m_Options.demoMode || m_Options.debugOverlay;
        const bool teleporting=developer && input.teleport>=4 && input.teleport<=8;
        const bool hurtTriggered=developer && input.hurt;
        if(input.cancel || hurtTriggered || (developer && input.switchCharacter) || teleporting) CancelInteraction();
        InteractionSystem::Tick(m_Actor,m_Targets,dt);
        InteractionSystem::TickAction(m_Actor,m_Action,dt);
        if(input.zoom) m_Camera.SetZoom(m_Camera.Zoom()*(input.zoom>0?1.15f:1.0f/1.15f));
        if(developer && input.switchCharacter) m_Character=1-m_Character;
        if(input.selectedItem>=0 && input.selectedItem<4) m_SelectedItem=input.selectedItem;
        const bool locked=(m_PlayerState==VisualState::Action || m_PlayerState==VisualState::Hurt) &&
                          !m_PlayerAnimation.Finished();
        if(!locked && !m_Action.Pending()) m_Facing=FacingFromVelocity(input.movement.x,input.movement.y,m_Facing);
        std::array<DynamicCollider,2> obstacles={{{m_Dog,DogCollider},{m_Npc,NpcCollider}}};
        const std::span<const DynamicCollider> playerObstacles(obstacles.data(),m_Options.demoMode?2u:1u);
        const auto previous=m_Player;
        m_Player=CollisionSystem::MoveAndSlide(m_World,m_Player,m_Action.Pending()?glm::vec2{0,0}:input.movement,input.running?300.0f:180.0f,
                                               dt,PlayerCollider,playerObstacles);
        m_Moving=glm::length(m_Player-previous)>0.001f;
        if(developer && input.teleport>=4 && input.teleport<=8)
        {
            glm::vec2 offset{-70,30};
            if(input.teleport<=6) m_Player=m_World.Spawn(static_cast<Zone>(input.teleport-4));
            else if(input.teleport==7){m_Player={1990,1740};m_Facing=Facing::Up;}
            else {m_Player={2140,1450};m_Facing=Facing::Left;offset={100,30};}
            m_Dog=m_Player+offset;m_Camera.SetPosition(m_Player);m_Moving=false;
        }
        m_Actor.position=m_Player;
        switch(m_Facing)
        {
        case Facing::Left:m_Actor.facing={-1,0};break;
        case Facing::Right:m_Actor.facing={1,0};break;
        case Facing::Up:m_Actor.facing={0,-1};break;
        default:m_Actor.facing={0,1};break;
        }
        bool actionTriggered=developer && input.action && !m_Action.Pending();
        bool started=false;
        if(input.interact && !hurtTriggered && !input.cancel && !teleporting)
        {
            auto* target=InteractionSystem::Nearest(m_Actor,m_Targets);
            if(target && !m_World.HasLineOfSight(m_Actor.position,target->position)) target=nullptr;
            const auto* clip=m_Visuals.Find(m_Character==0?"player_linh":"player_an",VisualState::Action,m_Facing);
            if(m_Action.Pending()) m_LastAction=InteractionResult::Busy;
            else if(!clip) m_LastAction=InteractionResult::NoAnimation;
            else m_LastAction=InteractionSystem::Start(m_Actor,target,m_SelectedItem,m_Action);
            if(m_LastAction==InteractionResult::Success){started=true;actionTriggered=true;}
            m_FeedbackTime=0.6f;
        }
        m_PlayerState=ResolvePlayerState(m_Moving,input.running,actionTriggered,hurtTriggered,
                                         m_PlayerState,m_PlayerAnimation.Finished());
        const bool selected=m_PlayerAnimation.Select(m_Visuals,m_Character==0?"player_linh":"player_an",m_PlayerState,m_Facing,
            (actionTriggered && m_PlayerState==VisualState::Action) || (hurtTriggered && m_PlayerState==VisualState::Hurt));
        if(started && !selected){CancelInteraction(InteractionResult::NoAnimation);started=false;}
        if(!started) InteractionSystem::Activate(m_Actor,m_Action);
        // The press frame is Start only. Never mutate the model on the initiating input frame.
        m_PlayerAnimation.Update(started?0.0f:dt,false);
        for(const auto& hit:m_PlayerAnimation.MarkerHits())
        {
            if(!m_Action.Pending() || m_PlayerState!=VisualState::Action) continue;
            if(hit.name=="IMPACT")
            {
                const auto target=TargetState(m_Action.targetId);
                const bool visible=target && m_World.HasLineOfSight(m_Actor.position,target->position);
                m_LastAction=InteractionSystem::Impact(m_Actor,m_Action,m_Targets,m_Events,visible);
                m_FeedbackTime=0.6f;
                if(m_LastAction==InteractionResult::Success)
                {
                    if(m_Action.kind==TargetKind::FruitTree) ++m_FruitHarvested;
                    if(m_Action.kind==TargetKind::Object)
                        if(const auto changed=TargetState(m_Action.targetId);changed && !changed->active)
                            m_World.SetObjectActive(changed->id,false);
                }
            }
            else if(hit.name=="COMPLETE") InteractionSystem::Complete(m_Actor,m_Action);
        }
        if(m_Action.Pending() && m_PlayerAnimation.Finished() && !m_Action.impactConsumed)
            CancelInteraction(InteractionResult::NoAnimation);
        m_Effects.Consume(m_Events.Take());m_Effects.Update(dt);
        m_FeedbackTime=std::max(0.0f,m_FeedbackTime-dt);

        // The old walk/action-in-place timer is gone. Patrol is explicit, opt-in demo behavior.
        const auto npcBefore=m_Npc;
        if(m_Options.demoMode && m_Options.npcAI)
        {
            const glm::vec2 goal=NpcPosition+glm::vec2{m_NpcPatrolRight?160.0f:0.0f,0};
            const auto direction=goal-m_Npc;
            const std::array<DynamicCollider,2> blocked={{{m_Player,PlayerCollider},{m_Dog,DogCollider}}};
            m_Npc=CollisionSystem::MoveAndSlide(m_World,m_Npc,direction,65.0f,dt,NpcCollider,blocked);
            if(glm::length(goal-m_Npc)<8) m_NpcPatrolRight=!m_NpcPatrolRight;
        }
        const bool npcMoving=glm::length(m_Npc-npcBefore)>0.001f;
        if(npcMoving) m_NpcFacing=FacingFromVelocity(m_Npc.x-npcBefore.x,m_Npc.y-npcBefore.y,m_NpcFacing);
        m_NpcAnimation.Select(m_Visuals,"npc_an",npcMoving?VisualState::Walk:VisualState::Idle,m_NpcFacing);
        m_NpcAnimation.Update(dt,false);

        const auto dogBefore=m_Dog;
        const float distance=glm::length(m_Player-m_Dog);
        const bool dogRunning=distance>180;
        if(m_Options.petFollow && distance>80)
        {
            const std::array<DynamicCollider,2> blocked={{{m_Player,PlayerCollider},{m_Npc,NpcCollider}}};
            m_Dog=CollisionSystem::MoveAndSlide(m_World,m_Dog,m_Player-m_Dog,dogRunning?210.0f:120.0f,
                dt,DogCollider,std::span<const DynamicCollider>(blocked.data(),m_Options.demoMode?2u:1u));
        }
        const bool dogMoving=glm::length(m_Dog-dogBefore)>0.001f;
        m_DogAnimation.Select(m_Visuals,"dog",dogMoving?(dogRunning?VisualState::Run:VisualState::Walk):VisualState::Idle,
                             FacingFromVelocity(m_Dog.x-dogBefore.x,m_Dog.y-dogBefore.y,Facing::Down));
        m_DogAnimation.Update(dt,false);
        m_GameAge+=dt;m_Camera.Follow(m_Player,dt);
    }
    DemoScene2D::SpriteDraw DemoScene2D::SpriteFor(std::string_view id,int frame,glm::vec2 feet,float height,bool flip) const
    {
        SpriteDraw sprite;
        if(!m_Assets) return sprite;
        sprite.texture=m_Assets->Loaded(id);
        const auto* record=m_Assets->Record(id);
        const auto* bounds=m_Assets->FrameBounds(id,frame);
        if(sprite.texture && record && bounds)
        {
            if(const auto uv=AssetManifest::FrameUv(*record,frame))
            {
                sprite.uv=*uv;
                if(flip){sprite.uv[0]+=sprite.uv[2];sprite.uv[2]=-sprite.uv[2];}
                sprite.placement=PlaceSprite(*bounds,feet,height,flip);
                return sprite;
            }
        }
        sprite.texture=nullptr;
        sprite.placement={feet-glm::vec2{height*0.2f,height},{height*0.4f,height},
                          feet-glm::vec2{height*0.2f,height},{height*0.4f,height}};
        return sprite;
    }
    DemoScene2D::SpriteDraw DemoScene2D::ActorSprite(int character,glm::vec2 feet,const AnimationController& animation) const
    {
        if(const auto* clip=animation.Clip();clip && m_Assets)
            if(const auto* record=m_Assets->Record(clip->assetId);record && m_Assets->Loaded(clip->assetId))
                return SpriteFor(clip->assetId,animation.Frame()-record->firstFrame,feet,HumanHeight,clip->flipX);
        return SpriteFor(character==0?"player_linh":"player_an",0,feet,HumanHeight);
    }
    void DemoScene2D::DrawActor(MyEngine::Canvas2D& canvas,int character,glm::vec2 feet,const AnimationController& animation)
    {
        const auto sprite=ActorSprite(character,feet,animation);
        const auto& p=sprite.placement;
        canvas.Rectangle(feet.x-22,feet.y-4,44,9,{0.08f,0.16f,0.09f,0.19f});
        if(sprite.texture) canvas.Sprite(*sprite.texture,p.origin.x,p.origin.y,p.size.x,p.size.y,sprite.uv);
        else canvas.Rectangle(p.origin.x,p.origin.y,p.size.x,p.size.y,{0.65f,0.5f,0.27f,1});
        if(m_Options.debugOverlay) canvas.Rectangle(feet.x-2,feet.y-2,4,4,{1,0.2f,0.15f,1});
    }
    void DemoScene2D::DrawDog(MyEngine::Canvas2D& canvas) const
    {
        const auto sprite=SpriteFor("dog_walk",m_DogAnimation.Frame(),m_Dog,PetHeight);
        const auto& p=sprite.placement;
        if(sprite.texture) canvas.Sprite(*sprite.texture,p.origin.x,p.origin.y,p.size.x,p.size.y,sprite.uv);
        else canvas.Rectangle(p.origin.x,p.origin.y,p.size.x,p.size.y,{0.55f,0.32f,0.16f,1});
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
        const auto player=ActorSprite(m_Character,m_Player,m_PlayerAnimation).placement;
        m_RenderOrder.Submit(RenderKind::Actor,0,player.visibleOrigin,player.visibleSize,m_Player.y);
        if(m_Options.demoMode)
        {
            const auto npc=ActorSprite(1,m_Npc,m_NpcAnimation).placement;
            m_RenderOrder.Submit(RenderKind::Actor,1,npc.visibleOrigin,npc.visibleSize,m_Npc.y);
        }
        const auto pet=SpriteFor("dog_walk",m_DogAnimation.Frame(),m_Dog,PetHeight).placement;
        m_RenderOrder.Submit(RenderKind::Animal,0,pet.visibleOrigin,pet.visibleSize,m_Dog.y);
        m_RenderOrder.Sort();
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
                if(entry.index==0) DrawActor(canvas,m_Character,m_Player,m_PlayerAnimation);
                else DrawActor(canvas,1,m_Npc,m_NpcAnimation);
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
        const auto bar=HotbarLayout::ForViewport(width,height);
        const float unit=bar.slot/64.0f;
        canvas.Rectangle(bar.x,bar.y,bar.width,bar.height,{0.20f,0.24f,0.16f,0.94f});
        canvas.Rectangle(bar.x+2*unit,bar.y+2*unit,bar.width-4*unit,bar.height-4*unit,{0.89f,0.85f,0.69f,0.98f});
        for(int i=0;i<4;++i)
        {
            const float x=bar.x+bar.padding+i*(bar.slot+bar.gap),y=bar.y+bar.padding;
            const bool selected=i==m_SelectedItem;
            canvas.Rectangle(x,y,bar.slot,bar.slot,selected?
                std::array<float,4>{0.69f,0.47f,0.20f,1}:std::array<float,4>{0.57f,0.62f,0.43f,1});
            canvas.Rectangle(x+2*unit,y+2*unit,bar.slot-4*unit,bar.slot-4*unit,
                             selected?std::array<float,4>{1,0.96f,0.79f,1}:std::array<float,4>{0.95f,0.93f,0.82f,1});
            const float inset=10*unit;
            if(m_Items && m_Assets)
            {
                if(const auto* frame=m_Assets->FrameBounds("items_farm",i))
                {
                    const auto icon=FitSprite(*frame,{x+inset,y+inset},{bar.slot-2*inset,bar.slot-2*inset});
                    canvas.Sprite(*m_Items,icon.origin.x,icon.origin.y,icon.size.x,icon.size.y,m_ItemRegions[static_cast<std::size_t>(i)]);
                }
            }
            else canvas.Rectangle(x+inset,y+inset,bar.slot-2*inset,bar.slot-2*inset,{0.65f,0.35f,0.16f,1});
            // Small index marks remain readable without introducing an unrelated font renderer.
            for(int mark=0;mark<=i;++mark)
                canvas.Rectangle(x+5*unit+mark*4*unit,y+bar.slot-6*unit,2*unit,2*unit,{0.39f,0.37f,0.23f,1});
        }
        const float meterWidth=bar.width*0.4f,meterX=bar.x+(bar.width-meterWidth)*0.5f;
        canvas.Rectangle(meterX,bar.y-9*unit,meterWidth,5*unit,{0.17f,0.22f,0.14f,1});
        canvas.Rectangle(meterX,bar.y-9*unit,meterWidth*std::clamp(m_Actor.stamina/20.0f,0.0f,1.0f),5*unit,{0.42f,0.67f,0.36f,1});
        if(m_FeedbackTime>0)
            canvas.Rectangle(bar.x+bar.width-6*unit,bar.y-9*unit,6*unit,5*unit,
                m_LastAction==InteractionResult::Success?std::array<float,4>{0.25f,0.78f,0.35f,m_FeedbackTime}:
                                                         std::array<float,4>{0.85f,0.25f,0.19f,m_FeedbackTime});
        if(m_Options.debugOverlay)
        {
            canvas.Rectangle(16,16,132,76,{0.99f,0.97f,0.88f,0.92f});
            for(int i=0;i<2;++i)
            {
                const float x=20+i*62.0f;
                const char* id=i==0?"player_linh":"player_an";
                const auto* portrait=m_Characters[static_cast<std::size_t>(i)];
                const auto* frame=m_Assets?m_Assets->FrameBounds(id,0):nullptr;
                if(portrait && frame)
                {
                    const auto p=FitSprite(*frame,{x,20},{58,58});
                    canvas.Sprite(*portrait,p.origin.x,p.origin.y,p.size.x,p.size.y);
                }
                if(i==m_Character) canvas.Rectangle(x,82,58,3,{0.71f,0.46f,0.14f,1});
            }
        }
        canvas.End();
    }
    void DemoScene2D::Shutdown()
    {
        CancelInteraction();
        m_Ready = false;
        m_Map=m_Items=m_DogTexture=m_CropTexture=nullptr;
        m_Assets=nullptr;
        m_Characters.fill(nullptr);
        m_Trees={};m_RenderOrder={};
        m_Visuals={};m_PlayerAnimation={};m_NpcAnimation={};m_DogAnimation={};m_CropAnimation={};
        m_Targets.clear();m_Events.Clear();m_Effects.Clear();
    }
}
