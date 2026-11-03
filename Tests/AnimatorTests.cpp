#include "Animation/Animator.h"
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
    int failures=0;
    void Check(bool value,const char* message)
    {
        if(!value){std::cerr<<message<<'\n';++failures;}
    }
}

int main()
{
    using MyEngine::AnimationClip;
    using MyEngine::Animator;
    Check(!AnimationClip{-1,1,8,true}.IsValid(8),"negative first frame accepted");
    Check(!AnimationClip{7,2,8,true}.IsValid(8),"clip exceeded frame bounds");
    Check(!AnimationClip{0,1,0,true}.IsValid(8),"zero fps accepted");
    Check(!AnimationClip{0,1,std::numeric_limits<float>::quiet_NaN(),true}.IsValid(8),"NaN fps accepted");

    Animator animator;
    const AnimationClip loop{2,3,10,true};
    Check(animator.Play("walk.down",loop,8),"valid loop clip rejected");
    animator.Update(0.11f);
    Check(animator.Frame()==3,"loop did not advance");
    const auto before=animator.Elapsed();
    Check(animator.Play("walk.down",loop,8),"same clip rejected");
    Check(animator.Elapsed()==before,"same clip restarted every frame");
    animator.Update(0.21f);
    Check(animator.Frame()==2,"loop did not wrap");

    animator.SetPaused(true);
    const int pausedFrame=animator.Frame();
    animator.Update(10.0f);
    Check(animator.Frame()==pausedFrame,"paused animator advanced");
    animator.SetPaused(false);
    animator.Update(-1.0f);
    animator.Update(std::numeric_limits<float>::quiet_NaN());
    animator.Update(std::numeric_limits<float>::infinity());
    Check(animator.Frame()==pausedFrame,"invalid dt changed frame");

    const AnimationClip action{5,2,4,false};
    Check(animator.Play("action.down",action,8),"non-loop clip rejected");
    animator.Update(9.0f);
    Check(animator.Frame()==6 && animator.Finished(),"non-loop clip did not clamp at last frame");
    Check(animator.Play("idle.down",{0,1,2,true},8),"state change failed");
    Check(animator.Frame()==0 && animator.Elapsed()==0.0,"state change did not restart once");

    animator.Update(0.4f);
    animator.SetPaused(true);
    const auto saved=animator.Save();
    Animator rebuilt;
    Check(rebuilt.Restore(saved,{0,1,2,true},8),"snapshot restore failed");
    Check(rebuilt.Frame()==animator.Frame() && rebuilt.Paused(),"snapshot did not rebuild timeline");
    auto broken=saved;broken.elapsed=std::numeric_limits<double>::quiet_NaN();
    Check(rebuilt.Restore(broken,{0,1,2,true},8) && rebuilt.Elapsed()==0.0,"bad save time was not sanitized");
    return failures==0?0:1;
}
