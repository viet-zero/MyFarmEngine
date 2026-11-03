#include "Texture2D.h"
#include <glad/gl.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <stb_image.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace MyEngine
{
    Texture2D::~Texture2D() { Reset(); }
    bool Texture2D::Load(const std::filesystem::path& path)
    {
        // filesystem::path keeps Unicode Windows paths intact.
        std::ifstream file(path, std::ios::binary);
        const std::vector<unsigned char> encoded{
            std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        if (!file || encoded.empty() ||
            encoded.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
        {
            std::cerr << "Cannot read PNG: " << path << '\n';
            return false;
        }
        int width = 0, height = 0, channels = 0;
        auto* pixels = stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()),
                                             &width, &height, &channels, 4);
        if (!pixels)
        {
            std::cerr << "Cannot decode PNG: " << path << ": " << stbi_failure_reason() << '\n';
            return false;
        }
        int maxSize = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
        if (width > maxSize || height > maxSize)
        {
            stbi_image_free(pixels);
            std::cerr << "Texture exceeds GPU size limit: " << path << '\n';
            return false;
        }
        Reset();
        glGenTextures(1, &m_Id);
        glBindTexture(GL_TEXTURE_2D, m_Id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        stbi_image_free(pixels);
        glBindTexture(GL_TEXTURE_2D, 0);
        m_Width = width;
        m_Height = height;
        return true;
    }
    void Texture2D::Reset()
    {
        if (m_Id)
            glDeleteTextures(1, &m_Id);
        m_Id = 0;
        m_Width = m_Height = 0;
    }
}
