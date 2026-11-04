#include "AssetTextures.h"
#include <iostream>

namespace MyGame
{
    bool AssetTextures::Initialize(const std::filesystem::path& assetsRoot)
    {
        Shutdown();
        const bool loaded=m_Manifest.Load(assetsRoot/"Data/Assets.csv",assetsRoot);
        for(const auto& error:m_Manifest.Errors()) std::cerr<<"Asset manifest: "<<error<<'\n';
        return loaded;
    }
    const MyEngine::Texture2D* AssetTextures::Load(std::string_view id)
    {
        const std::string name(id);
        if(const auto it=m_ById.find(name);it!=m_ById.end()) return it->second;
        const auto warn=[&](const std::string& reason)->const MyEngine::Texture2D*
        {
            if(m_Warned.insert(name).second) std::cerr<<"Asset fallback ["<<name<<"]: "<<reason<<'\n';
            m_ById[name]=nullptr;
            return nullptr;
        };
        const auto* record=m_Manifest.Find(id);
        if(!record) return warn("unknown_or_invalid_id");
        auto validation=m_Manifest.Validate(*record);
        if(!validation.valid) return warn(validation.reason);
        m_Bounds[name]=MeasureSpriteFrames(validation.image,*record);
        const auto normalized=validation.path.generic_u8string();
        std::string key(normalized.begin(),normalized.end());
        key+='|'+std::to_string(record->columns)+'x'+std::to_string(record->rows);
        key+='|'+std::to_string(record->frameWidth)+'x'+std::to_string(record->frameHeight);
        key+='|'+std::to_string(record->marginX)+','+std::to_string(record->marginY);
        key+='|'+std::to_string(record->spacingX)+','+std::to_string(record->spacingY);
        if(const auto it=m_Textures.find(key);it!=m_Textures.end())
            return m_ById[name]=it->second.get();
        auto texture=std::make_unique<MyEngine::Texture2D>();
        if(!texture->Upload(validation.image)) return warn("gpu_upload_failed");
        const auto* result=texture.get();
        m_Textures.emplace(key,std::move(texture));
        m_ById[name]=result;
        return result;
    }
    void AssetTextures::Shutdown()
    {
        m_ById.clear();m_Textures.clear();m_Warned.clear();m_Bounds.clear();
        m_Manifest={};
    }
}
