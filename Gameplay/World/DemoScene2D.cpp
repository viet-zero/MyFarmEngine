#include "DemoScene2D.h"
#include "Gameplay/Graphics/HudText.h"
#include "Gameplay/Systems/CollisionSystem.h"
#include "Input/Input.h"
#include "Resources/AssetPath.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <iostream>

namespace
{
    const glm::vec2 WorldSize=MyGame::WorldMap::Size();

    constexpr glm::vec2 NpcPosition{2360,1950};
    constexpr std::array<glm::vec2,6> GardenCells{{
        {2208,2016},{2272,2016},{2336,2016},{2208,2080},{2272,2080},{2336,2080}
    }};
    std::optional<glm::ivec2> TileAt(glm::vec2 position)
    {
        const auto bounds=MyGame::WorldMap::Size();
        if(!std::isfinite(position.x) || !std::isfinite(position.y) ||
           position.x<0 || position.y<0 || position.x>=bounds.x || position.y>=bounds.y) return std::nullopt;
        return glm::ivec2{position/static_cast<float>(MyGame::WorldMap::TileSize)};
    }
    int TileKey(glm::ivec2 cell)
    {
        return cell.y*MyGame::WorldMap::Width+cell.x;
    }
    std::string GardenCellId(std::size_t index)
    {
        return "crop.home."+std::to_string(index+1);
    }
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
        // The reviewed pilot is staged beside the executable, outside the shipped catalog.
        const auto review=dataRoot.parent_path()/"LocalReview";
        const auto loadReview=[&](MyEngine::Texture2D& texture,const char* name)
        {
            const auto path=review/name;
            std::error_code error;
            if(std::filesystem::is_regular_file(path,error) && !texture.Load(path))
                std::cerr<<"Local review image unavailable: "<<name<<'\n';
        };
        constexpr std::array carrotFrames{"carrot_sprout.png","carrot_young.png","carrot_mature.png"};
        for(std::size_t i=0;i<carrotFrames.size();++i) loadReview(m_LocalCarrot[i],carrotFrames[i]);
        loadReview(m_LocalCarrotIcon,"carrot_icon.png");
        loadReview(m_LocalSeedIcon,"seed_bag.png");
        loadReview(m_LocalSoil[0],"soil_dry.png");
        loadReview(m_LocalSoil[1],"soil_wet.png");
        m_Items=assets.Load("items_farm");
        if(const auto* items=assets.Record("items_farm"))
            for(int i=0;i<4;++i)
            {
                const auto uv=AssetManifest::FrameUv(*items,i);
                if(uv) m_ItemRegions[static_cast<std::size_t>(i)]=*uv;
                else {m_Items=nullptr;std::cerr<<"Asset fallback [items_farm]: four_item_regions_required\n";}
            }
        if(m_WorldArt.Load(dataRoot.parent_path()/"LocalV4"))
        {
            CacheTerrainArt();
            std::cout<<"World art v4 enabled: four terrain palettes and eight-direction character\n";
        }
        m_Ready=true; // Missing artwork uses geometric placeholders, not a fatal game error.
        if(!m_Scale.Load(dataRoot/"VisualScale.csv")) std::cerr<<"VisualScale missing/invalid: using defaults\n";
        if(m_WorldArt.LoadCozy(dataRoot.parent_path()/"Cozy")) std::cout<<"Cozy art v1 enabled\n";
        Reset();
        return m_Ready;
    }
    void DemoScene2D::Reset()
    {
        m_GardenControl=false;m_HoverWorld.reset();m_HavePickCamera=false;
        m_Player = m_World.Spawn(Zone::Farm);
        m_Dog=m_Player+glm::vec2{-70,30};
        CancelInteraction();
        const auto nextAction=m_Actor.nextActionId;
        m_Actor={};m_Actor.home=HomeStorage::PlayerGarden();m_Hotbar={};m_StorageOpen=false;m_StorageRow=0;m_StorageResult=TradeResult::None;m_MarketOpen=false;m_MarketRow=0;m_TradeResult=TradeResult::None;m_Actor.nextActionId=nextAction;m_Actor.position=m_Player;m_Action={};
        m_FruitHarvested=0;
        m_PlayerLook=m_NpcLook=2;
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
        for(std::size_t i=0;i<GardenCells.size();++i)
            m_Targets.push_back({GardenCellId(i),TargetKind::Crop,GardenCells[i]});
        m_Targets.push_back({"item.demo.01",TargetKind::Pickup,{3070,2020}});
        IndexTargetTiles();
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
        SynchronizePresentation();
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
                m_Actor.inventory.ResourceCount(),m_FruitHarvested,m_Action.id,m_Action.phase,m_LastAction,m_Effects.Active().size(),m_Actor.home.items.TotalCount(),m_Hotbar.Alpha(),m_StorageOpen,m_Actor.money,m_MarketOpen,m_GardenControl};
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
    PresentationSnapshot DemoScene2D::CurrentPresentation() const
    {return {m_Player,m_Npc,m_Dog,m_Camera.Position()};}
    PresentationSnapshot DemoScene2D::Presentation(float alpha) const
    {
        const float t=std::isfinite(alpha)?std::clamp(alpha,0.0f,1.0f):1.0f;
        return {glm::mix(m_Previous.player,m_Player,t),glm::mix(m_Previous.npc,m_Npc,t),
                glm::mix(m_Previous.pet,m_Dog,t),glm::mix(m_Previous.camera,m_Camera.Position(),t)};
    }
    void DemoScene2D::SetViewport(int width,int height)
    {
        if(width<=0 || height<=0) return;
        m_Camera.Resize(width,height);if(!m_GardenControl) m_Camera.SetPosition(m_Player);
        m_Previous.camera=m_Camera.Position();
    }
    void DemoScene2D::SynchronizePresentation()
    {m_Previous=CurrentPresentation();m_Input.Clear();}
    void DemoScene2D::FixedUpdate(float deltaTime)
    {const auto input=m_Input.Consume();UpdateSimulation(deltaTime,input);}
    void DemoScene2D::CaptureInput()
    {
        SceneInput input;
        input.movement={
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_D)?1.0f:0.0f)-(MyEngine::Input::IsKeyPressed(GLFW_KEY_A)?1.0f:0.0f),
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_S)?1.0f:0.0f)-(MyEngine::Input::IsKeyPressed(GLFW_KEY_W)?1.0f:0.0f)};
        input.toggleGardenControl=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_B);
        input.running=MyEngine::Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT);
        input.interact=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_F);
        input.cancel=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_C);
        input.toggleStorage=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_K);
        input.toggleMarket=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_M);
        input.save=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_J);
        input.load=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_L);
        input.toggleTileGrid=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_G);
        input.nextArtSeason=MyEngine::Input::ConsumeKeyPress(GLFW_KEY_T);
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
        const bool click=MyEngine::Input::ConsumeMousePress(GLFW_MOUSE_BUTTON_LEFT);
        if(auto* window=glfwGetCurrentContext();window && m_HavePickCamera && !input.paused)
        {
            double x=0,y=0;int w=0,h=0,fw=0,fh=0;
            glfwGetCursorPos(window,&x,&y);glfwGetWindowSize(window,&w,&h);glfwGetFramebufferSize(window,&fw,&fh);
            if(w>0 && h>0 && x>=0 && y>=0 && x<w && y<h)
            {
                const glm::vec2 screen{static_cast<float>(x)*fw/w,static_cast<float>(y)*fh/h};
                const auto bar=HotbarLayout::ForViewport(fw,fh);
                const bool overBar=screen.x>=bar.x && screen.x<bar.x+bar.width &&
                                   screen.y>=bar.y && screen.y<bar.y+bar.height;
                if(click && screen.x>=8 && screen.x<268 && screen.y>=8 && screen.y<28 &&
                   !m_StorageOpen && !m_MarketOpen) input.toggleGardenControl=true;
                else if(overBar && click && !m_StorageOpen && !m_MarketOpen)
                {
                    for(int i=0;i<4;++i)
                    {
                        const float left=bar.x+bar.padding+i*(bar.slot+bar.gap);
                        if(screen.x>=left && screen.x<left+bar.slot && screen.y>=bar.y+bar.padding &&
                           screen.y<bar.y+bar.padding+bar.slot) input.selectedItem=i;
                    }
                }
                else if(!overBar && screen.y>64 && !m_StorageOpen && !m_MarketOpen)
                {
                    input.hoverWorld=m_PickCamera.ScreenToWorld(screen);
                    if(click) input.pointerWorld=input.hoverWorld;
                }
            }
        }
        m_Input.Capture(input);
    }
    FarmSave DemoScene2D::CaptureSave() const
    {
        return {m_Player,m_Actor.money,m_SelectedItem,m_Actor.hotbar[2],m_Actor.inventory,m_Actor.home,m_Targets};
    }

    bool DemoScene2D::RestoreSave(const FarmSave& save)
    {
        auto restored=m_Targets;
        const auto world=WorldMap::Size();
        if(!m_Ready || save.money<0 || !CropFromSeed(save.seed) ||
           save.selectedSlot<0 || save.selectedSlot>=4 ||
           !std::isfinite(save.position.x) || !std::isfinite(save.position.y) ||
           (restored.size()!=save.targets.size() && restored.size()!=save.targets.size()+GardenCells.size()) ||
           save.position.x<0 || save.position.y<0 || save.position.x>=world.x || save.position.y>=world.y ||
           save.home.origin.x<0 || save.home.origin.y<0 || save.home.origin.x+save.home.size.x>world.x ||
           save.home.origin.y+save.home.size.y>world.y) return false;
        const bool legacy=restored.size()!=save.targets.size();
        if(legacy && std::any_of(save.targets.begin(),save.targets.end(),[](const auto& target)
           {return target.id.starts_with("crop.home.");})) return false;
        for(auto& target:restored)
        {
            if(legacy && target.id.starts_with("crop.home."))
            {
                target={target.id,TargetKind::Crop,target.position};
                continue;
            }
            const auto it=std::find_if(save.targets.begin(),save.targets.end(),[&](const auto& value){return value.id==target.id;});
            if(it==save.targets.end() || it->kind!=target.kind || it->stock>target.maxStock ||
               (target.kind==TargetKind::Crop && !it->active) ||
               (target.kind==TargetKind::Object && it->active!=(it->health>0))) return false;
            target.active=it->active;
            target.health=it->health;
            target.tilled=it->tilled;
            target.planted=it->planted;
            target.ready=it->ready;
            target.crop=it->crop;
            target.age=it->age;
            target.moisture=it->moisture;
            target.revision=it->revision;
            target.stock=it->stock;
        }
        auto restoredWorld=m_World;
        for(const auto& target:restored)
            if(target.kind==TargetKind::Object) restoredWorld.SetObjectActive(target.id,target.active);
        if(!restoredWorld.CanOccupy(save.position,PlayerCollider)) return false;
        Reset();
        m_World=std::move(restoredWorld);
        m_Targets=std::move(restored);
        IndexTargetTiles();
        m_Player=m_Actor.position=save.position;
        m_Dog=m_Player+glm::vec2{-70,30};
        m_Actor.money=save.money;
        m_Actor.inventory=save.bag;
        m_Actor.home=save.home;
        m_Actor.hotbar[2]=save.seed;
        m_SelectedItem=save.selectedSlot;
        m_Camera.SetPosition(m_Player);
        SynchronizePresentation();
        return true;
    }

    void DemoScene2D::IndexTargetTiles()
    {
        m_TargetTiles.clear();
        for(std::size_t i=0;i<m_Targets.size();++i)
            if(const auto tile=TileAt(m_Targets[i].position))
                m_TargetTiles[TileKey(*tile)].push_back(i);
    }

    std::optional<DemoScene2D::TileFocus> DemoScene2D::PointerTile(glm::vec2 world)
    {
        const auto cell=TileAt(world);
        if(!cell) return std::nullopt;
        const auto bucket=m_TargetTiles.find(TileKey(*cell));
        if(bucket!=m_TargetTiles.end())
            for(const auto index:bucket->second)
                if(m_Targets[index].active && m_Targets[index].kind==TargetKind::Crop)
                    return TileFocus{*cell,&m_Targets[index]};
        return TileFocus{*cell,nullptr};
    }
    std::optional<DemoScene2D::TileFocus> DemoScene2D::FocusedTile()
    {
        const auto start=TileAt(m_Actor.position);
        if(!start) return std::nullopt;
        const glm::ivec2 step{m_Actor.facing};
        const auto front=*start+step;
        std::optional<TileFocus> empty;
        if(m_World.TryCell(front.x,front.y)) empty=TileFocus{front};
        constexpr int reach=static_cast<int>(InteractionSystem::Range)/WorldMap::TileSize+1;
        for(int offset=1;offset<=reach;++offset)
        {
            const auto cell=*start+step*offset;
            const auto* terrain=m_World.TryCell(cell.x,cell.y);
            if(!terrain || terrain->blocked) break;
            const auto bucket=m_TargetTiles.find(TileKey(cell));
            if(bucket==m_TargetTiles.end()) continue;
            InteractionTarget* best=nullptr;
            float limit=InteractionSystem::Range;
            for(const auto index:bucket->second)
            {
                auto& target=m_Targets[index];
                if(!target.active) continue;
                const auto delta=target.position-m_Actor.position;
                const float distance=glm::length(delta);
                if(distance>limit || (distance>1 && glm::dot(delta,m_Actor.facing)<0.5f*distance) ||
                   !m_World.HasLineOfSight(m_Actor.position,target.position)) continue;
                limit=distance;
                best=&target;
            }
            if(best) return TileFocus{cell,best};
        }
        return empty;
    }

    void DemoScene2D::UpdateSimulation(float deltaTime,const SceneInput& input)
    {
        if(!m_Ready) return;
        const float dt=std::isfinite(deltaTime)?std::clamp(deltaTime,0.0f,0.1f):0.0f;
        if(input.reset && !m_Loading){Reset();return;}
        m_Actor.uiCapturesInput=input.paused;
        if(input.paused)
        {
            m_PlayerAnimation.Update(0,true);m_NpcAnimation.Update(0,true);m_DogAnimation.Update(0,true);
            m_Previous=CurrentPresentation();
            return;
        }
        m_Previous=CurrentPresentation();
        m_SaveMessageTime=std::max(0.0f,m_SaveMessageTime-dt);
        if(m_SaveJob.valid() && m_SaveJob.wait_for(std::chrono::seconds(0))==std::future_status::ready)
        {
            try
            {
                auto result=m_SaveJob.get();
                m_SaveMessage=result.message;
                if(result.state && !RestoreSave(*result.state)) m_SaveMessage="SAVE INVALID";
            }
            catch(const std::exception&) {m_SaveMessage="SQL ERROR";}
            m_Loading=false;
            m_SaveMessageTime=4;
            return;
        }
        if(m_Loading) return;
        m_HoverWorld=input.hoverWorld;
        if(input.toggleGardenControl && !m_Action.Pending() && !m_MarketOpen && !m_StorageOpen)
        {
            m_GardenControl=!m_GardenControl;
            m_HoverWorld.reset();m_Previous=CurrentPresentation();
        }
        if(input.toggleTileGrid) m_ShowTileGrid=!m_ShowTileGrid;
        if(input.nextArtSeason && m_WorldArt.Ready()) m_ArtSeason=(m_ArtSeason+1)%4;
        if((input.save || input.load) && !m_SaveJob.valid() && !m_Action.Pending() && !m_MarketOpen)
        {
            try
            {
                if(input.load)
                    m_SaveJob=std::async(std::launch::async,[]{return SaveManager::Read();});
                else
                    m_SaveJob=std::async(std::launch::async,[state=CaptureSave()]{return SaveManager::Write(state);});
                m_Loading=input.load;
                m_SaveMessage=input.load?"DANG TAI":"DANG LUU";
            }
            catch(const std::exception&) {m_SaveMessage="SQL ERROR";}
            m_SaveMessageTime=4;
            return;
        }
        if(input.toggleMarket && !m_Action.Pending())
        {
            m_MarketOpen=!m_MarketOpen;
            m_StorageOpen=false;
            m_TradeResult=TradeResult::None;
            m_Hotbar.Update(0,true);
            return;
        }
        if(m_MarketOpen)
        {
            if(input.selectedItem>=0 && input.selectedItem<static_cast<int>(Market.size()))
                m_MarketRow=static_cast<std::size_t>(input.selectedItem);
            const auto& row=Market[m_MarketRow];
            if(input.interact || input.cancel || input.toggleStorage)
            {
                const auto kind=input.interact?TradeKind::Buy:input.cancel?TradeKind::SellBag:TradeKind::SellHome;
                m_TradeResult=Trade(m_Actor,kind,input.interact?row.seed:row.produce,1,m_Actor.lastTradeId+1);
            }
            if(row.seed!=ItemId::None && m_Actor.inventory.Count(row.seed)>0 &&
               (input.action || (input.interact && m_TradeResult==TradeResult::Success)))
            {
                m_Actor.hotbar[2]=row.seed;
                m_SelectedItem=2;
                m_Hotbar.Update(0,true);
            }
            return;
        }
        m_Hotbar.Update(dt,input.toggleStorage || input.selectedItem>=0 || input.interact || input.cancel || input.pointerWorld.has_value() || m_GardenControl || glm::length(input.movement)>0);
        const bool developer=m_Options.demoMode || m_Options.debugOverlay;
        const bool teleporting=developer && input.teleport>=4 && input.teleport<=8;
        const bool hurtTriggered=developer && input.hurt;
        if((input.cancel && !m_StorageOpen) || hurtTriggered || (developer && input.switchCharacter) || teleporting) CancelInteraction();
        InteractionSystem::Tick(m_Actor,m_Targets,dt);
        InteractionSystem::TickAction(m_Actor,m_Action,dt);
        if(input.zoom) m_Camera.SetZoom(m_Camera.Zoom()*std::pow(1.15f,static_cast<float>(std::clamp(input.zoom,-8,8))));
        if(developer && input.switchCharacter) m_Character=1-m_Character;
        if(!m_StorageOpen && !input.toggleStorage && input.selectedItem>=0 && input.selectedItem<4)
            m_SelectedItem=input.selectedItem;
        const bool locked=(m_PlayerState==VisualState::Action || m_PlayerState==VisualState::Hurt) &&
                          !m_PlayerAnimation.Finished();
        if(!locked && !m_Action.Pending())
        {
            if(!m_GardenControl) m_Facing=FacingFromVelocity(input.movement.x,input.movement.y,m_Facing);
            if(!m_GardenControl) m_PlayerLook=ArtDirection(input.movement,m_PlayerLook);
        }
        std::array<DynamicCollider,2> obstacles={{{m_Dog,DogCollider},{m_Npc,NpcCollider}}};
        const std::span<const DynamicCollider> playerObstacles(obstacles.data(),m_Options.demoMode?2u:1u);
        const auto previous=m_Player;
        m_Player=CollisionSystem::MoveAndSlide(m_World,m_Player,(m_GardenControl || m_Action.Pending())?glm::vec2{0,0}:input.movement,input.running?300.0f:180.0f,
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
        const bool atHome=m_Actor.home.Receives(m_Actor.ownerId,m_Player,m_Player);
        const bool storageInput=m_StorageOpen || input.toggleStorage;
        if(!atHome) m_StorageOpen=false;
        else if(input.toggleStorage)
        {
            m_StorageOpen=!m_StorageOpen;
            m_StorageResult=TradeResult::None;
        }
        else if(m_StorageOpen && !teleporting)
        {
            if(input.selectedItem>=0 && input.selectedItem<static_cast<int>(Market.size()))
                m_StorageRow=static_cast<std::size_t>(input.selectedItem);
            if(input.interact!=input.cancel)
                m_StorageResult=TransferProduce(m_Actor,Market[m_StorageRow].produce,input.interact);
        }
        switch(m_Facing)
        {
        case Facing::Left:m_Actor.facing={-1,0};break;
        case Facing::Right:m_Actor.facing={1,0};break;
        case Facing::Up:m_Actor.facing={0,-1};break;
        default:m_Actor.facing={0,1};break;
        }
        if(m_GardenControl && input.pointerWorld && !storageInput && !hurtTriggered && !input.cancel)
        {
            const auto focus=PointerTile(*input.pointerWorld);
            m_LastAction=InteractionSystem::UseGarden(m_Actor,focus?focus->target:nullptr,m_SelectedItem,m_Events);
            m_FeedbackTime=0.6f;
        }
        bool actionTriggered=developer && input.action && !m_Action.Pending();
        bool started=false;
        if(!m_GardenControl && input.interact && !storageInput && !hurtTriggered && !input.cancel && !teleporting)
        {
            const auto focus=FocusedTile();
            auto* target=focus?focus->target:nullptr;
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
        m_Effects.Consume(m_Events.Events());
        m_Events.Clear();
        m_Effects.Update(dt);
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
        if(npcMoving)
        {
            m_NpcFacing=FacingFromVelocity(m_Npc.x-npcBefore.x,m_Npc.y-npcBefore.y,m_NpcFacing);
            m_NpcLook=ArtDirection(m_Npc-npcBefore,m_NpcLook);
        }
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
        m_GameAge+=dt;
        if(m_GardenControl)
        {
            const float length=glm::length(input.movement);
            if(length>0.001f && std::isfinite(length))
                m_Camera.SetPosition(m_Camera.Position()+input.movement/length*(input.running?650.0f:400.0f)*dt);
        }
        else m_Camera.Follow(m_Player,dt);
        if(teleporting) m_Previous=CurrentPresentation();
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
        if(m_WorldArt.Ready())
        {
            const bool npc=&animation==&m_NpcAnimation;
            const bool walking=animation.State()==VisualState::Walk || animation.State()==VisualState::Run;
            if(const auto* frame=m_WorldArt.Actor(npc?m_NpcLook:m_PlayerLook,walking,animation.Frame()))
            {
                SpriteDraw sprite;
                sprite.texture=&m_WorldArt.Sprites().Texture();
                sprite.uv=frame->uv;
                sprite.placement=PlaceSprite(frame->bounds,feet,m_Scale.human,false,frame->pivot);
                return sprite;
            }
        }
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
    void DemoScene2D::DrawDog(MyEngine::Canvas2D& canvas,glm::vec2 feet) const
    {
        const auto sprite=SpriteFor("dog_walk",m_DogAnimation.Frame(),feet,PetHeight);
        const auto& p=sprite.placement;
        if(sprite.texture) canvas.Sprite(*sprite.texture,p.origin.x,p.origin.y,p.size.x,p.size.y,sprite.uv);
        else canvas.Rectangle(p.origin.x,p.origin.y,p.size.x,p.size.y,{0.55f,0.32f,0.16f,1});
    }
    void DemoScene2D::CacheTerrainArt()
    {
        constexpr std::array<glm::ivec2,8> offsets{{{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}}};
        for(int season=0;season<4;++season)
        {
            auto& frames=m_TerrainArt[static_cast<std::size_t>(season)];
            frames.assign(WorldMap::Width*WorldMap::Height,nullptr);
            const auto& atlas=m_WorldArt.Terrain(season);
            for(int y=0;y<WorldMap::Height;++y)
                for(int x=0;x<WorldMap::Width;++x)
                {
                    const auto* cell=m_World.TryCell(x,y);
                    const bool water=cell->terrain==Terrain::Water;
                    unsigned int mask=0;
                    for(std::size_t bit=0;bit<offsets.size();++bit)
                    {
                        const auto offset=offsets[bit];
                        const auto* neighbor=m_World.TryCell(x+offset.x,y+offset.y);
                        if(!neighbor || (neighbor->terrain==Terrain::Water)==water) mask|=1u<<bit;
                    }
                    mask=CanonicalTerrainMask(mask);
                    std::string key;
                    if(!water && mask==255) key=cell->terrain==Terrain::Path?"path":"grass";
                    else key=std::string(water?"water_":"land_")+
                             (mask<10?"00":mask<100?"0":"")+std::to_string(mask);
                    frames[static_cast<std::size_t>(y*WorldMap::Width+x)]=atlas.Find(key);
                }
        }
    }

    void DemoScene2D::DrawWorldObject(MyEngine::Canvas2D& canvas,const WorldObject& object) const
    {
        const auto& atlas=m_WorldArt.Sprites();
        std::string_view id="rock";
        if(object.visualId.find("tree")!=std::string::npos) id="tree";
        else if(object.visualId.find("bush")!=std::string::npos) id="bush";
        else if(object.visualId.find("gate")!=std::string::npos) id="fence";
        else if(object.visualId.find("chest")!=std::string::npos) id="chest";
        else if(object.visualId.find("sign")!=std::string::npos) id="sign";
        else if(object.visualId.find("water")!=std::string::npos) return;
        const auto& selectedAtlas=id=="tree" && m_WorldArt.CozyReady()?m_WorldArt.Props():atlas;
        const auto* frame=selectedAtlas.Find(id);
        if(!frame) return;
        const auto feet=object.position+glm::vec2{object.size.x*0.5f,object.size.y};
        const float height=id=="tree"?m_Scale.tree:id=="fence"?m_Scale.gate:std::clamp(object.size.y,32.0f,96.0f);
        const auto placement=PlaceSprite(frame->bounds,feet,height,false,frame->pivot);
        canvas.Sprite(selectedAtlas.Texture(),placement.origin.x,placement.origin.y,placement.size.x,placement.size.y,frame->uv);
    }

    void DemoScene2D::DrawStaticObject(MyEngine::Canvas2D& canvas,const WorldObject& object) const
    {
        const auto& atlas=m_WorldArt.Sprites();
        if(object.gameplayId.find("fence")!=std::string::npos)
        {
            const bool horizontal=object.size.x>object.size.y;
            const float length=horizontal?object.size.x:object.size.y;
            for(float distance=0;distance<length;distance+=m_Scale.fence)
            {
                const auto origin=object.position+(horizontal?glm::vec2{distance,-32}:glm::vec2{-12,distance});
                atlas.Draw(canvas,"fence",origin,{m_Scale.fence,m_Scale.fence});
            }
            return;
        }
        const bool shop=object.gameplayId=="town.market";
        // The fountain collider belongs to the old baked town map; do not draw a house over it.
        if(object.gameplayId=="town.fountain") return;
        const auto& selectedAtlas=!shop && m_WorldArt.CozyReady()?m_WorldArt.Props():atlas;
        const auto* frame=selectedAtlas.Find(shop?"shop":"house");
        if(!frame) return;
        const auto feet=object.position+glm::vec2{object.size.x*0.5f,object.size.y};
        const auto placement=PlaceSprite(frame->bounds,feet,m_Scale.house,false,frame->pivot);
        canvas.Sprite(selectedAtlas.Texture(),placement.origin.x,placement.origin.y,placement.size.x,placement.size.y,frame->uv);
    }

    void DemoScene2D::DrawCrop(MyEngine::Canvas2D& canvas,const InteractionTarget& plot) const
    {
        const auto* it=&plot;
        const float scale=it->id.starts_with("crop.home.")?0.55f:1.0f;
        const auto* crop=FindCrop(it->crop);
        const double visualAge=crop?it->age*5.0/crop->growthSeconds:0;
        static const std::vector<double> stageDurations{1,1,1,2};
        const int stage=GrowthFrame(visualAge,stageDurations);
        if(it->crop==CropId::Carrot)
        {
            const auto& texture=m_LocalCarrot[static_cast<std::size_t>(std::clamp(stage-1,0,2))];
            if(texture.Id())
            {
                // The three reviewed 192 px frames share an alpha bottom at pixel 180.
                canvas.Sprite(texture,it->position.x-56*scale,it->position.y-105*scale,112*scale,112*scale);
                if(it->ready && m_LocalCarrotIcon.Id())
                    canvas.Sprite(m_LocalCarrotIcon,it->position.x+26*scale,it->position.y-91*scale,28*scale,28*scale);
                return;
            }
        }
        if(m_CropTexture && m_Assets)
            if(const auto* record=m_Assets->Record("turnip_growth"))
                if(const auto uv=AssetManifest::FrameUv(*record,stage))
                {
                    canvas.Sprite(*m_CropTexture,it->position.x-56*scale,it->position.y-94*scale,112*scale,112*scale,*uv);
                    return;
                }
        const float size=(14.0f+static_cast<float>(stage)*7.0f)*scale;
        const std::array<std::array<float,4>,4> colors={{{0.48f,0.72f,0.22f,1},{0.31f,0.68f,0.18f,1},
            {0.22f,0.58f,0.15f,1},{0.65f,0.82f,0.25f,1}}};
        canvas.Rectangle(it->position.x-size*0.5f,it->position.y-size,size,size,colors[static_cast<std::size_t>(stage)]);
    }
    void DemoScene2D::Render(MyEngine::Canvas2D& canvas,int width,int height,float interpolationAlpha)
    {
        if (!m_Ready) return;
        const auto presented=Presentation(interpolationAlpha);
        auto renderCamera=m_Camera;
        renderCamera.Resize(width,height);
        renderCamera.SetPosition(presented.camera);
        const float shake=m_Effects.Shake();
        renderCamera.SetPosition(renderCamera.Position()+glm::vec2{
            std::sin(static_cast<float>(m_GameAge)*87.0f)*shake,
            std::cos(static_cast<float>(m_GameAge)*71.0f)*shake});
        m_PickCamera=renderCamera;m_HavePickCamera=true;
        canvas.BeginWorld(width,height,renderCamera);
        if(m_WorldArt.Ready())
        {
            const auto range=m_World.VisibleTiles(renderCamera.Position(),renderCamera.ViewSize());
            const auto& atlas=m_WorldArt.Terrain(m_ArtSeason);
            const auto& frames=m_TerrainArt[static_cast<std::size_t>(m_ArtSeason)];
            const bool cozy=m_WorldArt.CozyReady() && m_ArtSeason==0;
            const auto& ground=m_WorldArt.Ground();
            const auto* waterFrame=atlas.Find("water_255");
            const auto* cozyGrass=cozy?ground.Find("grass"):nullptr;
            const auto* pathFrame=atlas.Find("path");
            for(int y=range.top;y<range.bottom;++y)
                for(int x=range.left;x<range.right;++x)
                    if(const auto* frame=frames[static_cast<std::size_t>(y*WorldMap::Width+x)])
                    {
                        const auto* base=cozy && frame!=pathFrame?cozyGrass:nullptr;
                        const float wx=static_cast<float>(x*WorldMap::TileSize),wy=static_cast<float>(y*WorldMap::TileSize);
                        if(base)
                        {
                            // One image spans eight cells. Geographic tint varies slowly, not as checkerboard noise.
                            auto uv=base->uv;
                            uv[0]+=uv[2]*static_cast<float>(x%8)/8;uv[1]+=uv[3]*static_cast<float>(y%8)/8;
                            uv[2]/=8;uv[3]/=8;
                            const float shade=0.95f+0.04f*std::sin(x*0.12f+y*0.08f);
                            const float forest=std::clamp((wx-3900.0f)/1200.0f,0.0f,1.0f);
                            canvas.Sprite(ground.Texture(),wx,wy,WorldMap::TileSize,WorldMap::TileSize,uv,
                                {shade*(1-forest*0.08f),shade,shade*(1-forest*0.03f),1});
                        }
                        else canvas.Sprite(atlas.Texture(),wx,wy,WorldMap::TileSize,WorldMap::TileSize,frame->uv);
                    }
            if(cozy)
            {
                // The source world defines one elliptical lake. Use the same center/radii;
                // keep its tile collision model, with a soft visual shore instead of unrelated v4 art.
                constexpr glm::vec2 lakeCenter{(WorldMap::LakeCenterX+0.5f)*WorldMap::TileSize,
                                                (WorldMap::LakeCenterY+0.5f)*WorldMap::TileSize};
                constexpr glm::vec2 lakeRadius{WorldMap::LakeRadiusX*WorldMap::TileSize,
                                                WorldMap::LakeRadiusY*WorldMap::TileSize};
                const auto shore=lakeCenter-lakeRadius-glm::vec2{12,12};
                if(WorldMap::Intersects(shore,lakeRadius*2.0f+glm::vec2{24,24},
                                       renderCamera.Position()-renderCamera.ViewSize()*0.5f,renderCamera.ViewSize()))
                {
                    canvas.Ellipse(shore.x,shore.y,lakeRadius.x*2+24,lakeRadius.y*2+24,{0.57f,0.64f,0.40f,1});
                    const auto* water=ground.Find("water");
                    canvas.SpriteEllipse(ground.Texture(),lakeCenter.x-lakeRadius.x,lakeCenter.y-lakeRadius.y,
                                         lakeRadius.x*2,lakeRadius.y*2,water->uv);
                }
                // Ripple highlights are one adjacent untextured batch, using simulation time only.
                for(int y=range.top;y<range.bottom;++y) for(int x=range.left;x<range.right;++x)
                {
                    if(frames[static_cast<std::size_t>(y*WorldMap::Width+x)]!=waterFrame || (x*7+y*13)%11) continue;
                    const float phase=static_cast<float>(m_GameAge)*1.2f+x*0.4f+y*0.7f;
                    const float rippleWidth=18+5*std::sin(phase);
                    canvas.Rectangle(x*64.0f+20,y*64.0f+30+2*std::sin(phase),rippleWidth,1.5f,
                                     {0.92f,0.98f,1,0.13f+0.06f*std::sin(phase)});
                }
                // Stable sparse clumps make each region distinct; prepared plots remain clear.
                const auto* meadow=ground.Find("meadow");
                for(int y=range.top;y<range.bottom;++y) for(int x=range.left;x<range.right;++x)
                {
                    const auto* cell=m_World.TryCell(x,y);
                    const auto hash=static_cast<unsigned int>(x)*73856093u ^ static_cast<unsigned int>(y)*19349663u;
                    const unsigned int density=cell->terrain==Terrain::Forest?7u:cell->zone==Zone::Farm?19u:13u;
                    if(cell->terrain==Terrain::Water || cell->terrain==Terrain::Path || hash%density ||
                       m_TargetTiles.contains(TileKey({x,y}))) continue;
                    const float clumpHeight=18.0f+static_cast<float>((hash>>8)%15);
                    const glm::vec2 feet{x*64.0f+16.0f+static_cast<float>((hash>>12)%32),y*64.0f+48};
                    const auto place=PlaceSprite(meadow->bounds,feet,clumpHeight,false,meadow->pivot);
                    canvas.Sprite(ground.Texture(),place.origin.x,place.origin.y,place.size.x,place.size.y,meadow->uv);
                }
            }
        }
        else if(m_Map)
            canvas.Sprite(*m_Map,0,0,WorldSize.x,WorldSize.y);
        else
        {
            const auto range=m_World.VisibleTiles(renderCamera.Position(),renderCamera.ViewSize());
            constexpr std::array<std::array<float,4>,5> colors={{{0.43f,0.65f,0.32f,1},
                {0.78f,0.68f,0.49f,1},{0.55f,0.38f,0.24f,1},{0.21f,0.51f,0.7f,1},{0.23f,0.48f,0.26f,1}}};
            for(int y=range.top;y<range.bottom;++y)
                for(int x=range.left;x<range.right;++x)
                    if(const auto* cell=m_World.TryCell(x,y))
                        canvas.Rectangle(static_cast<float>(x*WorldMap::TileSize),
                            static_cast<float>(y*WorldMap::TileSize),WorldMap::TileSize+0.5f,
                            WorldMap::TileSize+0.5f,colors[static_cast<std::size_t>(cell->terrain)]);
        }
        const auto visibleOrigin=renderCamera.Position()-renderCamera.ViewSize()*0.5f;
        m_RenderOrder.Begin(visibleOrigin,renderCamera.ViewSize());
        const auto focus=m_GardenControl?(m_HoverWorld?PointerTile(*m_HoverWorld):std::nullopt):FocusedTile();
        const auto* focusedTarget=focus?focus->target:nullptr;
        constexpr float tileSize=static_cast<float>(WorldMap::TileSize);
        if(m_ShowTileGrid)
        {
            const auto range=m_World.VisibleTiles(renderCamera.Position(),renderCamera.ViewSize());
            constexpr std::array<float,4> grid{0.98f,0.93f,0.71f,0.48f};
            const float left=static_cast<float>(range.left)*tileSize;
            const float top=static_cast<float>(range.top)*tileSize;
            const float widthWorld=static_cast<float>(range.right-range.left)*tileSize;
            const float heightWorld=static_cast<float>(range.bottom-range.top)*tileSize;
            for(int x=range.left;x<=range.right;++x)
                canvas.Rectangle(static_cast<float>(x)*tileSize,top,1,heightWorld,grid);
            for(int y=range.top;y<=range.bottom;++y)
                canvas.Rectangle(left,static_cast<float>(y)*tileSize,widthWorld,1,grid);
        }
        for(std::size_t i=0;i<m_Targets.size();++i)
        {
            const auto& target=m_Targets[i];
            if(!target.active) continue;
            if(target.kind==TargetKind::Crop)
            {
                const auto feet=target.position;
                if(!WorldMap::Intersects(feet-glm::vec2{60,105},{120,140},visibleOrigin,renderCamera.ViewSize())) continue;
                const auto tile=TileAt(feet);
                if(!tile) continue;
                const auto origin=glm::vec2(*tile)*tileSize;
                const auto soil=target.moisture>0?std::array<float,4>{0.30f,0.26f,0.19f,0.95f}:
                    std::array<float,4>{0.49f,0.33f,0.21f,0.95f};
                canvas.Rectangle(origin.x+1,origin.y+1,tileSize-2,tileSize-2,{0.91f,0.78f,0.49f,1});
                canvas.Rectangle(origin.x+3,origin.y+3,tileSize-6,tileSize-6,soil);
                const auto& texture=m_LocalSoil[target.moisture>0?1u:0u];
                if(m_WorldArt.Ready())
                {
                    const auto hasCrop=[&](glm::ivec2 cell)
                    {
                        const auto bucket=m_TargetTiles.find(TileKey(cell));
                        if(bucket==m_TargetTiles.end()) return false;
                        return std::any_of(bucket->second.begin(),bucket->second.end(),[&](std::size_t index){
                            const auto& other=m_Targets[index];
                            return other.active && other.kind==TargetKind::Crop;
                        });
                    };
                    const bool left=hasCrop(*tile-glm::ivec2{1,0});
                    const bool right=hasCrop(*tile+glm::ivec2{1,0});
                    const std::string part=left?(right?"giua":"cuoi"):(right?"dau":"don");
                    m_WorldArt.Terrain(m_ArtSeason).Draw(canvas,std::string(target.moisture>0?"bed_uot_":"bed_kho_")+part,
                                                       origin,{tileSize,tileSize});
                }
                else if(texture.Id())
                    canvas.Sprite(texture,origin.x+3,origin.y+3,tileSize-6,tileSize-6);
                const char* label=!target.planted?"O TRONG":target.ready?"DA CHIN":target.moisture>0?"DANG LON":"CAN TUOI";
                if(&target==focusedTarget || m_Options.debugOverlay)
                    HudText(canvas,label,origin.x+5,origin.y+tileSize-13,1.05f,{1,0.94f,0.78f,1});
                if(target.planted)
                    m_RenderOrder.Submit(RenderKind::Crop,i,feet-glm::vec2{56,105},{112,123},feet.y);
            }
            else if(target.kind==TargetKind::Pickup)
            {
                m_RenderOrder.Submit(RenderKind::Pickup,i,target.position-glm::vec2{42,44},{84,76},target.position.y);
            }
        }
        if(m_Map && !m_WorldArt.Ready())
            for(std::size_t i=0;i<m_Trees.Entries().size();++i)
            {
                const auto& tree=m_Trees.Entries()[i];
                m_RenderOrder.Submit(RenderKind::TreeCanopy,i,tree.canopyOrigin,tree.canopySize,tree.footY);
            }
        else
            for(std::size_t i=0;i<m_World.Objects().size();++i)
            {
                const auto& object=m_World.Objects()[i];
                if(object.active) m_RenderOrder.Submit(RenderKind::WorldObject,i,object.position-glm::vec2{m_Scale.tree,m_Scale.tree},object.size+glm::vec2{2*m_Scale.tree,m_Scale.tree},
                                                       object.position.y+object.size.y);
            }
        if(focus && !m_MarketOpen && !m_StorageOpen)
        {
            const auto origin=glm::vec2(focus->cell)*tileSize;
            const auto* cell=m_World.TryCell(focus->cell.x,focus->cell.y);
            const auto color=focusedTarget?std::array<float,4>{1,0.93f,0.53f,1}:
                cell && cell->blocked?std::array<float,4>{0.95f,0.35f,0.28f,0.9f}:
                std::array<float,4>{0.95f,0.93f,0.84f,0.65f};
            canvas.Rectangle(origin.x,origin.y,tileSize,3,color);
            canvas.Rectangle(origin.x,origin.y+tileSize-3,tileSize,3,color);
            canvas.Rectangle(origin.x,origin.y,3,tileSize,color);
            canvas.Rectangle(origin.x+tileSize-3,origin.y,3,tileSize,color);
        }
        if(m_WorldArt.Ready())
            for(std::size_t i=0;i<m_World.StaticColliders().size();++i)
            {
                const auto& object=m_World.StaticColliders()[i];
                m_RenderOrder.Submit(RenderKind::StaticObject,i,object.position-glm::vec2{m_Scale.house,m_Scale.house},
                                     object.size+glm::vec2{2*m_Scale.house,m_Scale.house},object.position.y+object.size.y);
            }
        const auto player=ActorSprite(m_Character,presented.player,m_PlayerAnimation).placement;
        if(!m_GardenControl) m_RenderOrder.Submit(RenderKind::Actor,0,player.visibleOrigin,player.visibleSize,presented.player.y);
        if(m_Options.demoMode)
        {
            const auto npc=ActorSprite(1,presented.npc,m_NpcAnimation).placement;
            m_RenderOrder.Submit(RenderKind::Actor,1,npc.visibleOrigin,npc.visibleSize,presented.npc.y);
        }
        const auto pet=SpriteFor("dog_walk",m_DogAnimation.Frame(),presented.pet,PetHeight).placement;
        if(!m_GardenControl) m_RenderOrder.Submit(RenderKind::Animal,0,pet.visibleOrigin,pet.visibleSize,presented.pet.y);
        m_RenderOrder.Sort();
        for(const auto& entry:m_RenderOrder.Entries())
        {
            switch(entry.kind)
            {
            case RenderKind::Crop:DrawCrop(canvas,m_Targets[entry.index]);break;
            case RenderKind::Pickup:
            {
                const auto& pickup=m_Targets[entry.index];
                const auto feet=pickup.position;
                canvas.Rectangle(feet.x-42,feet.y-8,84,36,{0.23f,0.31f,0.38f,0.94f});
                if(m_Items) canvas.Sprite(*m_Items,feet.x-26,feet.y-40,52,52,m_ItemRegions[2]);
                else canvas.Rectangle(feet.x-12,feet.y-12,24,24,{0.94f,0.78f,0.3f,1});
                HudText(canvas,"VAT PHAM",feet.x-36,feet.y+13,1.5f,{1,1,1,1});
                break;
            }
            case RenderKind::Actor:
                if(entry.index==0) DrawActor(canvas,m_Character,presented.player,m_PlayerAnimation);
                else DrawActor(canvas,1,presented.npc,m_NpcAnimation);
                break;
            case RenderKind::Animal:DrawDog(canvas,presented.pet);break;
            case RenderKind::TreeCanopy:
            {
                const auto& tree=m_Trees.Entries()[entry.index];
                const auto x=tree.canopyOrigin.x,y=tree.canopyOrigin.y;
                const auto w=tree.canopySize.x,h=tree.canopySize.y;
                canvas.SpriteEllipse(*m_Map,x,y,w,h,{x/WorldSize.x,y/WorldSize.y,w/WorldSize.x,h/WorldSize.y});
                break;
            }
            case RenderKind::StaticObject:
                DrawStaticObject(canvas,m_World.StaticColliders()[entry.index]);
                break;
            case RenderKind::WorldObject:
            {
                if(m_WorldArt.Ready())
                {
                    DrawWorldObject(canvas,m_World.Objects()[entry.index]);
                    break;
                }
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
        if(!m_MarketOpen && !m_StorageOpen)
        {
            canvas.Rectangle(8,8,260,20,{0.96f,0.94f,0.82f,0.95f});
            HudText(canvas,m_GardenControl?"B: VUON - CHUOT | WASD CAMERA":"B: VUON | WASD DI - F DUNG",14,13,1.2f,{0.20f,0.30f,0.16f,1});
        }
        const float opacity=m_Hotbar.Alpha();
        if(opacity>0 && !m_MarketOpen)
        {
        const auto rectangle=[&](float x,float y,float w,float h,std::array<float,4> color)
        {color[3]*=opacity;canvas.Rectangle(x,y,w,h,color);};
        const auto bar=HotbarLayout::ForViewport(width,height);
        const float unit=bar.slot/64.0f;
        rectangle(bar.x,bar.y,bar.width,bar.height,{0.20f,0.24f,0.16f,0.94f});
        rectangle(bar.x+2*unit,bar.y+2*unit,bar.width-4*unit,bar.height-4*unit,{0.89f,0.85f,0.69f,0.98f});
        for(int i=0;i<4;++i)
        {
            const float x=bar.x+bar.padding+i*(bar.slot+bar.gap),y=bar.y+bar.padding;
            const bool selected=i==m_SelectedItem;
            rectangle(x,y,bar.slot,bar.slot,selected?
                std::array<float,4>{0.69f,0.47f,0.20f,1}:std::array<float,4>{0.57f,0.62f,0.43f,1});
            rectangle(x+2*unit,y+2*unit,bar.slot-4*unit,bar.slot-4*unit,
                             selected?std::array<float,4>{1,0.96f,0.79f,1}:std::array<float,4>{0.95f,0.93f,0.82f,1});
            const float inset=10*unit;
            if(i==2 && m_LocalSeedIcon.Id())
                canvas.Sprite(m_LocalSeedIcon,x+inset,y+inset,bar.slot-2*inset,bar.slot-2*inset,{0,0,1,1},{1,1,1,opacity});
            else if(m_Items && m_Assets)
            {
                if(const auto* frame=m_Assets->FrameBounds("items_farm",i))
                {
                    const auto icon=FitSprite(*frame,{x+inset,y+inset},{bar.slot-2*inset,bar.slot-2*inset});
                    canvas.Sprite(*m_Items,icon.origin.x,icon.origin.y,icon.size.x,icon.size.y,m_ItemRegions[static_cast<std::size_t>(i)],{1,1,1,opacity});
                }
            }
            else rectangle(x+inset,y+inset,bar.slot-2*inset,bar.slot-2*inset,{0.65f,0.35f,0.16f,1});
            // Numeric quantities from the same inventory model; no font/image dependency.
            const auto quantity=std::to_string(m_Actor.inventory.Count(m_Actor.SelectedItem(i)));
            constexpr std::array<unsigned int,10> digits{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
            constexpr std::array<std::array<float,4>,7> segments{{
                {0,0,3,1},{2,0,1,3},{2,2,1,3},{0,4,3,1},{0,2,1,3},{0,0,1,3},{0,2,3,1}}};
            float numberX=x+bar.slot-5*unit-static_cast<float>(quantity.size())*5*unit;
            for(const char digit:quantity)
            {
                const auto mask=digits[static_cast<std::size_t>(digit-'0')];
                for(std::size_t segment=0;segment<segments.size();++segment)
                    if(mask&(1u<<segment))
                    {
                        const auto& r=segments[segment];
                        rectangle(numberX+r[0]*unit,y+bar.slot-13*unit+r[1]*unit,r[2]*unit,r[3]*unit,
                                         {0.20f,0.22f,0.14f,1});
                    }
                numberX+=5*unit;
            }
            // Small index marks remain readable without introducing an unrelated font renderer.
            for(int mark=0;mark<=i;++mark)
                rectangle(x+5*unit+mark*4*unit,y+bar.slot-6*unit,2*unit,2*unit,{0.39f,0.37f,0.23f,1});
        }
        const float meterWidth=bar.width*0.4f,meterX=bar.x+(bar.width-meterWidth)*0.5f;
        rectangle(meterX,bar.y-9*unit,meterWidth,5*unit,{0.17f,0.22f,0.14f,1});
        rectangle(meterX,bar.y-9*unit,meterWidth*std::clamp(m_Actor.stamina/20.0f,0.0f,1.0f),5*unit,{0.42f,0.67f,0.36f,1});
        if(m_FeedbackTime>0)
        {
            const char* message=m_LastAction==InteractionResult::StorageFull?"KHO DAY":
                m_LastAction==InteractionResult::Success && m_Action.impactConsumed && m_Action.toStorage?"VAO KHO":
                m_LastAction==InteractionResult::Success && m_Action.impactConsumed && m_Action.operation==FarmOperation::Water?"DA TUOI":"";
            HudText(canvas,message,bar.x+8*unit,bar.y-25*unit,1.3f*unit,{0.18f,0.23f,0.12f,opacity});
        }
        if(m_FeedbackTime>0)
            rectangle(bar.x+bar.width-6*unit,bar.y-9*unit,6*unit,5*unit,
                m_LastAction==InteractionResult::Success?std::array<float,4>{0.25f,0.78f,0.35f,m_FeedbackTime}:
                                                         std::array<float,4>{0.85f,0.25f,0.19f,m_FeedbackTime});
        }
        if(m_MarketOpen)
        {
            const float scale=std::min(static_cast<float>(width)/640,static_cast<float>(height)/360);
            const float x=static_cast<float>(width)*0.5f-150*scale;
            const float y=static_cast<float>(height)*0.5f-91*scale;
            canvas.Rectangle(x,y,300*scale,182*scale,{0.94f,0.93f,0.83f,0.98f});
            const auto label=[&](std::string_view value,float left,float top)
            {HudText(canvas,value,x+left*scale,y+top*scale,1.5f*scale);};
            label("TIEN "+std::to_string(m_Actor.money)+"     M DONG",12,10);
            label("MUA / BAN",182,29);
            for(std::size_t i=0;i<Market.size();++i)
            {
                const auto& row=Market[i];
                const float top=45+static_cast<float>(i)*17;
                if(i==m_MarketRow) canvas.Rectangle(x+8*scale,y+(top-2)*scale,284*scale,15*scale,{0.75f,0.84f,0.60f,1});
                label(std::to_string(i+1)+" "+row.label,12,top);
                label((row.seed==ItemId::None?std::string("-"):std::to_string(row.buyPrice))+" / "+std::to_string(row.sellPrice),182,top);
            }
            label("F MUA   C BAN TUI   K BAN KHO",12,117);
            label("SPACE CHON HAT",12,134);
            constexpr std::array results{"","DA GIAO DICH","SAI MAT HANG","DA GIAO DICH",
                "DANG BAN","NGOAI VUON","THIEU TIEN","TUI DAY","THIEU DO","TIEN DAY"};
            label(results[static_cast<std::size_t>(m_TradeResult)],12,155);
            if(m_LocalCarrotIcon.Id())
                canvas.Sprite(m_LocalCarrotIcon,x+145*scale,y+60*scale,15*scale,15*scale);
        }
        if(m_StorageOpen)
        {
            const float scale=std::min(1.0f,std::min(static_cast<float>(width)/640.0f,static_cast<float>(height)/360.0f));
            const float x=width-230*scale,y=16*scale;
            canvas.Rectangle(x,y,214*scale,156*scale,{0.94f,0.93f,0.83f,0.97f});
            HudText(canvas,"KHO - K DONG",x+10*scale,y+10*scale,1.5f*scale);
            HudText(canvas,"TUI/KHO",x+141*scale,y+24*scale,1.25f*scale);
            for(std::size_t i=0;i<Market.size();++i)
            {
                const float rowY=y+(39+static_cast<float>(i)*16)*scale;
                const auto& row=Market[i];
                if(i==m_StorageRow)
                    canvas.Rectangle(x+7*scale,rowY-2*scale,200*scale,15*scale,{0.75f,0.84f,0.60f,1});
                HudText(canvas,std::to_string(i+1)+" "+row.label,x+10*scale,rowY,1.4f*scale);
                const auto counts=std::to_string(m_Actor.inventory.Count(row.produce))+"/"+std::to_string(m_Actor.home.items.Count(row.produce));
                HudText(canvas,counts,x+141*scale,rowY,1.4f*scale);
            }
            HudText(canvas,"F RUT - C CAT",x+10*scale,y+107*scale,1.4f*scale);
            const char* feedback=m_StorageResult==TradeResult::Success?"DA CHUYEN":
                m_StorageResult==TradeResult::Full?"HET CHO":
                m_StorageResult==TradeResult::NoItems?"THIEU DO":
                m_StorageResult==TradeResult::Busy?"DANG BAN":"";
            HudText(canvas,feedback,x+10*scale,y+130*scale,1.4f*scale);
        }
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
        if(focusedTarget && (focusedTarget->kind==TargetKind::Crop || focusedTarget->kind==TargetKind::Pickup) && !m_MarketOpen && !m_StorageOpen)
        {
            const float scale=std::min(1.5f,static_cast<float>(width)/340.0f);
            const char* hint=m_GardenControl?(!focusedTarget->planted?"CHON HAT - CLICK GIEO":focusedTarget->ready?"CLICK THU HOACH":focusedTarget->moisture>0?"DANG LON":"CHON BINH - CLICK TUOI"):focusedTarget->kind==TargetKind::Pickup?"F NHAT VAT PHAM":!focusedTarget->planted?"3 CHON HAT - F GIEO":
                focusedTarget->ready?"F THU HOACH":focusedTarget->moisture>0?"DANG LON":"1 CHON BINH - F TUOI";
            const float left=static_cast<float>(width)*0.5f-72*scale;
            canvas.Rectangle(left,34,144*scale,15*scale,{0.95f,0.90f,0.77f,0.98f});
            HudText(canvas,hint,left+5*scale,38,scale);
        }
        if(m_WorldArt.Ready() && !m_StorageOpen && !m_MarketOpen)
        {
            constexpr std::array seasons{"XUAN","HA","THU","DONG"};
            const float left=static_cast<float>(width)-140;
            canvas.Rectangle(left,8,132,20,{0.95f,0.93f,0.82f,0.94f});
            HudText(canvas,std::string(seasons[static_cast<std::size_t>(m_ArtSeason)])+" - T MUA",left+5,13,1.3f);
        }
        if(m_SaveMessageTime>0)
        {
            const float scale=std::min(1.5f,static_cast<float>(width)/300.0f);
            canvas.Rectangle(8,8,146*scale,15*scale,{0.94f,0.93f,0.83f,0.95f});
            HudText(canvas,m_SaveMessage,12,12,scale);
        }
        canvas.End();
    }
    void DemoScene2D::Shutdown()
    {
        if(m_SaveJob.valid()) m_SaveJob.wait();
        m_SaveJob={};
        m_Loading=false;
        m_SaveMessageTime=0;
        CancelInteraction();
        m_Ready = false;
        for(auto& texture:m_LocalCarrot) texture.Reset();
        for(auto& texture:m_LocalSoil) texture.Reset();
        m_LocalCarrotIcon.Reset();
        m_LocalSeedIcon.Reset();
        m_WorldArt.Reset();
        for(auto& frames:m_TerrainArt) frames.clear();
        m_Map=m_Items=m_DogTexture=m_CropTexture=nullptr;
        m_Assets=nullptr;
        m_Characters.fill(nullptr);
        m_Trees={};m_RenderOrder={};
        m_Visuals={};m_PlayerAnimation={};m_NpcAnimation={};m_DogAnimation={};m_CropAnimation={};
        m_Targets.clear();m_Events.Clear();m_Effects.Clear();
    }
}
