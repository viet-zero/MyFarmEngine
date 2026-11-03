#pragma once
#include "Animation/Animator.h"
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace MyGame
{
    enum class VisualState {Idle,Walk,Run,Action,Hurt,Sleep};
    enum class Facing {Any,Down,Left,Right,Up};

    struct VisualClip
    {
        std::string assetId;
        MyEngine::AnimationClip timeline;
        Facing facing=Facing::Any;
        int availableFrames=1;
        bool flipX=false;
    };

    class VisualCatalog
    {
    public:
        bool Load(const std::filesystem::path& csv);
        const VisualClip* Find(std::string_view profile,VisualState state,Facing facing) const;
        std::vector<std::string> AssetIds() const;
        const std::vector<std::string>& Errors() const { return m_Errors; }
    private:
        std::map<std::string,VisualClip> m_Clips;
        std::vector<std::string> m_Errors;
    };

    struct VisualAnimationSnapshot
    {
        std::string profile;
        VisualState state=VisualState::Idle;
        Facing facing=Facing::Down;
        MyEngine::AnimatorSnapshot timeline;
    };

    class AnimationController
    {
    public:
        bool Select(const VisualCatalog& catalog,std::string_view profile,VisualState state,Facing facing,
                    bool restart=false);
        void Update(float deltaTime,bool paused);
        int Frame() const { return m_Animator.Frame(); }
        bool Finished() const { return m_Animator.Finished(); }
        const VisualClip* Clip() const { return m_Clip; }
        VisualState State() const { return m_State; }
        Facing Direction() const { return m_Facing; }
        VisualAnimationSnapshot Save() const;
        bool Rebuild(const VisualCatalog& catalog,const VisualAnimationSnapshot& snapshot);
    private:
        MyEngine::Animator m_Animator;
        const VisualClip* m_Clip=nullptr;
        std::string m_Profile;
        VisualState m_State=VisualState::Idle;
        Facing m_Facing=Facing::Down;
    };

    Facing FacingFromVelocity(float x,float y,Facing previous);
    VisualState ResolvePlayerState(bool moving,bool running,bool actionTriggered,bool hurtTriggered,
                                   VisualState current,bool currentFinished);
    int GrowthFrame(double age,const std::vector<double>& stageDurations);
}
