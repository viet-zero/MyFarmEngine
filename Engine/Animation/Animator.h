#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <cstdint>

namespace MyEngine
{
    struct AnimationMarker
    {
        std::string name;
        double time=0.0;
        bool operator==(const AnimationMarker&) const = default;
    };
    struct AnimationMarkerHit
    {
        std::string name;
        std::uint64_t occurrences=0;
    };
    struct AnimationClip
    {
        int firstFrame=0;
        int frameCount=1;
        float framesPerSecond=1.0f;
        bool loop=true;
        std::vector<AnimationMarker> markers{};
        double Duration() const { return static_cast<double>(frameCount)/framesPerSecond; }

        bool IsValid(int availableFrames) const;
    };

    struct AnimatorSnapshot
    {
        std::string clipKey;
        double elapsed=0.0;
        bool paused=false;
        bool started=false;
    };

    class Animator
    {
    public:
        bool Play(std::string_view clipKey,const AnimationClip& clip,int availableFrames);
        void Update(float deltaTime);
        // Valid until the next Update/Play/Restore/Reset. One bounded entry per marker.
        std::span<const AnimationMarkerHit> MarkerHits() const { return m_Hits; }
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
        bool m_Ready=false,m_Started=false;
        std::vector<AnimationMarkerHit> m_Hits;
    };
}
