#pragma once
#include <algorithm>
#include <cmath>
namespace MyGame
{
    class HotbarVisibility
    {
    public:
        static constexpr double VisibleSeconds=4.0,FadeSeconds=0.6;
        void Update(double seconds,bool activity)
        {
            if(activity){m_Remaining=VisibleSeconds;return;}
            if(std::isfinite(seconds) && seconds>0) m_Remaining=std::max(0.0,m_Remaining-seconds);
        }
        float Alpha() const {return static_cast<float>(std::clamp(m_Remaining/FadeSeconds,0.0,1.0));}
    private:
        double m_Remaining=VisibleSeconds;
    };
}
