#pragma once
#include <glm/glm.hpp>
#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include "Gameplay/Components/ColliderComponent.h"

namespace MyGame
{
    enum class Zone {Town,Farm,LakeForest};
    enum class Terrain {Grass,Path,Soil,Water,Forest};
    struct WorldCell {Zone zone;Terrain terrain;bool blocked;};
    struct TileRange {int left=0,top=0,right=0,bottom=0;}; // exclusive right/bottom
    struct WorldChunk {int x=0,y=0;TileRange tiles;};
    struct WorldObject
    {
        std::string gameplayId,visualId;
        glm::vec2 position;
        glm::vec2 size;
        bool blocked=false;
        bool active=true;
    };
    class WorldMap
    {
    public:
        static constexpr int TileSize=64, ChunkTiles=16, Width=96, Height=64;
        static constexpr int TownEndX=32,FarmEndX=64;
        static constexpr int LakeCenterX=81,LakeCenterY=19,LakeRadiusX=15,LakeRadiusY=10;
        static constexpr glm::vec2 Size(){return {Width*TileSize,Height*TileSize};}
        WorldMap();
        bool LoadObjects(const std::filesystem::path& csv);
        bool LoadStaticColliders(const std::filesystem::path& csv);
        const WorldCell* TryCell(int x,int y) const;
        bool CanMoveTo(glm::vec2 point) const;
        bool CanOccupy(glm::vec2 center,ColliderComponent collider) const;
        bool HasLineOfSight(glm::vec2 from,glm::vec2 to) const;
        TileRange VisibleTiles(glm::vec2 center,glm::vec2 viewSize,int padding=1) const;
        TileRange VisibleChunks(glm::vec2 center,glm::vec2 viewSize) const;
        WorldChunk ChunkAt(int x,int y) const;
        Zone ZoneAt(glm::vec2 point) const;
        glm::vec2 Spawn(Zone zone) const;
        const std::vector<WorldObject>& Objects() const {return m_Objects;}
        const std::vector<WorldObject>& StaticColliders() const {return m_StaticColliders;}
        bool SetObjectActive(std::string_view gameplayId,bool active);
        static bool Intersects(glm::vec2 a,glm::vec2 aSize,glm::vec2 b,glm::vec2 bSize);
    private:
        std::array<WorldCell,Width*Height> m_Cells{};
        std::vector<WorldObject> m_Objects;
        std::vector<WorldObject> m_StaticColliders;
    };
}
