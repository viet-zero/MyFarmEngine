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
    const auto hits=[](const Animator& value,std::string_view name)
    {std::uint64_t count=0;for(const auto& hit:value.MarkerHits())if(hit.name==name)count+=hit.occurrences;return count;};
    AnimationClip marked{0,4,4,false};
    marked.markers={{"START",0},{"FX",0.1},{"IMPACT",0.25},{"COMPLETE",1}};
    Check(animator.Play("marked",marked,4),"marked clip rejected");
    animator.SetPaused(false);animator.Update(0);
    Check(animator.MarkerHits().empty(),"zero dt fired marker");
    animator.Update(0.15f);
    Check(hits(animator,"START")==1 && hits(animator,"FX")==1 && hits(animator,"IMPACT")==0,"multiple early markers skipped");
    animator.SetPaused(true);animator.Update(1);
    Check(animator.MarkerHits().empty(),"paused marker repeated");
    animator.SetPaused(false);animator.Update(0.1f);
    Check(hits(animator,"IMPACT")==1,"impact boundary missed");
    const auto afterImpact=animator.Save();
    Animator restored;Check(restored.Restore(afterImpact,marked,4),"marked snapshot rejected");
    restored.Update(0.1f);Check(hits(restored,"IMPACT")==0 && hits(restored,"START")==0,"restore replayed past impact/start");
    animator.Update(100);
    Check(hits(animator,"COMPLETE")==1 && hits(animator,"IMPACT")==0 && animator.Finished(),"end marker or one-shot clamp failed");
    animator.Update(1);Check(animator.MarkerHits().empty(),"finished markers replayed");
    animator.Reset();animator.Play("marked",marked,4);animator.Update(2);
    Check(hits(animator,"START")==1 && hits(animator,"FX")==1 && hits(animator,"IMPACT")==1 && hits(animator,"COMPLETE")==1,"large dt skipped one-shot markers");
    AnimationClip looping{0,4,4,true};looping.markers={{"ZERO",0},{"MIDDLE",0.5},{"END",1}};
    animator.Play("looping",looping,4);animator.Update(2.5f);
    Check(hits(animator,"ZERO")==3 && hits(animator,"MIDDLE")==3 && hits(animator,"END")==2,"multi-loop event counts wrong");
    animator.Update(0.5f);const auto boundary=animator.Save();
    Check(hits(animator,"ZERO")==1 && hits(animator,"MIDDLE")==0 && hits(animator,"END")==1,"loop boundary repeated middle or skipped zero/end");
    restored.Restore(boundary,looping,4);restored.Update(0.1f);
    Check(hits(restored,"ZERO")==0,"loop restore fired already-emitted start");
    animator.Update(std::numeric_limits<float>::max());
    Check(animator.MarkerHits().size()<=looping.markers.size() && hits(animator,"ZERO")==std::numeric_limits<std::uint64_t>::max(),"large loop update was not bounded/saturated");
    const auto elapsed=animator.Elapsed();animator.Update(std::nanf(""));
    Check(animator.MarkerHits().empty() && animator.Elapsed()==elapsed,"invalid dt retained marker hits");
    Animator original;original.Play("copy",marked,4);original.Update(0.15f);
    Animator independent=original;original.Reset();
    Check(hits(independent,"START")==1 && hits(independent,"FX")==1,"copied event payload referenced another animator's storage");
    auto invalid=marked;invalid.markers={{"BAD",1.1}};Check(!invalid.IsValid(4),"marker outside duration accepted");
    invalid.markers={{"BAD",0.5},{"EARLY",0.1}};Check(!invalid.IsValid(4),"unsorted markers accepted");
    invalid.markers={{"BAD",0.5},{"BAD",0.5}};Check(!invalid.IsValid(4),"duplicate marker accepted");
    invalid.markers={{"",0.1}};Check(!invalid.IsValid(4),"empty marker accepted");
    invalid.markers={{"BAD",std::nan("")}};Check(!invalid.IsValid(4),"NaN marker accepted");
    return failures==0?0:1;
}
