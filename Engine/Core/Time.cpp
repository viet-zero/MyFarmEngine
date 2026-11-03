#include "Time.h"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace MyEngine
{
    float Time::s_DeltaTime = 0.0f;
    double Time::s_LastFrameTime = 0.0;

    void Time::Reset() { Reset(glfwGetTime()); }
    void Time::Reset(double currentTime)
    {
        s_LastFrameTime = currentTime;
        s_DeltaTime = 0.0f;
    }
    void Time::Update() { Update(glfwGetTime()); }
    void Time::Update(double currentTime)
    {
        s_DeltaTime = static_cast<float>(std::clamp(
            currentTime - s_LastFrameTime, 0.0, static_cast<double>(MaxDeltaTime)));
        s_LastFrameTime = currentTime;
    }
    float Time::GetDeltaTime() { return s_DeltaTime; }
    double Time::GetTime() { return glfwGetTime(); }
}
