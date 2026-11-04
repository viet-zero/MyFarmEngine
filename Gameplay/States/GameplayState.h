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
        explicit GameplayState(SceneOptions options={}) : m_Demo(options) {}
        void Enter() override;
        void Update(float deltaTime) override;
        void Render() override;
        void Exit() override;
        bool IsReady() const { return m_Ready; }
        int Mode() const { return m_Mode; }
        SceneSnapshot Snapshot() const { return m_Demo.Snapshot(); }
        const SceneOptions& Options() const { return m_Demo.Options(); }

    private:
        void SetMode(int mode);
        void LoadPreview();
        void UpdateTitle();
        MyEngine::Canvas2D m_Canvas;
        AssetTextures m_Assets;
        const MyEngine::Texture2D* m_Background=nullptr;
        const MyEngine::Texture2D* m_Preview=nullptr;
        DemoScene2D m_Demo;
        int m_Mode = 3;
        std::size_t m_Selected = 0;
        bool m_Ready = false;
    };
}
