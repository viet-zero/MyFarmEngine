#pragma once
#include "WorldMap.h"
#include <filesystem>
#include <string>
#include <vector>

namespace MyGame
{
    struct ForegroundTree
    {
        std::string gameplayId;
        glm::vec2 canopyOrigin{},canopySize{};
        float footY=0;
        glm::vec2 interaction{};
        int fruitCount=0;
    };
    class ForegroundTrees
    {
    public:
        bool Load(const std::filesystem::path& csv,const WorldMap& world);
        const std::vector<ForegroundTree>& Entries() const {return m_Trees;}
        bool Contains(std::string_view gameplayId) const;
    private:
        std::vector<ForegroundTree> m_Trees;
    };
}
