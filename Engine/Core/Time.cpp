#include "Time.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace MyEngine
{
    float Time::s_DeltaTime = 0.0f;
    double Time::s_LastFrameTime = 0.0;
    double Time::s_FrameDeltaTime = 0.0;

    void Time::Reset() { Reset(glfwGetTime()); }
    void Time::Reset(double currentTime)
    {
        s_LastFrameTime = std::isfinite(currentTime)?currentTime:0.0;
        s_DeltaTime = 0.0f;s_FrameDeltaTime=0;
    }
    void Time::Update() { Update(glfwGetTime()); }
    void Time::Update(double currentTime)
    {
        s_DeltaTime=0;s_FrameDeltaTime=0;
        if(!std::isfinite(currentTime)) return;
        const double elapsed=currentTime-s_LastFrameTime;
        s_LastFrameTime=currentTime;
        if(!std::isfinite(elapsed) || elapsed<=0) return;
        s_FrameDeltaTime=elapsed;
        s_DeltaTime=static_cast<float>(std::min(elapsed,static_cast<double>(MaxDeltaTime)));
    }
    float Time::GetDeltaTime() { return s_DeltaTime; }
    double Time::GetTime() { return glfwGetTime(); }
    double Time::GetFrameDeltaTime() { return s_FrameDeltaTime; }
    float FixedStepClock::Alpha() const
    {return static_cast<float>(std::clamp(m_Accumulator/StepSeconds,0.0,1.0));}
    FixedStepPlan FixedStepClock::Advance(double frameSeconds)
    {
        FixedStepPlan plan;plan.alpha=Alpha();
        if(!std::isfinite(frameSeconds) || frameSeconds<=0) return plan;
        const double accepted=std::min(frameSeconds,MaxFrameSeconds);
        m_Accumulator+=accepted;
        const int available=static_cast<int>(std::floor((m_Accumulator+StepSeconds*1e-9)/StepSeconds));
        plan.steps=std::min(available,MaxSteps);
        plan.droppedSeconds=frameSeconds-accepted+(available-plan.steps)*StepSeconds;
        // Keep only the fractional remainder. Old whole-tick debt never spirals next frame.
        m_Accumulator=std::max(0.0,m_Accumulator-available*StepSeconds);
        plan.alpha=Alpha();
        const auto limit=std::numeric_limits<std::uint64_t>::max();
        m_TotalSteps=m_TotalSteps>limit-static_cast<std::uint64_t>(plan.steps)?limit:m_TotalSteps+plan.steps;
        m_DroppedSeconds=std::min(std::numeric_limits<double>::max(),m_DroppedSeconds+plan.droppedSeconds);
        return plan;
    }
}
