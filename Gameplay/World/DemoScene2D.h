#pragma once
#include "Graphics/Canvas2D.h"
#include "Gameplay/Graphics/AssetTextures.h"
#include "Gameplay/Graphics/VisualCatalog.h"
#include "Gameplay/World/WorldMap.h"
#include "Gameplay/Systems/InteractionSystem.h"
#include "Gameplay/Effects/EffectPool.h"
#include <array>

namespace MyGame
{
    class DemoScene2D
    {
    public:
        bool Initialize(AssetTextures& assets,const std::filesystem::path& dataRoot);
        void Update(float deltaTime);
        void Render(MyEngine::Canvas2D& canvas, int width, int height);
        void Shutdown();
        void Reset();
    private:
        void DrawActor(MyEngine::Canvas2D& canvas,int character,glm::vec2 feet,
                       const AnimationController& animation,float bob);
        void DrawDog(MyEngine::Canvas2D& canvas) const;
        void DrawCrop(MyEngine::Canvas2D& canvas) const;
        MyEngine::Camera2D m_Camera;
        WorldMap m_World;
        InteractionActor m_Actor;
        std::vector<InteractionTarget> m_Targets;
        EventBus m_Events;
        EffectPool m_Effects;
        InteractionResult m_LastAction=InteractionResult::NoTarget;
        float m_FeedbackTime=0;
        AssetTextures* m_Assets=nullptr;
        const MyEngine::Texture2D* m_Map=nullptr;
        const MyEngine::Texture2D* m_Items=nullptr;
        std::array<const MyEngine::Texture2D*, 2> m_Characters{};
        std::array<std::array<float,4>,4> m_ItemRegions{};
        std::array<glm::vec2,2> m_Anchors{glm::vec2(0.5f,1),glm::vec2(0.5f,1)};
        VisualCatalog m_Visuals;
        AnimationController m_PlayerAnimation,m_NpcAnimation,m_DogAnimation,m_CropAnimation;
        glm::vec2 m_Player{760,620};
        glm::vec2 m_Dog{700,660};
        float m_WalkTime = 0;
        float m_NpcTime=0,m_DogTime=0;
        double m_GameAge=0;
        bool m_Moving = false;
        Facing m_Facing=Facing::Down;
        VisualState m_PlayerState=VisualState::Idle;
        int m_Character = 0, m_SelectedItem = 0;
        bool m_Ready = false;
    };
}
