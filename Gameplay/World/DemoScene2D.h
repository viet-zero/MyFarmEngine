#pragma once
#include "Graphics/Canvas2D.h"
#include "Gameplay/Graphics/AssetTextures.h"
#include <array>

namespace MyGame
{
    class DemoScene2D
    {
    public:
        bool Initialize(AssetTextures& assets);
        void Update(float deltaTime);
        void Render(MyEngine::Canvas2D& canvas, int width, int height);
        void Shutdown();
        void Reset();
    private:
        void DrawActor(MyEngine::Canvas2D& canvas, int character, glm::vec2 feet, float bob);
        MyEngine::Camera2D m_Camera;
        const MyEngine::Texture2D* m_Map=nullptr;
        const MyEngine::Texture2D* m_Items=nullptr;
        std::array<const MyEngine::Texture2D*, 2> m_Characters{};
        std::array<std::array<float,4>,4> m_ItemRegions{};
        std::array<glm::vec2,2> m_Anchors{glm::vec2(0.5f,1),glm::vec2(0.5f,1)};
        glm::vec2 m_Player{760,620};
        float m_WalkTime = 0;
        bool m_Moving = false;
        int m_Character = 0, m_SelectedItem = 0;
        bool m_Ready = false;
    };
}
