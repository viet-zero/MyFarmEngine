#pragma once
#include "SpriteGeometry.h"
#include "Graphics/Canvas2D.h"
#include <filesystem>
#include <map>
#include <string>

namespace MyGame
{
    // Pure coordinate rules shared by rendering and tests.
    unsigned int CanonicalTerrainMask(unsigned int mask);
    int ArtDirection(glm::vec2 movement,int previous);
    struct ArtFrame
    {
        std::array<float,4> uv{};
        SpriteFrameBounds bounds;
        glm::vec2 pivot{};
    };
    class ArtAtlas
    {
    public:
        bool Load(const std::filesystem::path& base);
        void Reset();
        const ArtFrame* Find(std::string_view id) const;
        const MyEngine::Texture2D& Texture() const {return m_Texture;}
        void Draw(MyEngine::Canvas2D& canvas,std::string_view id,glm::vec2 origin,glm::vec2 size) const;
    private:
        MyEngine::Texture2D m_Texture;
        std::map<std::string,ArtFrame,std::less<>> m_Frames;
    };
    class WorldArt
    {
    public:
        bool Load(const std::filesystem::path& root);
        void Reset();
        bool Ready() const {return m_Ready;}
        const ArtAtlas& Terrain(int season) const {return m_Terrain[static_cast<std::size_t>(season)];}
        const ArtAtlas& Sprites() const {return m_Sprites;}
        const ArtFrame* Actor(int direction,bool walking,int frame) const;
    private:
        std::array<ArtAtlas,4> m_Terrain;
        ArtAtlas m_Sprites;
        bool m_Ready=false;
    };
}
