#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
namespace MyGame
{
    std::uint32_t VisualHash(int x,int y,std::uint32_t seed=20261110u);
    std::vector<glm::vec2> PoissonDiskPoints(glm::vec2 extent,float radius,std::uint32_t seed);
}
