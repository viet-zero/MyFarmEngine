#include "RenderOrder.h"
#include <algorithm>
#include <cmath>

namespace MyGame
{
    void RenderOrder::Begin(glm::vec2 viewOrigin,glm::vec2 viewSize)
    {
        m_ViewOrigin=viewOrigin;m_ViewSize=viewSize;m_Entries.clear();
    }
    bool RenderOrder::Submit(RenderKind kind,std::size_t index,glm::vec2 origin,glm::vec2 size,float footY)
    {
        if(!std::isfinite(origin.x)||!std::isfinite(origin.y)||!std::isfinite(size.x)||
           !std::isfinite(size.y)||!std::isfinite(footY)||size.x<=0||size.y<=0||
           !std::isfinite(m_ViewOrigin.x)||!std::isfinite(m_ViewOrigin.y)||
           !std::isfinite(m_ViewSize.x)||!std::isfinite(m_ViewSize.y)||
           m_ViewSize.x<=0||m_ViewSize.y<=0) return false;
        if(origin.x>=m_ViewOrigin.x+m_ViewSize.x || m_ViewOrigin.x>=origin.x+size.x ||
           origin.y>=m_ViewOrigin.y+m_ViewSize.y || m_ViewOrigin.y>=origin.y+size.y) return false;
        m_Entries.push_back({kind,index,footY,m_Entries.size()});return true;
    }
    void RenderOrder::Sort()
    {
        std::sort(m_Entries.begin(),m_Entries.end(),[](const auto& a,const auto& b){
            return a.footY<b.footY || (a.footY==b.footY && a.sequence<b.sequence);
        });
    }
}
