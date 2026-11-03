#pragma once
#include "Resources/ImageData.h"
#include <filesystem>
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace MyGame
{
    struct AssetRecord
    {
        std::string id,kind,source,exportPath,status;
        int columns=1,rows=1,frameWidth=0,frameHeight=0;
        float anchorX=0.5f,anchorY=1;
        bool hasAlpha=false;
        int firstFrame=0,frameCount=1,marginX=0,marginY=0,spacingX=0,spacingY=0;
    };
    struct AssetValidation
    {
        bool valid=false;
        std::string reason;
        std::filesystem::path path;
        MyEngine::ImageData image;
    };
    class AssetManifest
    {
    public:
        bool Load(const std::filesystem::path& csv, const std::filesystem::path& assetsRoot);
        const AssetRecord* Find(std::string_view id) const;
        AssetValidation Validate(const AssetRecord& record) const;
        const std::vector<AssetRecord>& Records() const { return m_Records; }
        const std::vector<std::string>& Errors() const { return m_Errors; }
        static bool ResolveInside(const std::filesystem::path& root, std::string_view relative,
                                  std::string_view directory, std::filesystem::path& resolved);
        static std::optional<std::array<float,4>> FrameUv(const AssetRecord& record,int relativeFrame);
    private:
        std::filesystem::path m_Root;
        std::vector<AssetRecord> m_Records;
        std::vector<std::string> m_Errors;
    };
}
