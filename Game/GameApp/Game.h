#pragma once
class Game
{
public:
    bool Initialize();
    void Update(float dt);
    void Render();
    void Shutdown();

private:
    GameplayState m_gameplayState;
};