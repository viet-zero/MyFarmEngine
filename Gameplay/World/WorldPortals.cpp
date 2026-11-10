#include "WorldMap.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <set>
namespace MyGame
{
    bool WorldMap::LoadPortals(const std::filesystem::path& csv)
    {
        std::ifstream input(csv);std::string line;
        if(!input || !std::getline(input,line) || line!="id,collider_id,x,y,width,height,owner,door,pick_rise") return false;
        std::vector<WorldPortal> loaded;std::set<std::string> ids;
        while(std::getline(input,line))
        {
            for(char& c:line) if(c==',') c=' ';
            WorldPortal p;std::string extra;int door=0;
            std::istringstream row(line);
            if(!(row>>p.id>>p.colliderId>>p.position.x>>p.position.y>>p.size.x>>p.size.y>>p.ownerId>>door>>p.pickRise) || row>>extra ||
               p.id.empty() || p.id.size()>128 || loaded.size()>=64 || !ids.insert(p.id).second || p.ownerId==0 || door<0 || door>1 ||
               !std::isfinite(p.position.x) || !std::isfinite(p.position.y) || !std::isfinite(p.size.x) || !std::isfinite(p.size.y) ||
               !std::isfinite(p.pickRise) || p.pickRise<0 || p.pickRise>512 || p.position.x<0 || p.position.y<0 || p.size.x<=0 || p.size.y<=0 ||
               p.position.x+p.size.x>Size().x || p.position.y+p.size.y>Size().y) return false;
            p.door=door!=0;
            const auto matches=[&](const WorldObject& o){return o.gameplayId==p.colliderId && o.blocked &&
                p.position.x>=o.position.x && p.position.y>=o.position.y &&
                p.position.x+p.size.x<=o.position.x+o.size.x && p.position.y+p.size.y<=o.position.y+o.size.y;};
            if(std::none_of(m_Objects.begin(),m_Objects.end(),matches) && std::none_of(m_StaticColliders.begin(),m_StaticColliders.end(),matches)) return false;
            loaded.push_back(p);
        }
        if(loaded.empty()) return false;
        m_Portals=std::move(loaded);return true;
    }
    const WorldPortal* WorldMap::Portal(std::string_view id) const
    {
        const auto it=std::find_if(m_Portals.begin(),m_Portals.end(),[&](const auto& p){return p.id==id;});
        return it==m_Portals.end()?nullptr:&*it;
    }
    bool WorldMap::PortalPassage(const WorldObject& object,glm::vec2 low,glm::vec2 high) const
    {
        return std::any_of(m_Portals.begin(),m_Portals.end(),[&](const auto& p){
            return p.colliderId==object.gameplayId && p.Passable() &&
                low.x>=p.position.x+(p.door?0.0f:14.0f) && high.x<=p.position.x+p.size.x-(p.door?0.0f:14.0f);
        });
    }
    bool WorldMap::SetPortalOpen(std::string_view id,bool open,std::span<const DynamicCollider> occupants,bool snap)
    {
        const auto it=std::find_if(m_Portals.begin(),m_Portals.end(),[&](const auto& p){return p.id==id;});
        if(it==m_Portals.end()) return false;
        if(!open)
            for(const auto& body:occupants)
                if(!std::isfinite(body.center.x) || !std::isfinite(body.center.y) || !std::isfinite(body.collider.halfSize.x) ||
                   !std::isfinite(body.collider.halfSize.y) || body.collider.halfSize.x<=0 || body.collider.halfSize.y<=0 ||
                   Intersects(body.center-body.collider.halfSize,body.collider.halfSize*2.0f,it->position,it->size)) return false;
        it->open=open;if(snap) it->progress=open?1.0f:0.0f;return true;
    }
    void WorldMap::ResetPortals(){for(auto& p:m_Portals){p.open=false;p.progress=0;}}
    void WorldMap::TickPortals(float dt)
    {
        if(!std::isfinite(dt) || dt<=0) return;
        for(auto& p:m_Portals) p.progress=std::clamp(p.progress+(p.open?1.0f:-1.0f)*std::min(dt,0.1f)*4.0f,0.0f,1.0f);
    }
}
