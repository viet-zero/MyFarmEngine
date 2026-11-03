#include "Gameplay/Graphics/VisualCatalog.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
{
    int failures=0;
    void Check(bool value,const char* message){if(!value){std::cerr<<message<<'\n';++failures;}}
}

int main(int argc,char** argv)
{
    using namespace MyGame;
    Check(argc==2,"missing manifest argument");if(argc!=2) return 1;
    VisualCatalog catalog;
    Check(catalog.Load(argv[1]),"default visual catalog failed to load");
    AnimationController player;
    Check(player.Select(catalog,"player_linh",VisualState::Walk,Facing::Right),"player walk selection failed");
    player.Update(0.4f,false);const auto elapsed=player.Save().timeline.elapsed;
    Check(player.Select(catalog,"player_linh",VisualState::Walk,Facing::Left),"fallback direction failed");
    Check(player.Save().timeline.elapsed==elapsed,"same any-direction clip restarted");
    Check(player.Select(catalog,"player_linh",VisualState::Run,Facing::Left),"run selection failed");
    Check(player.Save().timeline.elapsed==0.0,"state change did not restart");
    player.Update(1.0f,true);Check(player.Save().timeline.elapsed==0.0,"pause advanced controller");
    player.Update(0.25f,false);
    const auto snapshot=player.Save();
    AnimationController loaded;
    Check(loaded.Rebuild(catalog,snapshot),"save/load rebuild failed");
    Check(loaded.State()==VisualState::Run && loaded.Direction()==Facing::Left &&
          loaded.Frame()==player.Frame(),"rebuilt controller lost state");

    AnimationController npc;
    Check(npc.Select(catalog,"npc_an",VisualState::Walk,Facing::Left),"NPC left clip missing");
    const auto* left=npc.Clip();
    Check(npc.Select(catalog,"npc_an",VisualState::Walk,Facing::Right) && npc.Clip()!=left,
          "NPC direction did not choose a distinct clip");
    AnimationController dog;
    Check(dog.Select(catalog,"dog",VisualState::Sleep,Facing::Down),"dog sleep state missing");
    AnimationController crop;
    Check(crop.Select(catalog,"crop_turnip",VisualState::Idle,Facing::Any),"crop profile missing");
    Check(GrowthFrame(0,{2,3,4,5})==0 && GrowthFrame(2,{2,3,4,5})==1 &&
          GrowthFrame(500,{2,3,4,5})==3,"crop growth frame selection failed");
    Check(GrowthFrame(std::nan(""),{2,3})==0 && GrowthFrame(4,{2,-1})==0,"invalid crop age/duration unsafe");
    Check(FacingFromVelocity(2,1,Facing::Down)==Facing::Right &&
          FacingFromVelocity(0,-2,Facing::Down)==Facing::Up &&
          FacingFromVelocity(0,0,Facing::Left)==Facing::Left,"velocity direction selection failed");

    const auto bad=std::filesystem::temp_directory_path()/"myfarm-bad-visual-clips.csv";
    {std::ofstream file(bad);file<<"profile,kind,state,direction,asset_id,first_frame,frame_count,fps,loop,total_frames\n"
        "bad,npc,walk,down,x,3,2,8,1,4\n";}
    VisualCatalog rejected;Check(!rejected.Load(bad) && !rejected.Errors().empty(),"out-of-bounds clip accepted");
    std::error_code error;std::filesystem::remove(bad,error);
    return failures==0?0:1;
}
