#include "ForegroundTrees.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <fstream>
#include <set>
#include <sstream>

namespace MyGame
{
    bool ForegroundTrees::Load(const std::filesystem::path& csv,const WorldMap& world)
    {
        std::ifstream file(csv);std::string line;
        if(!file || !std::getline(file,line)) return false;
        if(!line.empty() && line.back()=='\r') line.pop_back();
        if(line!="gameplay_id,canopy_x,canopy_y,width,height,foot_y,interact_x,interact_y,fruits") return false;
        std::vector<ForegroundTree> loaded;std::set<std::string> ids;
        while(std::getline(file,line))
        {
            if(!line.empty() && line.back()=='\r') line.pop_back();
            if(line.empty()) continue;
            std::array<std::string,9> fields;std::stringstream stream(line);std::string extra;
            for(auto& field:fields) if(!std::getline(stream,field,',')) return false;
            if(std::getline(stream,extra,',') || fields[0].empty() || !ids.insert(fields[0]).second) return false;
            std::array<float,7> n{};
            for(int i=0;i<7;++i)
            {
                const auto& value=fields[static_cast<std::size_t>(i+1)];
                const auto result=std::from_chars(value.data(),value.data()+value.size(),n[static_cast<std::size_t>(i)]);
                if(result.ec!=std::errc{} || result.ptr!=value.data()+value.size() || !std::isfinite(n[static_cast<std::size_t>(i)])) return false;
            }
            int fruits=0;
            const auto& count=fields[8];const auto parsed=std::from_chars(count.data(),count.data()+count.size(),fruits);
            if(parsed.ec!=std::errc{} || parsed.ptr!=count.data()+count.size() || fruits<=0 || fruits>100 ||
               n[0]<0 || n[1]<0 || n[2]<=0 || n[3]<=0 || n[0]+n[2]>WorldMap::Size().x ||
               n[1]+n[3]>WorldMap::Size().y || n[4]<n[1] || n[4]>WorldMap::Size().y ||
               n[5]<0 || n[5]>=WorldMap::Size().x || n[6]<0 || n[6]>=WorldMap::Size().y) return false;
            const auto found=std::find_if(world.Objects().begin(),world.Objects().end(),[&](const auto& object){
                return object.gameplayId==fields[0] && object.blocked;
            });
            if(found==world.Objects().end()) return false;
            loaded.push_back({fields[0],{n[0],n[1]},{n[2],n[3]},n[4],{n[5],n[6]},fruits});
        }
        if(loaded.empty()) return false;
        m_Trees=std::move(loaded);return true;
    }
    bool ForegroundTrees::Contains(std::string_view gameplayId) const
    {
        return std::any_of(m_Trees.begin(),m_Trees.end(),[&](const auto& tree){return tree.gameplayId==gameplayId;});
    }
}
