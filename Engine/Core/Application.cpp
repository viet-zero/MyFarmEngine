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
        auto* window = m_Window.GetNativeWindow();
        glfwSetWindowUserPointer(window, this);
        glfwSetWindowRefreshCallback(window, [](GLFWwindow* native)
        {
            static_cast<Application*>(glfwGetWindowUserPointer(native))->Refresh();
        });
        return true;
    }

    void Application::Run(unsigned int maxFrames)
    {
        if (!m_Initialized)
            return;
        unsigned int frames = 0;
        m_Running = true;
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
            m_InFrame = true;
            m_Game.Update(Time::GetDeltaTime());
            DrawFrame();
            m_InFrame = false;
            if (maxFrames != 0 && ++frames >= maxFrames)
                break;
        }
        m_Running = false;
    }

    bool Application::DrawFrame()
    {
        int width = 0, height = 0;
        m_Window.GetFramebufferSize(width, height);
        if (width == 0 || height == 0)
            return false;
        m_Renderer.Resize(width, height);
        m_Renderer.BeginFrame();
        m_Game.Render();
        m_Renderer.EndFrame();
        m_Window.SwapBuffers();
        return true;
    }

    void Application::Refresh()
    {
        // Windows can dispatch paint/resize while PollEvents is in its modal
        // move/resize loop. Repaint the last game state without another Update.
        if (!m_Initialized || !m_Running || m_InFrame || m_Window.ShouldClose())
            return;
        m_InFrame = true;
        DrawFrame();
        m_InFrame = false;
        // Time spent dragging must not become a simulation jump on release.
        Time::Reset();
    }

    void Application::Shutdown()
    {
        m_Running = false;
        if (auto* window = m_Window.GetNativeWindow())
        {
            glfwSetWindowRefreshCallback(window, nullptr);
            glfwSetWindowUserPointer(window, nullptr);
        }
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
