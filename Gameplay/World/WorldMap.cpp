#include "WorldMap.h"
#include <algorithm>
#include <cmath>
#include <charconv>
#include <fstream>
#include <set>
#include <sstream>

namespace MyGame
{
    WorldMap::WorldMap()
    {
        for(int y=0;y<Height;++y) for(int x=0;x<Width;++x)
        {
            const Zone zone=x<32?Zone::Town:(x<64?Zone::Farm:Zone::LakeForest);
            Terrain terrain=zone==Zone::Farm?Terrain::Soil:Terrain::Grass;
            if(zone==Zone::Town && (x%12==0 || y==31)) terrain=Terrain::Path;
            if(zone==Zone::Farm && (x%8==0 || y%9==0)) terrain=Terrain::Path;
            if(zone==Zone::LakeForest)
            {
                const int dx=x-79,dy=y-31;
                if(dx*dx+dy*dy<120) terrain=Terrain::Water;
                else if((x*17+y*13)%7<3) terrain=Terrain::Forest;
            }
            m_Cells[static_cast<std::size_t>(y*Width+x)]={zone,terrain,terrain==Terrain::Water};
        }
    }
    bool WorldMap::LoadObjects(const std::filesystem::path& csv)
    {
        std::ifstream file(csv);std::string line;
        if(!file || !std::getline(file,line)) return false;
        if(!line.empty() && line.back()=='\r') line.pop_back();
        if(line!="gameplay_id,visual_id,x,y,width,height,blocked") return false;
        std::vector<WorldObject> loaded;std::set<std::string> ids;
        while(std::getline(file,line))
        {
            if(!line.empty() && line.back()=='\r') line.pop_back();
            if(line.empty()) continue;
            std::array<std::string,7> fields;std::stringstream stream(line);std::string part;
            for(auto& field:fields) if(!std::getline(stream,field,',')) return false;
            if(std::getline(stream,part,',')) return false;
            if(fields[0].empty() || fields[1].empty() || fields[0]==fields[1] || !ids.insert(fields[0]).second) return false;
            std::array<float,4> numbers{};
            for(int i=0;i<4;++i)
            {
                const auto& value=fields[static_cast<std::size_t>(i+2)];
                const auto result=std::from_chars(value.data(),value.data()+value.size(),numbers[static_cast<std::size_t>(i)]);
                if(result.ec!=std::errc{} || result.ptr!=value.data()+value.size() || !std::isfinite(numbers[static_cast<std::size_t>(i)])) return false;
            }
            if(numbers[2]<=0 || numbers[3]<=0 || numbers[0]<0 || numbers[1]<0 ||
               numbers[0]+numbers[2]>Size().x || numbers[1]+numbers[3]>Size().y ||
               (fields[6]!="0" && fields[6]!="1")) return false;
            loaded.push_back({fields[0],fields[1],{numbers[0],numbers[1]},
                              {numbers[2],numbers[3]},fields[6]=="1"});
        }
        if(loaded.empty()) return false;
        m_Objects=std::move(loaded);return true;
    }
    const WorldCell* WorldMap::TryCell(int x,int y) const
    {
        if(x<0 || y<0 || x>=Width || y>=Height) return nullptr;
        return &m_Cells[static_cast<std::size_t>(y*Width+x)];
    }
    bool WorldMap::CanMoveTo(glm::vec2 point) const
    {
        if(!std::isfinite(point.x)||!std::isfinite(point.y)) return false;
        const auto* cell=TryCell(static_cast<int>(std::floor(point.x/TileSize)),
                                 static_cast<int>(std::floor(point.y/TileSize)));
        if(!cell || cell->blocked) return false;
        for(const auto& object:m_Objects)
            if(object.blocked && Intersects(point-glm::vec2{12,12},{24,24},object.position,object.size)) return false;
        return true;
    }
    TileRange WorldMap::VisibleTiles(glm::vec2 center,glm::vec2 viewSize,int padding) const
    {
        const auto low=center-viewSize*0.5f,high=center+viewSize*0.5f;
        const int extra=std::clamp(padding,0,ChunkTiles);
        return {std::clamp(static_cast<int>(std::floor(low.x/TileSize))-extra,0,Width),
                std::clamp(static_cast<int>(std::floor(low.y/TileSize))-extra,0,Height),
                std::clamp(static_cast<int>(std::ceil(high.x/TileSize))+extra,0,Width),
                std::clamp(static_cast<int>(std::ceil(high.y/TileSize))+extra,0,Height)};
    }
    Zone WorldMap::ZoneAt(glm::vec2 point) const
    {
        const auto* cell=TryCell(static_cast<int>(std::floor(point.x/TileSize)),
                                 static_cast<int>(std::floor(point.y/TileSize)));
        return cell?cell->zone:Zone::Farm;
    }
    glm::vec2 WorldMap::Spawn(Zone zone) const
    {
        switch(zone)
        {
        case Zone::Town:return {1024,2020};
        case Zone::Farm:return {3000,2020};
        case Zone::LakeForest:return {4350,1950};
        }
        return {3000,2020};
    }
    bool WorldMap::Intersects(glm::vec2 a,glm::vec2 aSize,glm::vec2 b,glm::vec2 bSize)
    {return a.x<b.x+bSize.x && b.x<a.x+aSize.x && a.y<b.y+bSize.y && b.y<a.y+aSize.y;}
}
