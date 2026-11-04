#include "Gameplay/Graphics/VisualCatalog.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace
{
    int failures=0,checks=0;
    void Check(bool ok,const std::string& message){++checks;if(!ok){++failures;std::cerr<<message<<'\n';}}
    void Text(const std::filesystem::path& path,const std::string& value)
    {std::ofstream file(path,std::ios::binary);file<<value;if(!file)throw std::runtime_error("fixture write failed");}
    bool Contains(const std::vector<std::string>& errors,std::string_view value)
    {for(const auto& error:errors)if(error.find(value)!=std::string::npos)return true;return false;}
    const std::string AssetHeader="asset_id,kind,source,export_path,columns,rows,frame_width,frame_height,anchor_x,anchor_y,has_alpha,status,first_frame,frame_count,margin_x,margin_y,spacing_x,spacing_y\n";
    const std::string VisualHeader="profile,kind,state,direction,asset_id,first_frame,frame_count,fps,loop,total_frames,flip_x\n";
    const std::string CoverageHeader="schema_version,profile,kind,state,directions,pivot_policy,layer_policy\n";
    std::string Asset(std::string status="ready",std::string kind="character",int first=0,int count=4,int width=2)
    {return "fixture,"+kind+",Original/fixture.png,Export/fixture.png,2,2,"+std::to_string(width)+",2,0.5,1,true,"+status+","+std::to_string(first)+","+std::to_string(count)+",0,0,0,0\n";}
    std::string Clip(std::string id="fixture",int first=0,int count=4,int total=4,std::string direction="down",std::string state="walk",std::string kind="player")
    {return "hero,"+kind+","+state+","+direction+","+id+","+std::to_string(first)+","+std::to_string(count)+",6,1,"+std::to_string(total)+",0\n";}
    std::string Rule(std::string directions="down",std::string version="1",std::string pivot="alpha_bounds_v1",std::string layer="single_sprite_v1",std::string state="walk")
    {return version+",hero,player,"+state+","+directions+","+pivot+","+layer+"\n";}
}
int main(int argc,char** argv)
{
    if(argc!=5)return 2;
    using namespace MyGame;
    AssetManifest source;Check(source.Load(argv[2],argv[1]),"source asset catalog failed");
    VisualCatalog current;Check(current.Load(argv[3]),"current clips failed");
    Check(current.ClipCount()>0 && current.ValidateAgainst(source,argv[4]).empty(),"actual declared catalog did not cross-validate");
    const auto root=std::filesystem::temp_directory_path()/
        ("myfarm_visual_validation_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    if(!std::filesystem::create_directory(root))throw std::runtime_error("unique test directory unavailable");
    struct Cleanup{std::filesystem::path path;~Cleanup(){std::error_code error;std::filesystem::remove_all(path,error);}} cleanup{root};
    std::filesystem::create_directory(root/"Original");std::filesystem::create_directory(root/"Export");
    // Deterministic 4x4 RGBA fixture, unrelated to shipped artwork or its dimensions.
    constexpr unsigned char png[]={137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,4,0,0,0,4,8,6,0,0,0,169,241,158,126,0,0,0,20,73,68,65,84,120,156,99,208,216,18,197,0,196,255,97,152,129,116,1,0,6,98,31,85,218,46,197,150,0,0,0,0,73,69,78,68,174,66,96,130};
    for(const auto& path:{root/"Original/fixture.png",root/"Export/fixture.png"})
    {
        std::ofstream file(path,std::ios::binary);
        file.write(reinterpret_cast<const char*>(png),static_cast<std::streamsize>(sizeof(png)));
        if(!file)throw std::runtime_error("PNG fixture write failed");
    }
    const auto assetsCsv=root/"Assets.csv",visualsCsv=root/"VisualClips.csv",coverageCsv=root/"Coverage.csv";
    AssetManifest assets;VisualCatalog visuals;
    const auto run=[&](const std::string& name,const std::string& asset,const std::string& clip,
                       const std::string& coverage,const std::string& expected,bool parse=true)
    {
        Text(assetsCsv,AssetHeader+asset);Text(visualsCsv,VisualHeader+clip);Text(coverageCsv,CoverageHeader+coverage);
        Check(assets.Load(assetsCsv,root),name+": assets load failed");
        Check(visuals.Load(visualsCsv)==parse,name+": unexpected visual parser status");
        const auto errors=visuals.ValidateAgainst(assets,coverageCsv);
        Check(expected.empty()?errors.empty():Contains(errors,expected),name+": expected "+expected);
        if(!expected.empty() && !Contains(errors,expected))for(const auto& error:errors)std::cerr<<error<<'\n';
    };
    run("valid",Asset(),Clip(),Rule(),"");
    run("unknown reference",Asset(),Clip("missing"),Rule(),"visual_row_2:hero|walk|down:asset=missing:unknown_asset");
    run("not reviewed",Asset("needs_review"),Clip(),Rule(),"asset_not_ready");
    run("disabled",Asset("disabled"),Clip(),Rule(),"asset_not_ready");
    run("wrong asset type",Asset("ready","animal"),Clip(),Rule(),"asset_kind_mismatch");
    run("wrong profile type",Asset(),Clip("fixture",0,4,4,"down","walk","npc"),Rule(),"profile_kind_mismatch");
    run("unsupported clip kind",Asset(),Clip("fixture",0,4,4,"down","walk","unknown"),Rule(),"asset_kind_mismatch");
    run("fake total",Asset(),Clip("fixture",0,4,5),Rule(),"total_frames_mismatch");
    run("bad decoded grid",Asset("ready","character",0,4,3),Clip(),Rule(),"asset_invalid:grid_dimensions_mismatch");
    run("nonzero window valid",Asset("ready","character",2,2),Clip("fixture",2,2),Rule(),"");
    run("before asset window",Asset("ready","character",2,2),Clip("fixture",1,2),Rule(),"clip_outside_asset_window");
    run("after asset window",Asset("ready","character",0,2),Clip("fixture",1,2),Rule(),"clip_outside_asset_window");
    run("missing direction",Asset(),Clip(),Rule("down|left"),"coverage_row_2:hero|walk:missing_direction:left");
    run("any is not directional artwork",Asset(),Clip("fixture",0,4,4,"any"),Rule("down|left|right|up"),"missing_direction:up");
    run("declared any valid",Asset(),Clip("fixture",0,4,4,"any"),Rule("any"),"");
    run("undeclared direction",Asset(),Clip(),Rule("left"),"direction_not_declared");
    run("missing state contract",Asset(),Clip()+Clip("fixture",0,1,4,"down","idle"),Rule(),"coverage_not_declared");
    run("future schema",Asset(),Clip(),Rule("down","2"),"unsupported_schema_version");
    run("future pivot",Asset(),Clip(),Rule("down","1","authored_v2"),"unsupported_pivot_or_layer_policy");
    run("future layers",Asset(),Clip(),Rule("down","1","alpha_bounds_v1","layered_v2"),"unsupported_pivot_or_layer_policy");
    run("duplicate contract",Asset(),Clip(),Rule()+Rule(),"duplicate_profile_state");
    run("duplicate direction",Asset(),Clip(),Rule("down|down"),"invalid_or_duplicate_directions");
    run("ambiguous fallback",Asset(),Clip(),Rule("any|down"),"invalid_or_duplicate_directions");
    run("unsupported direction",Asset(),Clip(),Rule("north_east"),"invalid_or_duplicate_directions");
    run("empty coverage",Asset(),Clip(),"","coverage_empty");
    run("bad clip range",Asset(),Clip("fixture",3,2),Rule(),"visual_parse:row_2:frame_bounds",false);
    run("partial parse not accepted",Asset(),Clip("fixture",3,2)+Clip(),Rule(),"visual_parse:",false);
    run("duplicate asset ID",Asset()+Asset(),Clip(),Rule(),"asset_parse:duplicate_id");
    run("valid again",Asset(),Clip(),Rule(),"");
    AnimationController controller;Check(controller.Select(visuals,"hero",VisualState::Walk,Facing::Down),"controller selection failed");
    controller.Update(0.2f,false);const auto* clipPointer=controller.Clip();const auto frame=controller.Frame();
    const auto missing=visuals.ValidateAgainst(assets,root/"does-not-exist.csv");
    Check(Contains(missing,"coverage_manifest_missing"),"missing coverage silently accepted");
    Check(controller.Clip()==clipPointer && controller.Frame()==frame,"validation changed controller/clip state");
    Check(visuals.ValidateAgainst(assets,coverageCsv).empty(),"validation retained old errors");
    Text(coverageCsv,"bad_header\n");Check(Contains(visuals.ValidateAgainst(assets,coverageCsv),"coverage_invalid_header"),"bad coverage header accepted");
    Text(coverageCsv,"\xEF\xBB\xBF"+CoverageHeader+Rule());
    Check(visuals.ValidateAgainst(assets,coverageCsv).empty(),"coverage UTF-8 BOM rejected");
    Text(visualsCsv,"\xEF\xBB\xBF"+VisualHeader+Clip());
    Check(visuals.Load(visualsCsv) && visuals.ValidateAgainst(assets,coverageCsv).empty(),"visual UTF-8 BOM rejected");
    std::cout<<"Cross-validation checks="<<checks<<"; failures="<<failures<<'\n';
    return failures?1:0;
}
