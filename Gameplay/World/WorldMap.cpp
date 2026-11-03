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
            const Zone zone=x<TownEndX?Zone::Town:(x<FarmEndX?Zone::Farm:Zone::LakeForest);
            Terrain terrain=zone==Zone::Farm?Terrain::Soil:Terrain::Grass;
            if(zone==Zone::Town && (x%12==0 || y==31)) terrain=Terrain::Path;
            if(zone==Zone::Farm && (x%8==0 || y%9==0)) terrain=Terrain::Path;
            if(zone==Zone::LakeForest)
            {
                const int dx=x-LakeCenterX,dy=y-LakeCenterY;
                if((dx*dx*LakeRadiusY*LakeRadiusY+dy*dy*LakeRadiusX*LakeRadiusX)<
                   LakeRadiusX*LakeRadiusX*LakeRadiusY*LakeRadiusY) terrain=Terrain::Water;
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
    bool WorldMap::LoadStaticColliders(const std::filesystem::path& csv)
    {
        std::ifstream file(csv);std::string line;
        if(!file || !std::getline(file,line) || line!="id,x,y,width,height") return false;
        std::vector<WorldObject> loaded;std::set<std::string> ids;
        while(std::getline(file,line))
        {
            if(!line.empty() && line.back()=='\r') line.pop_back();
            if(line.empty()) continue;
            std::array<std::string,5> fields;std::stringstream stream(line);std::string part;
            for(auto& field:fields) if(!std::getline(stream,field,',')) return false;
            if(std::getline(stream,part,',') || fields[0].empty() || !ids.insert(fields[0]).second) return false;
            std::array<float,4> numbers{};
            for(int i=0;i<4;++i)
            {
                const auto& value=fields[static_cast<std::size_t>(i+1)];
                const auto result=std::from_chars(value.data(),value.data()+value.size(),numbers[static_cast<std::size_t>(i)]);
                if(result.ec!=std::errc{} || result.ptr!=value.data()+value.size() || !std::isfinite(numbers[static_cast<std::size_t>(i)])) return false;
            }
            if(numbers[0]<0 || numbers[1]<0 || numbers[2]<=0 || numbers[3]<=0 ||
               numbers[0]+numbers[2]>Size().x || numbers[1]+numbers[3]>Size().y) return false;
            loaded.push_back({fields[0],fields[0],{numbers[0],numbers[1]},
                              {numbers[2],numbers[3]},true,true});
        }
        if(loaded.empty()) return false;
        m_StaticColliders=std::move(loaded);return true;
    }
    bool WorldMap::SetObjectActive(std::string_view gameplayId,bool active)
    {
        for(auto& object:m_Objects)
            if(object.gameplayId==gameplayId){object.active=active;return true;}
        return false;
    }
    const WorldCell* WorldMap::TryCell(int x,int y) const
    {
        if(x<0 || y<0 || x>=Width || y>=Height) return nullptr;
        return &m_Cells[static_cast<std::size_t>(y*Width+x)];
    }
    bool WorldMap::CanMoveTo(glm::vec2 point) const
    {
        return CanOccupy(point,{{12,12}});
    }
    bool WorldMap::CanOccupy(glm::vec2 center,ColliderComponent collider) const
    {
        const auto h=collider.halfSize;
        if(!std::isfinite(center.x)||!std::isfinite(center.y)||!std::isfinite(h.x)||!std::isfinite(h.y)||
           h.x<=0 || h.y<=0 || center.x-h.x<0 || center.y-h.y<0 ||
           center.x+h.x>Size().x || center.y+h.y>Size().y) return false;
        const auto low=center-h,high=center+h;
        const int left=static_cast<int>(std::floor(low.x/TileSize));
        const int top=static_cast<int>(std::floor(low.y/TileSize));
        const int right=static_cast<int>(std::floor(std::nextafter(high.x,-INFINITY)/TileSize));
        const int bottom=static_cast<int>(std::floor(std::nextafter(high.y,-INFINITY)/TileSize));
        for(int y=top;y<=bottom;++y) for(int x=left;x<=right;++x)
        {
            const auto* cell=TryCell(x,y);
            if(!cell || cell->blocked) return false;
        }
        const auto intersects=[&](const WorldObject& object){
            return object.active && object.blocked && Intersects(low,h*2.0f,object.position,object.size);
        };
        return std::none_of(m_Objects.begin(),m_Objects.end(),intersects) &&
               std::none_of(m_StaticColliders.begin(),m_StaticColliders.end(),intersects);
    }
    bool WorldMap::HasLineOfSight(glm::vec2 from,glm::vec2 to) const
    {
        if(!std::isfinite(from.x)||!std::isfinite(from.y)||!std::isfinite(to.x)||!std::isfinite(to.y)) return false;
        const float distance=glm::length(to-from);
        if(!std::isfinite(distance)) return false;
        const int steps=std::max(1,static_cast<int>(std::ceil(distance/8.0f)));
        for(int i=0;i<=steps;++i)
        {
            const glm::vec2 point=from+(to-from)*(static_cast<float>(i)/static_cast<float>(steps));
            const auto* cell=TryCell(static_cast<int>(std::floor(point.x/TileSize)),
                                     static_cast<int>(std::floor(point.y/TileSize)));
            if(!cell || cell->blocked) return false;
            for(const auto& object:m_StaticColliders)
                if(Intersects(point-glm::vec2{1,1},glm::vec2{2,2},object.position,object.size)) return false;
        }
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
    TileRange WorldMap::VisibleChunks(glm::vec2 center,glm::vec2 viewSize) const
    {
        const auto tiles=VisibleTiles(center,viewSize);
        return {tiles.left/ChunkTiles,tiles.top/ChunkTiles,
                (tiles.right+ChunkTiles-1)/ChunkTiles,(tiles.bottom+ChunkTiles-1)/ChunkTiles};
    }
    WorldChunk WorldMap::ChunkAt(int x,int y) const
    {
        return {x,y,{x*ChunkTiles,y*ChunkTiles,
                     std::min((x+1)*ChunkTiles,Width),std::min((y+1)*ChunkTiles,Height)}};
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
        case Zone::Town:return {1550,1950};
        case Zone::Farm:return {2200,1950};
        case Zone::LakeForest:return {4350,2150};
        }
        return {2200,1950};
    }
    bool WorldMap::Intersects(glm::vec2 a,glm::vec2 aSize,glm::vec2 b,glm::vec2 bSize)
    {return a.x<b.x+bSize.x && b.x<a.x+aSize.x && a.y<b.y+bSize.y && b.y<a.y+aSize.y;}
}
