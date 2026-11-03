#include "ImageData.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_MAX_DIMENSIONS 8192
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <stb_image.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace MyEngine
{
    bool DecodePng(const std::filesystem::path& path, ImageData& result, std::string& error)
    {
        result = {};
        error.clear();
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) { error="file_missing_or_unreadable"; return false; }
        const auto length=file.tellg();
        if (length<8 || length>32*1024*1024) { error="encoded_size_limit"; return false; }
        std::vector<unsigned char> encoded(static_cast<std::size_t>(length));
        file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(encoded.data()),static_cast<std::streamsize>(encoded.size())))
        { error="file_read_failed"; return false; }
        constexpr std::array<unsigned char,8> signature={137,80,78,71,13,10,26,10};
        if (!std::equal(signature.begin(),signature.end(),encoded.begin()))
        { error="not_png"; return false; }
        int w=0,h=0,channels=0;
        const auto size=static_cast<int>(encoded.size());
        if (!stbi_info_from_memory(encoded.data(),size,&w,&h,&channels) || w<=0 || h<=0 ||
            static_cast<std::uint64_t>(w)*static_cast<std::uint64_t>(h)*4>64*1024*1024)
        { error="invalid_png_or_decoded_size_limit"; return false; }
        std::unique_ptr<unsigned char,decltype(&stbi_image_free)> pixels(
            stbi_load_from_memory(encoded.data(),size,&w,&h,&channels,4),stbi_image_free);
        if (!pixels) { error="png_decode_failed"; return false; }
        result.width=w;
        result.height=h;
        result.alphaChannel=channels==2 || channels==4;
        const auto count=static_cast<std::size_t>(w)*static_cast<std::size_t>(h)*4;
        result.rgba.assign(pixels.get(),pixels.get()+count);
        result.alphaMin=255;
        result.alphaMax=0;
        for (std::size_t i=3;i<count;i+=4)
        {
            result.alphaMin=std::min(result.alphaMin,result.rgba[i]);
            result.alphaMax=std::max(result.alphaMax,result.rgba[i]);
        }
        return true;
    }
}
