#include "VisualCatalog.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <sstream>
#include <set>

namespace
{
    std::optional<MyGame::VisualState> State(std::string_view value)
    {
        using MyGame::VisualState;
        if(value=="idle") return VisualState::Idle;if(value=="walk") return VisualState::Walk;
        if(value=="run") return VisualState::Run;if(value=="action") return VisualState::Action;
        if(value=="hurt") return VisualState::Hurt;if(value=="sleep") return VisualState::Sleep;
        return std::nullopt;
    }
    std::optional<MyGame::Facing> Direction(std::string_view value)
    {
        using MyGame::Facing;
        if(value=="any") return Facing::Any;if(value=="down") return Facing::Down;
        if(value=="left") return Facing::Left;if(value=="right") return Facing::Right;
        if(value=="up") return Facing::Up;return std::nullopt;
    }
    template<class T> bool Number(std::string_view text,T& value)
    {
        const auto result=std::from_chars(text.data(),text.data()+text.size(),value);
        return result.ec==std::errc{} && result.ptr==text.data()+text.size();
    }
    std::vector<std::string> Split(const std::string& line,char separator=',')
    {
        std::vector<std::string> values;std::stringstream stream(line);std::string value;
        while(std::getline(stream,value,separator)) values.push_back(value);
        if(!line.empty() && line.back()==separator) values.emplace_back();
        return values;
    }
    std::string_view AssetKind(std::string_view kind)
    {
        if(kind=="player" || kind=="npc") return "character";
        if(kind=="dog") return "animal";
        if(kind=="crop") return "crop";
        return {};
    }
    std::string StateName(MyGame::VisualState state)
    {
        constexpr const char* names[]={"idle","walk","run","action","hurt","sleep"};
        return names[static_cast<std::size_t>(state)];
    }
    std::string FacingName(MyGame::Facing facing)
    {
        constexpr const char* names[]={"any","down","left","right","up"};
        return names[static_cast<std::size_t>(facing)];
    }
    std::string Key(std::string_view profile,MyGame::VisualState state,MyGame::Facing facing)
    {return std::string(profile)+'|'+StateName(state)+'|'+FacingName(facing);}
}

namespace MyGame
{
    bool VisualCatalog::Load(const std::filesystem::path& csv)
    {
        m_Clips.clear();m_Errors.clear();std::ifstream file(csv);
        if(!file){m_Errors.emplace_back("manifest_missing");return false;}
        std::string line;std::getline(file,line);
        constexpr std::string_view oldHeader="profile,kind,state,direction,asset_id,first_frame,frame_count,fps,loop,total_frames";
        constexpr std::string_view header="profile,kind,state,direction,asset_id,first_frame,frame_count,fps,loop,total_frames,flip_x";
        if(line.starts_with("\xEF\xBB\xBF")) line.erase(0,3);
        if(!line.empty() && line.back()=='\r') line.pop_back();
        const bool hasFlip=line==header;
        if(!hasFlip && line!=oldHeader){m_Errors.emplace_back("invalid_header");return false;}
        int row=1;
        while(std::getline(file,line))
        {
            ++row;if(!line.empty() && line.back()=='\r') line.pop_back();if(line.empty()) continue;
            const auto fields=Split(line);
            if(fields.size()!=(hasFlip?11u:10u)){m_Errors.push_back("row_"+std::to_string(row)+":field_count");continue;}
            const auto state=State(fields[2]);const auto facing=Direction(fields[3]);
            int first=0,count=0,total=0;float fps=0;int loop=0,flip=0;
            if(fields[0].empty() || !state || !facing || !Number(fields[5],first) || !Number(fields[6],count) ||
               !Number(fields[7],fps) || !Number(fields[8],loop) || !Number(fields[9],total) || (loop!=0 && loop!=1) ||
               (hasFlip && (!Number(fields[10],flip) || (flip!=0 && flip!=1))))
            {m_Errors.push_back("row_"+std::to_string(row)+":invalid_value");continue;}
            VisualClip clip;
            clip.assetId=fields[4];clip.timeline={first,count,fps,loop==1};
            clip.facing=*facing;clip.availableFrames=total;clip.flipX=flip==1;
            clip.profile=fields[0];clip.kind=fields[1];clip.state=*state;clip.sourceRow=row;
            if(!clip.timeline.IsValid(total))
            {m_Errors.push_back("row_"+std::to_string(row)+":frame_bounds");continue;}
            const auto key=Key(fields[0],*state,*facing);
            if(!m_Clips.emplace(key,std::move(clip)).second)
                m_Errors.push_back("row_"+std::to_string(row)+":duplicate_clip");
        }
        if(file.bad()) m_Errors.emplace_back("manifest_read_failed");
        return m_Errors.empty() && !m_Clips.empty();
    }

