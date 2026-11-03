#pragma once
#include "Texture2D.h"
#include <array>

namespace MyEngine
{
    // Pixel coordinates, origin at top-left; bitmap assets keep their native UV orientation.
    class Canvas2D
    {
    public:
        Canvas2D() = default;
        ~Canvas2D() { Shutdown(); }
        Canvas2D(const Canvas2D&) = delete;
        Canvas2D& operator=(const Canvas2D&) = delete;
        bool Initialize();
        void Shutdown();
        void Begin(int width, int height);
        void Image(const Texture2D& texture, float x, float y, float width, float height);
        void Cover(const Texture2D& texture);
        void Rectangle(float x, float y, float width, float height,
                       const std::array<float, 4>& color);
        void End();
    private:
        void Draw(unsigned int texture, float x, float y, float width, float height,
                  const std::array<float, 4>& color, float uvX = 1.0f, float uvY = 1.0f);
        unsigned int m_Program = 0, m_Vao = 0, m_Vbo = 0, m_White = 0;
        int m_Rect = -1, m_Color = -1, m_Uv = -1;
        int m_Width = 0, m_Height = 0;
    };
}
