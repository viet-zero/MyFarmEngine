#pragma once
#include "Gameplay/World/WorldMap.h"
#include <span>

namespace MyGame
{
    class CollisionSystem
    {
    public:
        static glm::vec2 MoveAndSlide(const WorldMap& world,glm::vec2 position,
            glm::vec2 direction,float speed,float deltaTime,ColliderComponent collider,
            std::span<const DynamicCollider> otherActors={});
    };
}
