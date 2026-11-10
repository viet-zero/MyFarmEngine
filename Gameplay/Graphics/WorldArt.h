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
    struct VisualScale
    {
        float human=112,tree=336,house=420,gate=80,fence=64;
        bool Load(const std::filesystem::path& path);
    };
    class WorldArt
    {
    public:
        bool Load(const std::filesystem::path& root);
        void Reset();
        bool LoadCozy(const std::filesystem::path& root);
        bool CozyReady() const {return m_CozyReady;}
        const ArtAtlas& Details() const {return m_Details;}
        const ArtAtlas& Ground() const {return m_Ground;}
        const ArtAtlas& Props() const {return m_Props;}
        bool Ready() const {return m_Ready;}
        const ArtAtlas& Terrain(int season) const {return m_Terrain[static_cast<std::size_t>(season)];}
        const ArtAtlas& Sprites() const {return m_Sprites;}
        const ArtFrame* Actor(int direction,bool walking,int frame) const;
    private:
        std::array<ArtAtlas,4> m_Terrain;
        ArtAtlas m_Sprites,m_Ground,m_Props,m_Details;
        bool m_CozyReady=false;
        bool m_Ready=false;
    };
}
