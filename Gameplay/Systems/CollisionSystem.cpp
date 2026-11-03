#include "CollisionSystem.h"
#include <algorithm>
#include <cmath>

namespace MyGame
{
    glm::vec2 CollisionSystem::MoveAndSlide(const WorldMap& world,glm::vec2 position,
        glm::vec2 direction,float speed,float deltaTime,ColliderComponent collider,
        std::span<const DynamicCollider> otherActors)
    {
        if(!std::isfinite(direction.x)||!std::isfinite(direction.y)||
           !std::isfinite(speed)||!std::isfinite(deltaTime)||speed<=0 || deltaTime<=0 ||
           !world.CanOccupy(position,collider)) return position;
        const float magnitude=std::hypot(direction.x,direction.y);
        if(!std::isfinite(magnitude)||magnitude<=0) return position;
        // Clamp abnormal frame spikes and inputs to keep substep count bounded.
        const glm::vec2 delta=(direction/magnitude)*std::min(speed,2000.0f)*std::min(deltaTime,0.1f);
        const int steps=std::max(1,static_cast<int>(std::ceil(std::max(std::abs(delta.x),std::abs(delta.y))/8.0f)));
        const glm::vec2 step=delta/static_cast<float>(steps);
        const auto clear=[&](glm::vec2 candidate){
            if(!world.CanOccupy(candidate,collider)) return false;
            for(const auto& other:otherActors)
                if(std::isfinite(other.center.x)&&std::isfinite(other.center.y)&&
                   WorldMap::Intersects(candidate-collider.halfSize,collider.halfSize*2.0f,
                       other.center-other.collider.halfSize,other.collider.halfSize*2.0f)) return false;
            return true;
        };
        for(int i=0;i<steps;++i)
        {
            const float nextX=std::clamp(position.x+step.x,collider.halfSize.x,
                                           WorldMap::Size().x-collider.halfSize.x);
            if(clear({nextX,position.y})) position.x=nextX;
            const float nextY=std::clamp(position.y+step.y,collider.halfSize.y,
                                           WorldMap::Size().y-collider.halfSize.y);
            if(clear({position.x,nextY})) position.y=nextY;
        }
        return position;
    }
}