    const VisualClip* VisualCatalog::Find(std::string_view profile,VisualState state,Facing facing) const
    {
        if(const auto it=m_Clips.find(Key(profile,state,facing));it!=m_Clips.end()) return &it->second;
        if(const auto it=m_Clips.find(Key(profile,state,Facing::Any));it!=m_Clips.end()) return &it->second;
        return nullptr;
    }
    std::vector<std::string> VisualCatalog::AssetIds() const
    {
        std::set<std::string> ids;
        for(const auto& [key,clip]:m_Clips)
        {
            (void)key;
            if(!clip.assetId.empty()) ids.insert(clip.assetId);
        }
        return {ids.begin(),ids.end()};
    }

    std::vector<std::string> VisualCatalog::ValidateAgainst(const AssetManifest& assets,
                                                          const std::filesystem::path& coverageCsv) const
    {
        std::vector<std::string> errors;
        for(const auto& error:m_Errors) errors.push_back("visual_parse:"+error);
        for(const auto& error:assets.Errors()) errors.push_back("asset_parse:"+error);
        if(m_Clips.empty()) errors.emplace_back("visual_catalog_empty");
        struct Rule {std::string kind;std::set<Facing> directions;int row=0;};
        using RuleKey=std::pair<std::string,VisualState>;
        std::map<RuleKey,Rule> rules;
        std::map<std::string,std::string> profileKinds;
        std::ifstream file(coverageCsv);
        if(!file){errors.emplace_back("coverage_manifest_missing");return errors;}
        std::string line;std::getline(file,line);
        if(line.starts_with("\xEF\xBB\xBF")) line.erase(0,3);
        if(!line.empty() && line.back()=='\r') line.pop_back();
        if(line!="schema_version,profile,kind,state,directions,pivot_policy,layer_policy")
        {errors.emplace_back("coverage_invalid_header");return errors;}
        int row=1;
        while(std::getline(file,line))
        {
            ++row;if(!line.empty() && line.back()=='\r') line.pop_back();if(line.empty()) continue;
            const auto fields=Split(line);
            const auto diagnostic=[&](std::string reason)
            {
                errors.push_back("coverage_row_"+std::to_string(row)+":"+
                    (fields.size()>1?fields[1]:"?")+":"+reason);
            };
            if(fields.size()!=7){diagnostic("field_count");continue;}
            if(fields[0]!="1"){diagnostic("unsupported_schema_version");continue;}
            const auto state=State(fields[3]);
            if(fields[1].empty() || AssetKind(fields[2]).empty() || !state)
            {diagnostic("invalid_profile_kind_or_state");continue;}
            if(fields[5]!="alpha_bounds_v1" || fields[6]!="single_sprite_v1")
            {diagnostic("unsupported_pivot_or_layer_policy");continue;}
            Rule rule;rule.kind=fields[2];rule.row=row;
            bool valid=true;
            for(const auto& value:Split(fields[4],'|'))
            {
                const auto facing=Direction(value);
                if(!facing || !rule.directions.insert(*facing).second) valid=false;
            }
            if(!valid || rule.directions.empty() || (rule.directions.contains(Facing::Any) && rule.directions.size()>1))
            {diagnostic("invalid_or_duplicate_directions");continue;}
            if(const auto it=profileKinds.find(fields[1]);it!=profileKinds.end() && it->second!=rule.kind)
            {diagnostic("profile_kind_conflict");continue;}
            profileKinds[fields[1]]=rule.kind;
            if(!rules.emplace(RuleKey{fields[1],*state},std::move(rule)).second) diagnostic("duplicate_profile_state");
        }
        if(file.bad()) errors.emplace_back("coverage_read_failed");
        if(rules.empty()) errors.emplace_back("coverage_empty");
        // Decode/validate each referenced asset once, even when many directions share an atlas.
        std::map<std::string,std::string> invalidAssets;
        for(const auto& [key,clip]:m_Clips)
        {
            const auto diagnostic=[&](std::string reason)
            {errors.push_back("visual_row_"+std::to_string(clip.sourceRow)+":"+key+":asset="+clip.assetId+":"+reason);};
            const auto rule=rules.find({clip.profile,clip.state});
            if(rule==rules.end()) diagnostic("coverage_not_declared");
            else
            {
                if(rule->second.kind!=clip.kind) diagnostic("profile_kind_mismatch");
                if(!rule->second.directions.contains(clip.facing)) diagnostic("direction_not_declared");
            }
            const auto* asset=assets.Find(clip.assetId);
            if(!asset){diagnostic("unknown_asset");continue;}
            if(asset->status!="ready"){diagnostic("asset_not_ready");continue;}
            if(AssetKind(clip.kind).empty() || AssetKind(clip.kind)!=asset->kind) diagnostic("asset_kind_mismatch");
            auto cached=invalidAssets.find(clip.assetId);
            if(cached==invalidAssets.end())
            {
                const auto checked=assets.Validate(*asset);
                cached=invalidAssets.emplace(clip.assetId,checked.valid?"":checked.reason).first;
            }
            if(!cached->second.empty()){diagnostic("asset_invalid:"+cached->second);continue;}
            const long long grid=static_cast<long long>(asset->columns)*asset->rows;
            if(clip.availableFrames!=grid) diagnostic("total_frames_mismatch");
            const long long end=static_cast<long long>(clip.timeline.firstFrame)+clip.timeline.frameCount;
            if(clip.timeline.firstFrame<asset->firstFrame || end>static_cast<long long>(asset->firstFrame)+asset->frameCount)
                diagnostic("clip_outside_asset_window");
        }
        for(const auto& [key,rule]:rules)
            for(const auto facing:rule.directions)
                // Do not let Any fallback pretend to provide directional artwork.
                if(!m_Clips.contains(Key(key.first,key.second,facing)))
                    errors.push_back("coverage_row_"+std::to_string(rule.row)+":"+key.first+"|"+StateName(key.second)+
                                     ":missing_direction:"+FacingName(facing));
        return errors;
    }

