#pragma once
#include "Gameplay/Assets/AssetManifest.h"
#include "Graphics/Texture2D.h"
#include <map>
#include <memory>
#include <set>

namespace MyGame
{
    // Visual resource owner only. Gameplay state/components can keep stable asset IDs.
    class AssetTextures
    {
    public:
        bool Initialize(const std::filesystem::path& assetsRoot);
        const MyEngine::Texture2D* Load(std::string_view id);
        const AssetRecord* Record(std::string_view id) const { return m_Manifest.Find(id); }
        void Shutdown();
        std::size_t CachedCount() const { return m_Textures.size(); }
    private:
        AssetManifest m_Manifest;
        std::map<std::string,std::unique_ptr<MyEngine::Texture2D>> m_Textures;
        std::map<std::string,const MyEngine::Texture2D*> m_ById;
        std::set<std::string> m_Warned;
    };
}
