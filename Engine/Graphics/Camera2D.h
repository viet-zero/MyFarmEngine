#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace MyEngine
{
    // World units use pixels at zoom 1; world Y points down, like PNG rows.
    class Camera2D
    {
    public:
        void Resize(int width, int height)
        {
            m_Width = std::max(1, width);
            m_Height = std::max(1, height);
            Constrain();
        }
        void SetWorldSize(glm::vec2 size) { m_WorldSize = glm::max(size, glm::vec2(1)); Constrain(); }
        void SetPosition(glm::vec2 position) { m_Position = position; Constrain(); }
        void SetZoom(float zoom) { m_Zoom = std::clamp(zoom, 0.5f, 2.5f); Constrain(); }
        float Zoom() const { return m_Zoom; }
        glm::vec2 Position() const { return m_Position; }
        glm::vec2 ViewSize() const
        {
            const float h = 720.0f / m_Zoom;
            return {h * static_cast<float>(m_Width) / static_cast<float>(m_Height), h};
        }
        void Follow(glm::vec2 target, float deltaTime)
        {
            SetPosition(glm::mix(m_Position, target, 1.0f - std::exp(-8.0f * std::max(0.0f, deltaTime))));
        }
        glm::mat4 ViewProjection() const
        {
            const auto half = ViewSize()*0.5f;
            return glm::ortho(m_Position.x-half.x, m_Position.x+half.x,
                              m_Position.y+half.y, m_Position.y-half.y, -1.0f, 1.0f);
        }
        glm::vec2 WorldToScreen(glm::vec2 point) const
        {
            return (point - m_Position + ViewSize()*0.5f) / ViewSize()
                * glm::vec2(m_Width, m_Height);
        }
        glm::vec2 ScreenToWorld(glm::vec2 point) const
        {
            return point / glm::vec2(m_Width, m_Height) * ViewSize()
                + m_Position - ViewSize()*0.5f;
        }
    private:
        void Constrain()
        {
            // Fit limits avoid exposing empty strips outside the world on resize/zoom-out.
            const float minimum = std::max({0.5f, 720.0f/m_WorldSize.y,
                720.0f*static_cast<float>(m_Width)/static_cast<float>(m_Height)/m_WorldSize.x});
            m_Zoom = std::clamp(m_Zoom, minimum, std::max(2.5f,minimum));
            const auto half = ViewSize()*0.5f;
            for (int axis=0; axis<2; ++axis)
                m_Position[axis] = m_WorldSize[axis] <= half[axis]*2
                    ? m_WorldSize[axis]*0.5f
                    : std::clamp(m_Position[axis], half[axis], m_WorldSize[axis]-half[axis]);
        }
        int m_Width = 1280, m_Height = 720;
        float m_Zoom = 1.0f;
        glm::vec2 m_Position{1200,800}, m_WorldSize{2400,1600};
    };
}
