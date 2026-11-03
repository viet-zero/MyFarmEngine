#pragma once

#include "GameState.h"
#include "Graphics/Canvas2D.h"
#include "Gameplay/World/DemoScene2D.h"
#include <array>
#include <cstddef>

namespace MyGame
{
    class GameplayState : public GameState
    {
    public:
        void Enter() override;
        void Update(float deltaTime) override;
        void Render() override;
        void Exit() override;
        bool IsReady() const { return m_Ready; }

    private:
        void SetMode(int mode);
        void LoadPreview();
        void UpdateTitle();
        MyEngine::Canvas2D m_Canvas;
        MyEngine::Texture2D m_Background, m_Preview;
        DemoScene2D m_Demo;
        int m_Mode = 3;
        std::size_t m_Selected = 0;
        bool m_Ready = false;
    };
}
