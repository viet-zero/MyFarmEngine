#include "Core/Time.h"
#include <cmath>
#include <iostream>

int main()
{
    using MyEngine::Time;
    int failures = 0;
    const auto expect = [&](float expected, const char* label)
    {
        if (std::abs(Time::GetDeltaTime() - expected) > 0.00001f)
        {
            std::cerr << label << ": unexpected delta " << Time::GetDeltaTime() << '\n';
            ++failures;
        }
    };
    Time::Reset(100.0);
    expect(0.0f, "reset");
    Time::Update(100.016);
    expect(0.016f, "normal frame");
    Time::Update(105.0);
    expect(Time::MaxDeltaTime, "stall cap");
    Time::Update(104.0);
    expect(0.0f, "backward clock");
    Time::Reset(1000000000.0);
    Time::Update(1000000000.016);
    expect(0.016f, "long uptime precision");
    Time::Reset(42.0);
    expect(0.0f, "restart reset");
    return failures == 0 ? 0 : 1;
}
