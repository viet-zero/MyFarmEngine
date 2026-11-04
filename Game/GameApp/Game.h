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
        void Update(float deltaTime) override;
        void Render() override;
        void Shutdown() override;

    private:
        GameplayState m_gameplayState;
    };
}
