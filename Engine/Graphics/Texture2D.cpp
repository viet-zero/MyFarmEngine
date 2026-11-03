#include "Texture2D.h"
#include <glad/gl.h>
#include <iostream>

namespace MyEngine
{
    Texture2D::~Texture2D() { Reset(); }
    bool Texture2D::Load(const std::filesystem::path& path)
    {
        ImageData image;
        std::string error;
        if (!DecodePng(path,image,error))
        {
            std::cerr << "Cannot decode PNG: " << path << ": " << error << '\n';
            return false;
        }
        return Upload(image);
    }
    bool Texture2D::Upload(const ImageData& image)
    {
        const int width=image.width,height=image.height;
        if(width<=0 || height<=0 || image.rgba.size()!=static_cast<std::size_t>(width)*static_cast<std::size_t>(height)*4)
            return false;
        int maxSize = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
        if (width > maxSize || height > maxSize)
        {
            std::cerr << "Texture exceeds GPU size limit\n";
            return false;
        }
        unsigned int candidate=0;
        glGenTextures(1, &candidate);
        glBindTexture(GL_TEXTURE_2D, candidate);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        if(glGetError()!=GL_NO_ERROR) {glDeleteTextures(1,&candidate);return false;}
        Reset();
        m_Id=candidate;
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
