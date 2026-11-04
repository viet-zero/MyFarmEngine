#pragma once

#include "Core/IGame.h"
#include "Gameplay/States/GameplayState.h"

namespace MyGame
{
    class Game : public MyEngine::IGame
    {
    public:
        explicit Game(SceneOptions options={}) : m_gameplayState(options) {}
        bool Initialize() override;
        void ProcessInput() override { m_gameplayState.ProcessInput(); }
        bool SimulationPaused() const override { return m_gameplayState.SimulationPaused(); }
        void SynchronizePresentation() override { m_gameplayState.SynchronizePresentation(); }
        void ResizeViewport(int width,int height) override { m_gameplayState.ResizeViewport(width,height); }
        void Update(float deltaTime) override;
        void Render() override;
        void Render(float alpha) override { m_gameplayState.Render(alpha); }
        void Shutdown() override;

    private:
        GameplayState m_gameplayState;
    };
}
