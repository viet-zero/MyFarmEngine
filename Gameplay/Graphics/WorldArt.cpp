#include "WorldArt.h"
#include <cmath>
#include <fstream>
#include <sstream>

namespace MyGame
{
    unsigned int CanonicalTerrainMask(unsigned int mask)
    {
        mask&=255;
        if((mask&5)!=5) mask&=~2u;
        if((mask&20)!=20) mask&=~8u;
        if((mask&80)!=80) mask&=~32u;
        if((mask&65)!=65) mask&=~128u;
        return mask;
    }
    int ArtDirection(glm::vec2 movement,int previous)
    {
        if(!std::isfinite(movement.x) || !std::isfinite(movement.y) ||
           glm::dot(movement,movement)<0.000001f) return previous;
        constexpr double pi=3.14159265358979323846;
        const double angle=std::atan2(static_cast<double>(movement.y),static_cast<double>(movement.x));
        return (static_cast<int>(std::floor((angle+pi/8)/(pi/4)))+8)%8;
    }
    void ArtAtlas::Reset()
    {
        m_Frames.clear();
        m_Texture.Reset();
    }
    const ArtFrame* ArtAtlas::Find(std::string_view id) const
    {
        const auto found=m_Frames.find(id);
        return found==m_Frames.end()?nullptr:&found->second;
    }
    bool ArtAtlas::Load(const std::filesystem::path& base)
    {
        Reset();
        auto csv=base;csv+=".csv";
        auto png=base;png+=".png";
        std::ifstream input(csv);
        std::string line;
        if(!input || !std::getline(input,line) || !m_Texture.Load(png)) return false;
        if(!line.empty() && line.back()=='\r') line.pop_back();
        if(line!="id,x,y,w,h,left,top,cw,ch,reference,px,py") {Reset();return false;}
        while(std::getline(input,line))
        {
            for(char& c:line) if(c==',') c=' ';
            std::istringstream row(line);
            std::string id;
            int x=0,y=0;
            ArtFrame frame;
            auto& b=frame.bounds;
            if(!(row>>id>>x>>y>>b.frameWidth>>b.frameHeight>>b.left>>b.top>>b.width>>b.height>>
                 b.referenceHeight>>frame.pivot.x>>frame.pivot.y) ||
               x<0 || y<0 || b.frameWidth<=0 || b.frameHeight<=0 ||
               b.frameWidth>m_Texture.Width() || b.frameHeight>m_Texture.Height() ||
               x>m_Texture.Width()-b.frameWidth || y>m_Texture.Height()-b.frameHeight ||
               b.left<0 || b.top<0 || b.width<=0 || b.height<=0 || b.referenceHeight<=0 ||
               b.width>b.frameWidth || b.height>b.frameHeight ||
               b.left>b.frameWidth-b.width || b.top>b.frameHeight-b.height ||
               !std::isfinite(frame.pivot.x) || !std::isfinite(frame.pivot.y) ||
               frame.pivot.x<0 || frame.pivot.y<0 || frame.pivot.x>b.frameWidth || frame.pivot.y>b.frameHeight)
            {Reset();return false;}
            frame.uv={static_cast<float>(x)/m_Texture.Width(),static_cast<float>(y)/m_Texture.Height(),
                      static_cast<float>(b.frameWidth)/m_Texture.Width(),static_cast<float>(b.frameHeight)/m_Texture.Height()};
            if(!m_Frames.emplace(id,frame).second){Reset();return false;}
        }
        return !m_Frames.empty();
    }
    void ArtAtlas::Draw(MyEngine::Canvas2D& canvas,std::string_view id,glm::vec2 origin,glm::vec2 size) const
    {
        if(const auto* frame=Find(id))
            canvas.Sprite(m_Texture,origin.x,origin.y,size.x,size.y,frame->uv);
    }
    void WorldArt::Reset()
    {
        for(auto& atlas:m_Terrain) atlas.Reset();
        m_Sprites.Reset();
        m_Ground.Reset();m_Props.Reset();m_CozyReady=false;
        m_Ready=false;
    }
    bool WorldArt::Load(const std::filesystem::path& root)
    {
        Reset();
        if(!std::filesystem::is_regular_file(root/"sprites.csv")) return false;
        if(!m_Sprites.Load(root/"sprites")){Reset();return false;}
        for(int i=0;i<4;++i)
        {
            if(!m_Terrain[static_cast<std::size_t>(i)].Load(root/("terrain"+std::to_string(i))))
            {Reset();return false;}
            const auto& terrain=m_Terrain[static_cast<std::size_t>(i)];
            for(unsigned int mask=0;mask<256;++mask)
            {
                if(CanonicalTerrainMask(mask)!=mask) continue;
                const auto suffix=(mask<10?"00":mask<100?"0":"")+std::to_string(mask);
                if(!terrain.Find("land_"+suffix) || !terrain.Find("water_"+suffix)){Reset();return false;}
            }
            for(const char* key:{"grass","path","bed_kho_dau","bed_kho_giua","bed_kho_cuoi","bed_kho_don",
                                "bed_uot_dau","bed_uot_giua","bed_uot_cuoi","bed_uot_don"})
                if(!terrain.Find(key)){Reset();return false;}
        }
        for(int direction=0;direction<8;++direction)
            for(int frame=0;frame<3;++frame)
                if(!Actor(direction,frame>0,frame-1)){Reset();return false;}
        m_Ready=true;
        return true;
    }
    bool VisualScale::Load(const std::filesystem::path& path)
    {
        std::ifstream input(path);
        std::string line;
        if(!input || !std::getline(input,line)) return false;
        if(line.starts_with("\xEF\xBB\xBF")) line.erase(0,3);
        if(line!="key,value") return false;
        auto candidate=*this;
        std::map<std::string,float*> values{{"human",&candidate.human},{"tree",&candidate.tree},
            {"house",&candidate.house},{"gate",&candidate.gate},{"fence",&candidate.fence}};
        std::map<std::string,bool> seen;
        while(std::getline(input,line))
        {
            for(char& c:line) if(c==',') c=' ';
            std::istringstream row(line);std::string key,extra;float value=0;
            if(!(row>>key>>value) || row>>extra || !std::isfinite(value) || value<16 || value>1024 ||
               !values.contains(key) || seen[key]) return false;
            seen[key]=true;*values.at(key)=value;
        }
        if(seen.size()!=values.size()) return false;
        *this=candidate;return true;
    }
    bool WorldArt::LoadCozy(const std::filesystem::path& root)
    {
        m_CozyReady=false;
        if(!m_Ground.Load(root/"ground") || !m_Props.Load(root/"props") ||
           !m_Ground.Find("grass") || !m_Ground.Find("water") || !m_Ground.Find("meadow") ||
           !m_Props.Find("tree") || !m_Props.Find("house"))
        {m_Ground.Reset();m_Props.Reset();return false;}
        m_CozyReady=true;return true;
    }
    const ArtFrame* WorldArt::Actor(int direction,bool walking,int frame) const
    {
        constexpr std::array names{"e","se","s","sw","w","nw","n","ne"};
        if(direction<0 || direction>=8) return nullptr;
        const auto id="actor_"+std::string(names[static_cast<std::size_t>(direction)])+
                      (walking?"_walk_"+std::to_string(frame%2):"_idle");
        return m_Sprites.Find(id);
    }
}
