#include "WorldDistribution.h"
#include <algorithm>
#include <cmath>
#include <random>
namespace MyGame
{
    std::uint32_t VisualHash(int x,int y,std::uint32_t seed)
    {
        auto h=static_cast<std::uint32_t>(x)*73856093u ^ static_cast<std::uint32_t>(y)*19349663u ^ seed;
        h^=h>>16;h*=0x7feb352du;h^=h>>15;h*=0x846ca68bu;return h^(h>>16);
    }
    std::vector<glm::vec2> PoissonDiskPoints(glm::vec2 extent,float radius,std::uint32_t seed)
    {
        if(!std::isfinite(extent.x)||!std::isfinite(extent.y)||!std::isfinite(radius)||
           extent.x<=0||extent.y<=0||extent.x>10000||extent.y>10000||radius<24) return {};
        const float cell=radius/std::sqrt(2.0f);
        const int width=static_cast<int>(std::ceil(extent.x/cell));
        const int height=static_cast<int>(std::ceil(extent.y/cell));
        std::vector<int> grid(static_cast<std::size_t>(width)*height,-1),active;
        std::vector<glm::vec2> points;
        std::mt19937 random(seed);
        const auto unit=[&](){return static_cast<float>(random()>>8)*(1.0f/16777216.0f);};
        const auto insert=[&](glm::vec2 p){
            const int index=static_cast<int>(points.size());points.push_back(p);active.push_back(index);
            grid[static_cast<std::size_t>(static_cast<int>(p.y/cell)*width+static_cast<int>(p.x/cell))]=index;
        };
        insert({unit()*extent.x,unit()*extent.y});
        while(!active.empty() && points.size()<8192)
        {
            const auto selected=static_cast<std::size_t>(random())%active.size();
            const auto center=points[static_cast<std::size_t>(active[selected])];bool accepted=false;
            for(int attempt=0;attempt<30;++attempt)
            {
                const float angle=6.28318530718f*unit(),distance=radius*std::sqrt(1+3*unit());
                const auto p=center+glm::vec2{std::cos(angle),std::sin(angle)}*distance;
                if(p.x<0||p.y<0||p.x>=extent.x||p.y>=extent.y) continue;
                const int cx=static_cast<int>(p.x/cell),cy=static_cast<int>(p.y/cell);
                bool clear=true;
                for(int y=std::max(0,cy-2);y<=std::min(height-1,cy+2) && clear;++y)
                    for(int x=std::max(0,cx-2);x<=std::min(width-1,cx+2);++x)
                    {
                        const int i=grid[static_cast<std::size_t>(y*width+x)];
                        if(i>=0 && glm::dot(points[static_cast<std::size_t>(i)]-p,points[static_cast<std::size_t>(i)]-p)<radius*radius)
                        {clear=false;break;}
                    }
                if(clear){insert(p);accepted=true;break;}
            }
            if(!accepted){active[selected]=active.back();active.pop_back();}
        }
        return points;
    }
}
