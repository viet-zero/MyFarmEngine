#pragma once
#include <cstddef>
#include <string>
#include <string_view>

namespace MyEngine
{
    struct AnimationClip
    {
        int firstFrame=0;
        int frameCount=1;
        float framesPerSecond=1.0f;
        bool loop=true;

        bool IsValid(int availableFrames) const;
    };

    struct AnimatorSnapshot
    {
        std::string clipKey;
        double elapsed=0.0;
        bool paused=false;
    };

    class Animator
    {
    public:
        bool Play(std::string_view clipKey,const AnimationClip& clip,int availableFrames);
        void Update(float deltaTime);
        void SetPaused(bool paused) { m_Paused=paused; }
        bool Paused() const { return m_Paused; }
        bool Finished() const;
        int Frame() const;
        std::string_view ClipKey() const { return m_Key; }
        double Elapsed() const { return m_Elapsed; }
        AnimatorSnapshot Save() const;
        bool Restore(const AnimatorSnapshot& snapshot,const AnimationClip& clip,int availableFrames);
        void Reset();

    private:
        std::string m_Key;
        AnimationClip m_Clip{};
        double m_Elapsed=0.0;
        bool m_Paused=false;
        bool m_Ready=false;
    };
}
