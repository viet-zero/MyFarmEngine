#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace MyEngine
{
    inline std::filesystem::path Utf8Path(std::string_view text)
    { return std::filesystem::path(std::u8string(text.begin(),text.end())); }
    struct ImageData
    {
        int width = 0, height = 0;
        bool alphaChannel = false;
        unsigned char alphaMin = 255, alphaMax = 255;
        std::vector<unsigned char> rgba;
    };
    // CPU-only PNG validation; does not require an OpenGL context.
    bool DecodePng(const std::filesystem::path& path, ImageData& result, std::string& error);
}
