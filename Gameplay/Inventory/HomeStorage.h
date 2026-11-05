#pragma once
#include "Inventory.h"
#include <glm/glm.hpp>
#include <cmath>
namespace MyGame
{
    // A separate owned container, using the same stack/atomic inventory implementation.
    struct HomeStorage
    {
        bool enabled=false;
        std::uint64_t ownerId=0;
        glm::vec2 origin{0,0},size{0,0};
        Inventory items{Inventory::MaxSlots};
        bool Contains(glm::vec2 point) const
        {
            return std::isfinite(point.x) && std::isfinite(point.y) &&
                   std::isfinite(origin.x) && std::isfinite(origin.y) && std::isfinite(size.x) && std::isfinite(size.y) &&
                   size.x>0 && size.y>0 && point.x>=origin.x && point.y>=origin.y &&
                   point.x-origin.x<size.x && point.y-origin.y<size.y;
        }
        bool Receives(std::uint64_t actorOwner,glm::vec2 actor,glm::vec2 harvest) const
        {return enabled && ownerId!=0 && ownerId==actorOwner && Contains(actor) && Contains(harvest);}
        static HomeStorage PlayerGarden()
        {
            HomeStorage home;home.enabled=true;home.ownerId=1;
            home.origin={1400,640};home.size={2440,1664};return home;
        }
    };
}
