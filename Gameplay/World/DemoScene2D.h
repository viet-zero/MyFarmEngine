#pragma once
#include "Graphics/Canvas2D.h"
#include "Gameplay/Graphics/AssetTextures.h"
#include "Gameplay/Graphics/VisualCatalog.h"
#include "Gameplay/World/WorldMap.h"
#include "Gameplay/World/ForegroundTrees.h"
#include "Gameplay/Graphics/RenderOrder.h"
#include "Gameplay/Systems/InteractionSystem.h"
#include "Gameplay/Effects/EffectPool.h"
#include <array>
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
    private:
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
        void DrawCrop(MyEngine::Canvas2D& canvas) const;
        MyEngine::Camera2D m_Camera;
        SceneInputBuffer m_Input;
        PresentationSnapshot m_Previous;
        WorldMap m_World;
        ForegroundTrees m_Trees;
        RenderOrder m_RenderOrder;
        InteractionActor m_Actor;
        InteractionAction m_Action;
        std::vector<InteractionTarget> m_Targets;
        EventBus m_Events;
        EffectPool m_Effects;
        InteractionResult m_LastAction=InteractionResult::NoTarget;
        float m_FeedbackTime=0;
        AssetTextures* m_Assets=nullptr;
        const MyEngine::Texture2D* m_Map=nullptr;
        const MyEngine::Texture2D* m_Items=nullptr;
        const MyEngine::Texture2D* m_DogTexture=nullptr;
        const MyEngine::Texture2D* m_CropTexture=nullptr;
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
