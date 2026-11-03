#pragma once
#include <glm/glm.hpp>
#include <algorithm>

namespace MyGame
{
    inline glm::vec2 MoveWithinBounds(glm::vec2 position, glm::vec2 direction,
                                     float speed, float deltaTime, glm::vec2 worldSize)
    {
        const float length = glm::length(direction);
        if (length > 0) direction /= length;
        position += direction * std::max(0.0f, speed) * std::max(0.0f, deltaTime);
        return glm::clamp(position, glm::vec2(72), glm::max(glm::vec2(72), worldSize-glm::vec2(72)));
    }
}
