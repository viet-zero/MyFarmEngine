#pragma once
#include "Texture2D.h"
#include "Shader.h"
#include "Camera2D.h"
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
        void BeginWorld(int width, int height, const Camera2D& camera);
        void Image(const Texture2D& texture, float x, float y, float width, float height);
        void Sprite(const Texture2D& texture, float x, float y, float width, float height,
                    const std::array<float, 4>& uv = {0,0,1,1},
                    const std::array<float, 4>& tint = {1,1,1,1});
        void SpriteEllipse(const Texture2D& texture,float x,float y,float width,float height,
                           const std::array<float,4>& uv);
        void Cover(const Texture2D& texture);
        void Rectangle(float x, float y, float width, float height,
                       const std::array<float, 4>& color);
        void End();
    private:
        void Draw(unsigned int texture, float x, float y, float width, float height,
                  const std::array<float, 4>& color, float uvX = 1.0f, float uvY = 1.0f);
        void SetProjection(const glm::mat4& projection);
        Shader m_Shader;
        unsigned int m_Vao = 0, m_Vbo = 0, m_White = 0;
        int m_Rect = -1, m_Color = -1, m_Uv = -1, m_Projection = -1, m_EllipseMask = -1;
        int m_Width = 0, m_Height = 0;
    };
}
