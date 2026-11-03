#pragma once

#include "Window.h"
#include "IGame.h"
#include "Graphics/Renderer.h"

namespace MyEngine
{
    class Application
    {
    public:
        explicit Application(IGame& game);
        ~Application();
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        bool Initialize(unsigned int width = 1280, unsigned int height = 720,
                        const char* title = "MyFarmEngine", bool vsync = true);
        // Zero runs until ESC or window close; a frame limit supports smoke tests.
        void Run(unsigned int maxFrames = 0);
        void Shutdown();

    private:
        void Refresh();
        bool DrawFrame();
        IGame& m_Game;
        Window m_Window;
        Renderer m_Renderer;
        bool m_GameStarted = false;
        bool m_Initialized = false;
        bool m_Running = false;
        bool m_InFrame = false;
    };
}
