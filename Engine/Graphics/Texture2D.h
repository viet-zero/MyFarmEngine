#pragma once
#include <filesystem>
#include "Resources/ImageData.h"

namespace MyEngine
{
    class Texture2D
    {
    public:
        Texture2D() = default;
        ~Texture2D();
        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;
        bool Load(const std::filesystem::path& path);
        bool Upload(const ImageData& image);
        void Reset();
        unsigned int Id() const { return m_Id; }
        int Width() const { return m_Width; }
        int Height() const { return m_Height; }
    private:
        unsigned int m_Id = 0;
        int m_Width = 0, m_Height = 0;
    };
}
