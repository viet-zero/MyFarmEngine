#pragma once
#include <glm/glm.hpp>

namespace MyGame
{
    // Gameplay footprint measured around the actor's feet; independent of sprite size.
    struct ColliderComponent
    {
        glm::vec2 halfSize{18.0f,12.0f};
    };
}
