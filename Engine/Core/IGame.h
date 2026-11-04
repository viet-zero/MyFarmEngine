#pragma once

namespace MyEngine
{
    // The game must outlive Application. Shutdown also cleans partial initialization.
    class IGame
    {
    public:
        virtual ~IGame() = default;
        virtual bool Initialize() = 0;
        // Once per render frame, even when no fixed tick is due.
        virtual void ProcessInput() {}
        virtual bool SimulationPaused() const { return false; }
        virtual void SynchronizePresentation() {}
        virtual void ResizeViewport(int width,int height) { (void)width;(void)height; }
        // Application always supplies 1/60 second here, zero or more calls per frame.
        virtual void Update(float deltaTime) = 0;
        virtual void Render() = 0;
        virtual void Render(float alpha) { (void)alpha;Render(); }
        virtual void Shutdown() = 0;
    };
}
