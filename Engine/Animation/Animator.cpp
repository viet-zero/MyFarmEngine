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
        return std::isfinite(framesPerSecond) && framesPerSecond>0.0f;
    }

    bool Animator::Play(std::string_view clipKey,const AnimationClip& clip,int availableFrames)
    {
        if(clipKey.empty() || !clip.IsValid(availableFrames)) return false;
        if(m_Ready && m_Key==clipKey && m_Clip.firstFrame==clip.firstFrame &&
           m_Clip.frameCount==clip.frameCount && m_Clip.framesPerSecond==clip.framesPerSecond &&
           m_Clip.loop==clip.loop) return true;
        m_Key=clipKey;
        m_Clip=clip;
        m_Elapsed=0.0;
        m_Ready=true;
        return true;
    }

    void Animator::Update(float deltaTime)
    {
        if(!m_Ready || m_Paused || !std::isfinite(deltaTime) || deltaTime<=0.0f) return;
        const double next=m_Elapsed+static_cast<double>(deltaTime);
        if(!std::isfinite(next)) return;
        const double duration=static_cast<double>(m_Clip.frameCount)/m_Clip.framesPerSecond;
        if(m_Clip.loop) m_Elapsed=std::fmod(next,duration);
        else m_Elapsed=std::min(next,duration);
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

    AnimatorSnapshot Animator::Save() const { return {m_Key,m_Elapsed,m_Paused}; }

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
        return true;
    }

    void Animator::Reset()
    {
        m_Key.clear();m_Clip={};m_Elapsed=0.0;m_Paused=false;m_Ready=false;
    }
}
