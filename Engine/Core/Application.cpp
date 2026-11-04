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
        Time::Reset();m_FixedClock={};m_WasSuspended=false;
        m_Game.SynchronizePresentation();
        while (!m_Window.ShouldClose())
        {
            m_Window.PollEvents();
            if (m_Window.ShouldClose() || Input::IsKeyPressed(GLFW_KEY_ESCAPE) ||
                Input::ConsumeKeyPress(GLFW_KEY_ESCAPE))
                break;

            int width = 0, height = 0;
            m_Window.GetFramebufferSize(width, height);
            if(width==0 || height==0 || glfwGetWindowAttrib(m_Window.GetNativeWindow(),GLFW_ICONIFIED))
            {
                AdvanceFrame(0,true);
                glfwWaitEventsTimeout(0.1);
                Time::Reset();
                continue;
            }
            Time::Update();
            const bool suspended=!glfwGetWindowAttrib(m_Window.GetNativeWindow(),GLFW_FOCUSED);
            AdvanceFrame(Time::GetFrameDeltaTime(),suspended);
            if (maxFrames != 0 && ++frames >= maxFrames)
                break;
        }
        m_Running = false;
    }

    FixedStepPlan Application::AdvanceFrame(double frameSeconds,bool suspended)
    {
        if(!m_Initialized || m_InFrame) return {};
        m_InFrame=true;
        m_Game.ProcessInput();
        int width=0,height=0;m_Window.GetFramebufferSize(width,height);
        const bool minimized=width==0 || height==0 || glfwGetWindowAttrib(m_Window.GetNativeWindow(),GLFW_ICONIFIED);
        FixedStepPlan plan;
        if(suspended || minimized || m_Game.SimulationPaused())
        {
            m_FixedClock.Reset();m_WasSuspended=true;
            Input::ClearPresses();m_Game.SynchronizePresentation();
        }
        else
        {
            if(m_WasSuspended)
            {
                frameSeconds=0;m_WasSuspended=false;
                m_FixedClock.Reset();
            }
            plan=m_FixedClock.Advance(frameSeconds);
            for(int step=0;step<plan.steps;++step)
                m_Game.Update(static_cast<float>(FixedStepClock::StepSeconds));
        }
        if(!minimized) DrawFrame();
        m_InFrame=false;
        return plan;
    }

    bool Application::DrawFrame()
    {
        int width = 0, height = 0;
        m_Window.GetFramebufferSize(width, height);
        if (width == 0 || height == 0)
            return false;
        if(width!=m_ViewWidth || height!=m_ViewHeight)
        {
            m_Game.ResizeViewport(width,height);m_ViewWidth=width;m_ViewHeight=height;
        }
        m_Renderer.Resize(width, height);
        m_Renderer.BeginFrame();
        m_Game.Render(m_FixedClock.Alpha());
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
        m_FixedClock.Reset();m_Game.SynchronizePresentation();
        DrawFrame();
        m_InFrame = false;
        // Time spent dragging must not become a simulation jump on release.
        Time::Reset();
    }

    void Application::Shutdown()
    {
        m_Running = false;
        m_FixedClock={};m_WasSuspended=false;m_ViewWidth=m_ViewHeight=0;
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
