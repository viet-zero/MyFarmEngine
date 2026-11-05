#pragma once
#include "Texture2D.h"
#include "Shader.h"
#include "Camera2D.h"
#include <array>
#include <vector>

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
        unsigned int DrawCalls() const { return m_DrawCalls; }
    private:
        void Draw(unsigned int texture, float x, float y, float width, float height,
                  const std::array<float, 4>& color, float uvX = 1.0f, float uvY = 1.0f);
        struct Instance { std::array<float,4> rect, uv, tint; };
        static constexpr int BatchCapacity = 1024;
        void Submit(unsigned int texture, const Instance& instance, bool ellipse = false);
        void Flush();
        void SetProjection(const glm::mat4& projection);
        Shader m_Shader;
        unsigned int m_Vao = 0, m_Vbo = 0, m_White = 0;
        unsigned int m_InstanceBuffer = 0, m_Texture = 0, m_DrawCalls = 0;
        bool m_Ellipse = false;
        std::vector<Instance> m_Batch;
        int m_Projection = -1, m_EllipseMask = -1;
        int m_Width = 0, m_Height = 0;
    };
}
