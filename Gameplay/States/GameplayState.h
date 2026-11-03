#pragma once

#include "GameState.h"
#include "Graphics/Canvas2D.h"
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
        int m_Mode = 0;
        std::size_t m_Selected = 0;
        bool m_Ready = false;
    };
}
