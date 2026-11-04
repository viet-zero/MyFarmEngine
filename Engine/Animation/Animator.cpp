#include "Animator.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace MyEngine
{
    bool AnimationClip::IsValid(int availableFrames) const
    {
        if(firstFrame<0 || frameCount<=0 || availableFrames<=0) return false;
        if(firstFrame>availableFrames || frameCount>availableFrames-firstFrame) return false;
        if(!std::isfinite(framesPerSecond) || framesPerSecond<=0.0f) return false;
        const double duration=Duration();
        double previous=-1.0;
        for(std::size_t i=0;i<markers.size();++i)
        {
            const auto& marker=markers[i];
            if(marker.name.empty() || !std::isfinite(marker.time) || marker.time<0.0 ||
               marker.time>duration || marker.time<previous) return false;
            for(std::size_t j=0;j<i;++j)
                if(markers[j]==marker) return false;
            previous=marker.time;
        }
        return true;
    }

    bool Animator::Play(std::string_view clipKey,const AnimationClip& clip,int availableFrames)
    {
        if(clipKey.empty() || !clip.IsValid(availableFrames)) return false;
        if(m_Ready && m_Key==clipKey && m_Clip.firstFrame==clip.firstFrame &&
           m_Clip.frameCount==clip.frameCount && m_Clip.framesPerSecond==clip.framesPerSecond &&
           m_Clip.loop==clip.loop && m_Clip.markers==clip.markers) return true;
        m_Key=clipKey;
        m_Clip=clip;
        m_Elapsed=0.0;
        m_Ready=true;m_Started=false;m_Hits.clear();
        return true;
    }

    void Animator::Update(float deltaTime)
    {
        m_Hits.clear();
        if(!m_Ready || m_Paused || !std::isfinite(deltaTime) || deltaTime<=0.0f) return;
        const double duration=m_Clip.Duration();
        const double next=m_Elapsed+static_cast<double>(deltaTime);
        if(!std::isfinite(next)) return;
        for(const auto& marker:m_Clip.markers)
        {
            double count=0;
            if(m_Clip.loop)
                count=std::floor((next-marker.time)/duration)-std::floor((m_Elapsed-marker.time)/duration);
            else if(marker.time>m_Elapsed && marker.time<=std::min(next,duration)) count=1;
            if(!m_Started && marker.time==0.0) count+=1;
            if(count>0)
            {
                const auto limit=std::numeric_limits<std::uint64_t>::max();
                const auto occurrences=count>=static_cast<double>(limit)?limit:static_cast<std::uint64_t>(count);
                m_Hits.push_back({marker.name,occurrences});
            }
        }
        m_Elapsed=m_Clip.loop?std::fmod(next,duration):std::min(next,duration);
        m_Started=true;
    }

    bool Animator::Finished() const
    {
        if(!m_Ready || m_Clip.loop) return false;
        return m_Elapsed>=static_cast<double>(m_Clip.frameCount)/m_Clip.framesPerSecond;
    }

    int Animator::Frame() const
    {
        if(!m_Ready) return 0;
        auto relative=static_cast<int>(m_Elapsed*static_cast<double>(m_Clip.framesPerSecond));
        relative=std::clamp(relative,0,m_Clip.frameCount-1);
        return m_Clip.firstFrame+relative;
    }

    AnimatorSnapshot Animator::Save() const { return {m_Key,m_Elapsed,m_Paused,m_Started}; }

    bool Animator::Restore(const AnimatorSnapshot& snapshot,const AnimationClip& clip,int availableFrames)
    {
        if(snapshot.clipKey.empty() || !clip.IsValid(availableFrames)) return false;
        m_Key=snapshot.clipKey;
        m_Clip=clip;
        m_Elapsed=0.0;
        m_Ready=true;
        if(std::isfinite(snapshot.elapsed) && snapshot.elapsed>=0.0)
        {
            const double duration=static_cast<double>(clip.frameCount)/clip.framesPerSecond;
            m_Elapsed=clip.loop?std::fmod(snapshot.elapsed,duration):std::min(snapshot.elapsed,duration);
        }
        m_Paused=snapshot.paused;
        m_Started=snapshot.started || m_Elapsed>0.0;m_Hits.clear();
        return true;
    }

    void Animator::Reset()
    {
        m_Key.clear();m_Clip={};m_Elapsed=0.0;m_Paused=false;m_Ready=false;m_Started=false;m_Hits.clear();
    }
}
