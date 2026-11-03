#include "Application.h"
#include "Time.h"
#include "Input/Input.h"
#include <GLFW/glfw3.h>

namespace MyEngine
{
    Application::Application(IGame& game) : m_Game(game) {}
    Application::~Application() { Shutdown(); }

    bool Application::Initialize(unsigned int width, unsigned int height,
                                 const char* title, bool vsync)
    {
        if (m_Initialized)
            return true;
        if (!m_Window.Create(width, height, title, vsync))
            return false;
        Input::Initialize(m_Window.GetNativeWindow());
        if (!m_Renderer.Initialize())
        {
            Shutdown();
            return false;
        }
        m_GameStarted = true;
        if (!m_Game.Initialize())
        {
            Shutdown();
            return false;
        }
        Time::Reset();
        m_Initialized = true;
        return true;
    }

    void Application::Run(unsigned int maxFrames)
    {
        if (!m_Initialized)
            return;
        unsigned int frames = 0;
        Time::Reset();
        while (!m_Window.ShouldClose())
        {
            m_Window.PollEvents();
            if (m_Window.ShouldClose() || Input::IsKeyPressed(GLFW_KEY_ESCAPE))
                break;

            int width = 0, height = 0;
            m_Window.GetFramebufferSize(width, height);
            if (width == 0 || height == 0)
            {
                // Sleep in GLFW while minimized instead of spinning on the CPU.
                glfwWaitEventsTimeout(0.1);
                Time::Reset();
                continue;
            }
            Time::Update();
            m_Game.Update(Time::GetDeltaTime());
            m_Renderer.Resize(width, height);
            m_Renderer.BeginFrame();
            m_Game.Render();
            m_Renderer.EndFrame();
            m_Window.SwapBuffers();
            if (maxFrames != 0 && ++frames >= maxFrames)
                break;
        }
    }

    void Application::Shutdown()
    {
        if (m_GameStarted)
        {
            m_Game.Shutdown();
            m_GameStarted = false;
        }
        Input::Initialize(nullptr);
        m_Renderer.Shutdown();
        m_Window.Destroy();
        m_Initialized = false;
    }
}
