#pragma once
#include "Graphics/Canvas2D.h"
#include "Gameplay/Graphics/AssetTextures.h"
#include "Gameplay/Graphics/VisualCatalog.h"
#include "Gameplay/World/WorldMap.h"
#include "Gameplay/World/ForegroundTrees.h"
#include "Gameplay/Graphics/RenderOrder.h"
#include "Gameplay/Graphics/HotbarVisibility.h"
#include "Gameplay/Graphics/WorldArt.h"
#include "Gameplay/Systems/Trade.h"
#include "Gameplay/Systems/InteractionSystem.h"
#include "Gameplay/Effects/EffectPool.h"
#include <array>
#include <future>
#include <unordered_map>
#include "Gameplay/Save/Save.h"
#include "SceneOptions.h"
#include "Gameplay/Player/SceneInput.h"

namespace MyGame
{
    struct PresentationSnapshot
    {
        glm::vec2 player{0,0},npc{0,0},pet{0,0},camera{0,0};
    };
    struct SceneSnapshot
    {
        glm::vec2 player,npc,pet;
        VisualState playerState,npcState,petState;
        int playerFrame,npcFrame,petFrame;
        int inventoryCount=0,fruitHarvested=0;
        std::uint64_t actionId=0;
        ActionPhase actionPhase=ActionPhase::Idle;
        InteractionResult actionResult=InteractionResult::NoTarget;
        std::size_t effects=0;
        int storedItemCount=0;
        float hotbarAlpha=1;
        bool storageOpen=false;
        int money=0;
        bool marketOpen=false;
        bool gardenControl=false;
    };
    class DemoScene2D
    {
    public:
        explicit DemoScene2D(SceneOptions options={}) : m_Options(options) {}
        const SceneOptions& Options() const { return m_Options; }
        void SetOptions(SceneOptions options);
        // Same simulation path used by live input and deterministic integration tests.
        void UpdateSimulation(float deltaTime,const SceneInput& input);
        SceneSnapshot Snapshot() const;
        const Inventory& Bag() const {return m_Actor.inventory;}
        const HomeStorage& Storage() const {return m_Actor.home;}
        float HotbarAlpha() const {return m_Hotbar.Alpha();}
        ItemId SelectedItem() const {return m_Actor.SelectedItem(m_SelectedItem); }
        std::optional<InteractionTarget> TargetState(std::string_view id) const;
        bool Initialize(AssetTextures& assets,const std::filesystem::path& dataRoot);
        void CaptureInput();
        void CaptureInput(const SceneInput& input) { m_Input.Capture(input); }
        void FixedUpdate(float deltaTime);
        void SynchronizePresentation();
        void SetViewport(int width,int height);
        PresentationSnapshot Presentation(float alpha) const;
        void Render(MyEngine::Canvas2D& canvas,int width,int height,float alpha=1.0f);
        void Shutdown();
        void Reset();
        FarmSave CaptureSave() const;
        bool RestoreSave(const FarmSave& save);
    private:
        struct TileFocus
        {
            glm::ivec2 cell;
            InteractionTarget* target=nullptr;
        };
        std::optional<TileFocus> FocusedTile();
        std::optional<TileFocus> PointerTile(glm::vec2 world);
        void IndexTargetTiles();
        void CacheTerrainArt();
        void DrawWorldObject(MyEngine::Canvas2D& canvas,const WorldObject& object) const;
        struct FencePanel {glm::vec2 origin,size;};
        std::vector<FencePanel> m_FencePanels;
        void DrawFencePanel(MyEngine::Canvas2D& canvas,const FencePanel& panel) const;
        void DrawStaticObject(MyEngine::Canvas2D& canvas,const WorldObject& object) const;
        PresentationSnapshot CurrentPresentation() const;
        void CancelInteraction(InteractionResult reason=InteractionResult::Cancelled);
        struct SpriteDraw
        {
            const MyEngine::Texture2D* texture=nullptr;
            std::array<float,4> uv{0,0,1,1};
            SpritePlacement placement;
        };
        SpriteDraw SpriteFor(std::string_view id,int frame,glm::vec2 feet,float height,bool flip=false) const;
        SpriteDraw ActorSprite(int character,glm::vec2 feet,const AnimationController& animation) const;
        void DrawActor(MyEngine::Canvas2D& canvas,int character,glm::vec2 feet,
                       const AnimationController& animation);
        void DrawDog(MyEngine::Canvas2D& canvas,glm::vec2 feet) const;
        void DrawCrop(MyEngine::Canvas2D& canvas,const InteractionTarget& plot) const;
        MyEngine::Camera2D m_Camera,m_PickCamera;
        VisualScale m_Scale;
        bool m_GardenControl=false,m_HavePickCamera=false;
        std::optional<glm::vec2> m_HoverWorld;
        SceneInputBuffer m_Input;
        PresentationSnapshot m_Previous;
        WorldMap m_World;
        WorldArt m_WorldArt;
        std::array<std::vector<const ArtFrame*>,4> m_TerrainArt;
        int m_ArtSeason=0,m_PlayerLook=2,m_NpcLook=2;
        ForegroundTrees m_Trees;
        RenderOrder m_RenderOrder;
        InteractionActor m_Actor;
        HotbarVisibility m_Hotbar;
        bool m_StorageOpen=false;
        std::size_t m_StorageRow=0;
        TradeResult m_StorageResult=TradeResult::None;
        bool m_MarketOpen=false;
        std::size_t m_MarketRow=0;
        TradeResult m_TradeResult=TradeResult::None;
        InteractionAction m_Action;
        std::future<SaveResult> m_SaveJob;
        bool m_Loading=false;
        std::string m_SaveMessage;
        float m_SaveMessageTime=0;
        std::vector<InteractionTarget> m_Targets;
        std::unordered_map<int,std::vector<std::size_t>> m_TargetTiles;
        bool m_ShowTileGrid=false;
        EventBus m_Events;
        EffectPool m_Effects;
        InteractionResult m_LastAction=InteractionResult::NoTarget;
        float m_FeedbackTime=0;
        AssetTextures* m_Assets=nullptr;
        const MyEngine::Texture2D* m_Map=nullptr;
        const MyEngine::Texture2D* m_Items=nullptr;
        const MyEngine::Texture2D* m_DogTexture=nullptr;
        const MyEngine::Texture2D* m_CropTexture=nullptr;
        std::array<MyEngine::Texture2D,3> m_LocalCarrot;
        std::array<MyEngine::Texture2D,2> m_LocalSoil;
        MyEngine::Texture2D m_LocalCarrotIcon,m_LocalSeedIcon;
        std::array<const MyEngine::Texture2D*, 2> m_Characters{};
        std::array<std::array<float,4>,4> m_ItemRegions{};
        SceneOptions m_Options;
        VisualCatalog m_Visuals;
        AnimationController m_PlayerAnimation,m_NpcAnimation,m_DogAnimation,m_CropAnimation;
        glm::vec2 m_Player{760,620};
        glm::vec2 m_Dog{700,660};
        glm::vec2 m_Npc{2360,1950};
        Facing m_NpcFacing=Facing::Down;
        bool m_NpcPatrolRight=true;
        double m_GameAge=0;
        bool m_Moving = false;
        Facing m_Facing=Facing::Down;
        VisualState m_PlayerState=VisualState::Idle;
        int m_Character = 0, m_SelectedItem = 0;
        int m_FruitHarvested = 0;
        bool m_Ready = false;
    };
}
