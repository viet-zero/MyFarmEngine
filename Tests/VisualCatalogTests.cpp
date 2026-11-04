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
    Check(catalog.AssetIds().size()>=12,"directional sprite assets not registered");
    for(const auto direction:{Facing::Down,Facing::Left,Facing::Right,Facing::Up})
        for(const auto state:{VisualState::Idle,VisualState::Walk,VisualState::Run,VisualState::Action})
        {
            Check(catalog.Find("player_linh",state,direction)!=nullptr,"Linh directional clip missing");
            Check(catalog.Find("player_an",state,direction)!=nullptr,"An directional clip missing");
        }
    AnimationController player;
    Check(player.Select(catalog,"player_linh",VisualState::Walk,Facing::Right),"player walk selection failed");
    player.Update(0.4f,false);const auto elapsed=player.Save().timeline.elapsed;
    Check(player.Select(catalog,"player_linh",VisualState::Walk,Facing::Right),"same direction failed");
    Check(player.Save().timeline.elapsed==elapsed,"same clip restarted");
    const auto* right=player.Clip();
    Check(player.Select(catalog,"player_linh",VisualState::Walk,Facing::Left),"left selection failed");
    Check(player.Clip()!=right && player.Save().timeline.elapsed==0.0,"direction change did not switch/restart");
    Check(player.Select(catalog,"player_linh",VisualState::Run,Facing::Left),"run selection failed");
    Check(player.Save().timeline.elapsed==0.0,"state change did not restart");
    player.Update(1.0f,true);Check(player.Save().timeline.elapsed==0.0,"pause advanced controller");
    player.Update(0.25f,false);
    const auto snapshot=player.Save();
    AnimationController loaded;
    Check(loaded.Rebuild(catalog,snapshot),"save/load rebuild failed");
    Check(loaded.State()==VisualState::Run && loaded.Direction()==Facing::Left &&
          loaded.Frame()==player.Frame(),"rebuilt controller lost state");
    Check(player.Select(catalog,"player_linh",VisualState::Action,Facing::Left),"action clip missing");
    player.Update(0.25f,false);
    Check(player.Finished(),"one-shot action did not finish");
    Check(player.Select(catalog,"player_linh",VisualState::Action,Facing::Left,true) &&
          !player.Finished() && player.Save().timeline.elapsed==0.0,
          "new action input did not restart the finished clip");
    Check(catalog.Find("player_an",VisualState::Walk,Facing::Left)->flipX,
          "An left walk should mirror reviewed side sheet");
    Check(catalog.Find("player_linh",VisualState::Idle,Facing::Left)->flipX,
          "Linh left idle should mirror reviewed pose");
    Check(!catalog.Find("player_linh",VisualState::Walk,Facing::Left)->flipX,
          "Linh left walk has its own left-facing art");
    Check(ResolvePlayerState(true,false,false,false,VisualState::Idle,true)==VisualState::Walk &&
          ResolvePlayerState(true,true,false,false,VisualState::Walk,true)==VisualState::Run &&
          ResolvePlayerState(true,true,true,false,VisualState::Run,true)==VisualState::Action &&
          ResolvePlayerState(true,true,false,false,VisualState::Action,false)==VisualState::Action &&
          ResolvePlayerState(false,false,false,false,VisualState::Action,true)==VisualState::Idle,
          "state transitions ignored action priority or completion");

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
    const std::string eventsHeader="profile,kind,state,direction,asset_id,first_frame,frame_count,fps,loop,total_frames,flip_x,events\n";
    const std::string eventRow="hero,player,action,down,player_linh,0,1,5,0,1,0,";
    {std::ofstream file(bad);file<<eventsHeader<<eventRow<<"IMPACT@0.1|COMPLETE@0.2\n";}
    VisualCatalog eventCatalog;Check(eventCatalog.Load(bad),"explicit event schema rejected");
    AnimationController eventController;eventController.Select(eventCatalog,"hero",VisualState::Action,Facing::Down);
    eventController.Update(0.12f,false);
    Check(eventController.MarkerHits().size()==1 && eventController.MarkerHits()[0].name=="IMPACT","explicit marker not passed through controller");
    const auto* legacy=catalog.Find("player_linh",VisualState::Action,Facing::Down);
    Check(legacy && legacy->timeline.markers.size()==2 && legacy->timeline.markers[0].name=="IMPACT","legacy action missing compatible markers");
    for(const std::string events:{"IMPACT@0.05|IMPACT@0.1|COMPLETE@0.2","IMPACT@0.1","IMPACT@0|COMPLETE@0.2","IMPACT@nan|COMPLETE@0.2","COMPLETE@0.2|IMPACT@0.1","COMPLETE@0.2","IMPACT@bad|COMPLETE@0.2",""})
    {
        {std::ofstream file(bad);file<<eventsHeader<<eventRow<<events<<'\n';}
        VisualCatalog invalidEvents;Check(!invalidEvents.Load(bad),"invalid/ambiguous action event metadata accepted");
    }
    std::error_code error;std::filesystem::remove(bad,error);
    return failures==0?0:1;
}
