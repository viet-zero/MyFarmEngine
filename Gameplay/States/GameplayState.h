#pragma once

#include "GameState.h"

namespace MyGame
{
    class GameplayState : public GameState
    {
    public:
        void Enter() override;
        void Update(float deltaTime) override;
        void Render() override;
        void Exit() override;
    };
}