#pragma once
#include "Gameplay/States/GameplayState.h"
namespace MyGame
{
    class Game
    {
    public:
        bool Initialize();
        void Update(float deltaTime);
        void Render();
        void Shutdown();

    private:
        GameplayState m_gameplayState;
    };
}