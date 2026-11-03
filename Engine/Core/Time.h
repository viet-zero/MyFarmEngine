#pragma once

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

    private:
        static float s_DeltaTime;
        static double s_LastFrameTime;
    };
}
