class GameplayState
{
public:
    void Enter();
    void Update(float dt);
    void Render();
    void Exit();

private:
    Player m_player;
    World m_world;
};