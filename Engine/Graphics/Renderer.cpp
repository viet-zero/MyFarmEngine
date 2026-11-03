#include "Renderer.h"
#include <glad/gl.h>

namespace MyEngine
{
    bool Renderer::Initialize()
    {
        m_Width = m_Height = 0;
        return true;
    }
    void Renderer::Resize(int width, int height)
    {
        if (width != m_Width || height != m_Height)
        {
            glViewport(0, 0, width, height);
            m_Width = width;
            m_Height = height;
        }
    }
    void Renderer::BeginFrame()
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void Renderer::EndFrame() {}
    void Renderer::Shutdown() { m_Width = m_Height = 0; }
}
