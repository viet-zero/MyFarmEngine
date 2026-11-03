#pragma once

namespace MyEngine
{
    // The game must outlive Application. Shutdown also cleans partial initialization.
    class IGame
    {
    public:
        virtual ~IGame() = default;
        virtual bool Initialize() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Render() = 0;
        virtual void Shutdown() = 0;
    };
}
