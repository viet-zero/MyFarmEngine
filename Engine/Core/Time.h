#pragma once
#include <cstdint>

namespace MyEngine
{
    class Time
    {
    public:
        static constexpr float MaxDeltaTime = 0.1f;
        static void Reset();
        static void Reset(double currentTime);
        static void Update();
        static void Update(double currentTime);
        static float GetDeltaTime();
        static double GetTime();
        static double GetFrameDeltaTime();

    private:
        static float s_DeltaTime;
        static double s_LastFrameTime,s_FrameDeltaTime;
    };
    struct FixedStepPlan
    {
        int steps=0;
        float alpha=0;
        double droppedSeconds=0;
    };
    // Accumulator owned by Application; Time remains the wall-clock source.
    class FixedStepClock
    {
    public:
        static constexpr double StepSeconds=1.0/60.0,MaxFrameSeconds=0.25;
        static constexpr int MaxSteps=6;
        FixedStepPlan Advance(double frameSeconds);
        void Reset() { m_Accumulator=0; }
        float Alpha() const;
        std::uint64_t TotalSteps() const { return m_TotalSteps; }
        double DroppedSeconds() const { return m_DroppedSeconds; }
    private:
        double m_Accumulator=0,m_DroppedSeconds=0;
        std::uint64_t m_TotalSteps=0;
    };
}
