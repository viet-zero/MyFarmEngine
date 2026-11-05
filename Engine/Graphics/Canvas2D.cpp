#include "Canvas2D.h"
#include "Resources/AssetPath.h"
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cstddef>
#include <iostream>

namespace MyEngine
{
    bool Canvas2D::Initialize()
    {
        if (m_Shader.Id())
            return true;
        if (!m_Shader.Load(FindAsset("Shaders/canvas.vert"), FindAsset("Shaders/canvas.frag")))
            return false;
        m_Projection = glGetUniformLocation(m_Shader.Id(), "viewProjection");
        m_EllipseMask = glGetUniformLocation(m_Shader.Id(), "ellipseMask");
        // Two triangles; the top-left image row maps to the top of the window.
        constexpr float vertices[] = {0,0, 1,0, 1,1, 0,0, 1,1, 0,1};
        glGenVertexArrays(1, &m_Vao);
        glGenBuffers(1, &m_Vbo);
        glBindVertexArray(m_Vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
        glGenBuffers(1, &m_InstanceBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
        glBufferData(GL_ARRAY_BUFFER, BatchCapacity * sizeof(Instance), nullptr, GL_STREAM_DRAW);
        constexpr std::array offsets{offsetof(Instance, rect), offsetof(Instance, uv), offsetof(Instance, tint)};
        for(unsigned int i=0; i<offsets.size(); ++i)
        {
            glEnableVertexAttribArray(i+1);
            glVertexAttribPointer(i+1, 4, GL_FLOAT, GL_FALSE, sizeof(Instance),
                                  reinterpret_cast<const void*>(offsets[i]));
            glVertexAttribDivisor(i+1, 1);
        }
        m_Batch.reserve(BatchCapacity);
        constexpr unsigned char white[] = {255,255,255,255};
        glGenTextures(1, &m_White);
        glBindTexture(GL_TEXTURE_2D, m_White);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUseProgram(m_Shader.Id());
        glUniform1i(glGetUniformLocation(m_Shader.Id(), "image"), 0);
        glUniform1i(m_EllipseMask,0);
        End();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return true;
    }
    void Canvas2D::Begin(int width, int height)
    {
        m_DrawCalls = 0;
        m_Width = width;
        m_Height = height;
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(m_Shader.Id());
        glUniform1i(m_EllipseMask,0);
        glBindVertexArray(m_Vao);
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_CULL_FACE);
        if (width > 0 && height > 0)
            SetProjection(glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f));
    }
    void Canvas2D::SetProjection(const glm::mat4& projection)
    {
        glUniformMatrix4fv(m_Projection, 1, GL_FALSE, glm::value_ptr(projection));
    }
    void Canvas2D::BeginWorld(int width, int height, const Camera2D& camera)
    {
        Begin(width, height);
        SetProjection(camera.ViewProjection());
    }
    void Canvas2D::Draw(unsigned int texture, float x, float y, float width, float height,
                        const std::array<float, 4>& color, float uvX, float uvY)
    {
        Submit(texture, {{x,y,width,height},{(1-uvX)*0.5f,(1-uvY)*0.5f,uvX,uvY},color});
    }
    void Canvas2D::Submit(unsigned int texture,const Instance& instance,bool ellipse)
    {
        if(!texture || m_Width<=0 || m_Height<=0) return;
        // Only merge adjacent compatible draws to preserve alpha compositing order.
        if(texture!=m_Texture || ellipse!=m_Ellipse || m_Batch.size()==BatchCapacity) Flush();
        m_Texture=texture;
        m_Ellipse=ellipse;
        m_Batch.push_back(instance);
    }
    void Canvas2D::Flush()
    {
        if(m_Batch.empty()) return;
        glUniform1i(m_EllipseMask,m_Ellipse?1:0);
        glBindTexture(GL_TEXTURE_2D,m_Texture);
        glBindBuffer(GL_ARRAY_BUFFER,m_InstanceBuffer);
        glBufferSubData(GL_ARRAY_BUFFER,0,m_Batch.size()*sizeof(Instance),m_Batch.data());
        glDrawArraysInstanced(GL_TRIANGLES,0,6,static_cast<GLsizei>(m_Batch.size()));
        m_Batch.clear();
        ++m_DrawCalls;
    }
    void Canvas2D::Image(const Texture2D& texture, float x, float y, float width, float height)
    {
        Draw(texture.Id(), x, y, width, height, {1,1,1,1});
    }
    void Canvas2D::Sprite(const Texture2D& texture, float x, float y, float width, float height,
                          const std::array<float, 4>& uv, const std::array<float, 4>& tint)
    {
        Submit(texture.Id(), {{x,y,width,height},uv,tint});
    }
    void Canvas2D::SpriteEllipse(const Texture2D& texture,float x,float y,float width,float height,
                                 const std::array<float,4>& uv)
    {
        Submit(texture.Id(), {{x,y,width,height},uv,{1,1,1,1}},true);
    }
    void Canvas2D::Cover(const Texture2D& texture)
    {
        if (!texture.Id() || m_Width <= 0 || m_Height <= 0)
            return;
        const float screenRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        const float imageRatio = static_cast<float>(texture.Width()) / static_cast<float>(texture.Height());
        Draw(texture.Id(), 0, 0, static_cast<float>(m_Width), static_cast<float>(m_Height),
             {1,1,1,1}, std::min(1.0f, screenRatio/imageRatio),
             std::min(1.0f, imageRatio/screenRatio));
    }
    void Canvas2D::Rectangle(float x, float y, float width, float height,
                             const std::array<float, 4>& color)
    {
        Draw(m_White, x, y, width, height, color);
    }
    void Canvas2D::End()
    {
        Flush();
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        glDisable(GL_BLEND);
    }
    void Canvas2D::Shutdown()
    {
        if (m_White) glDeleteTextures(1, &m_White);
        m_Batch.clear();
        if (m_InstanceBuffer) glDeleteBuffers(1, &m_InstanceBuffer);
        m_InstanceBuffer = 0;
        if (m_Vbo) glDeleteBuffers(1, &m_Vbo);
        if (m_Vao) glDeleteVertexArrays(1, &m_Vao);
        m_Shader.Reset();
        m_White = m_Vbo = m_Vao = 0;
    }
}