    bool AnimationController::Select(const VisualCatalog& catalog,std::string_view profile,VisualState state,Facing facing,
                                     bool restart)
    {
        const auto* clip=catalog.Find(profile,state,facing);if(!clip) return false;
        const auto resolved=clip->facing;
        const auto key=Key(profile,state,resolved);
        if(restart) m_Animator.Reset();
        if(!m_Animator.Play(key,clip->timeline,clip->availableFrames)) return false;
        m_Clip=clip;m_Profile=profile;m_State=state;m_Facing=facing;return true;
    }
    void AnimationController::Update(float deltaTime,bool paused)
    {m_Animator.SetPaused(paused);m_Animator.Update(deltaTime);}
    VisualAnimationSnapshot AnimationController::Save() const
    {return {m_Profile,m_State,m_Facing,m_Animator.Save()};}
    bool AnimationController::Rebuild(const VisualCatalog& catalog,const VisualAnimationSnapshot& snapshot)
    {
        const auto* clip=catalog.Find(snapshot.profile,snapshot.state,snapshot.facing);if(!clip) return false;
        auto timeline=snapshot.timeline;timeline.clipKey=Key(snapshot.profile,snapshot.state,clip->facing);
        if(!m_Animator.Restore(timeline,clip->timeline,clip->availableFrames)) return false;
        m_Clip=clip;m_Profile=snapshot.profile;m_State=snapshot.state;m_Facing=snapshot.facing;return true;
    }
    Facing FacingFromVelocity(float x,float y,Facing previous)
    {
        if(!std::isfinite(x) || !std::isfinite(y) || (x==0.0f && y==0.0f)) return previous;
        if(std::abs(x)>std::abs(y)) return x<0?Facing::Left:Facing::Right;
        return y<0?Facing::Up:Facing::Down;
    }
    VisualState ResolvePlayerState(bool moving,bool running,bool actionTriggered,bool hurtTriggered,
                                   VisualState current,bool currentFinished)
    {
        if(hurtTriggered) return VisualState::Hurt;
        if(actionTriggered) return VisualState::Action;
        if((current==VisualState::Action || current==VisualState::Hurt) && !currentFinished) return current;
        return moving?(running?VisualState::Run:VisualState::Walk):VisualState::Idle;
    }
    int GrowthFrame(double age,const std::vector<double>& stageDurations)
    {
        if(stageDurations.empty()) return 0;
        if(!std::isfinite(age) || age<=0.0) return 0;
        double boundary=0.0;
        for(std::size_t i=0;i<stageDurations.size();++i)
        {
            const double duration=stageDurations[i];
            if(!std::isfinite(duration) || duration<=0.0) return 0;
            boundary+=duration;if(age<boundary) return static_cast<int>(i);
        }
        return static_cast<int>(stageDurations.size()-1);
    }
}
