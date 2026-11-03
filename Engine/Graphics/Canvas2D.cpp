#include "Canvas2D.h"
#include <glad/gl.h>
#include <algorithm>
#include <iostream>

namespace
{
    constexpr auto VertexShader = R"(#version 330 core
layout(location=0) in vec2 position;
uniform vec4 rectangle;
uniform vec2 uvScale;
out vec2 uv;
void main() {
    gl_Position = vec4(rectangle.xy + position * rectangle.zw, 0.0, 1.0);
    uv = (position - 0.5) * uvScale + 0.5;
})";
    constexpr auto FragmentShader = R"(#version 330 core
in vec2 uv;
uniform sampler2D image;
uniform vec4 tint;
out vec4 color;
void main() { color = texture(image, uv) * tint; }
)";
    unsigned int Compile(unsigned int type, const char* source)
    {
        const auto shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        int valid = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &valid);
        if (!valid)
        {
            char message[2048] = {};
            glGetShaderInfoLog(shader, sizeof(message), nullptr, message);
            std::cerr << "Canvas shader failed: " << message << '\n';
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }
}

namespace MyEngine
{
    bool Canvas2D::Initialize()
    {
        if (m_Program)
            return true;
        const auto vertex = Compile(GL_VERTEX_SHADER, VertexShader);
        const auto fragment = Compile(GL_FRAGMENT_SHADER, FragmentShader);
        if (!vertex || !fragment)
        {
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            return false;
        }
        m_Program = glCreateProgram();
        glAttachShader(m_Program, vertex);
        glAttachShader(m_Program, fragment);
        glLinkProgram(m_Program);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        int valid = 0;
        glGetProgramiv(m_Program, GL_LINK_STATUS, &valid);
        if (!valid)
        {
            char message[2048] = {};
            glGetProgramInfoLog(m_Program, sizeof(message), nullptr, message);
            std::cerr << "Canvas link failed: " << message << '\n';
            Shutdown();
            return false;
        }
        m_Rect = glGetUniformLocation(m_Program, "rectangle");
        m_Color = glGetUniformLocation(m_Program, "tint");
        m_Uv = glGetUniformLocation(m_Program, "uvScale");
        // Two triangles; the top-left image row maps to the top of the window.
        constexpr float vertices[] = {0,0, 1,0, 1,1, 0,0, 1,1, 0,1};
        glGenVertexArrays(1, &m_Vao);
        glGenBuffers(1, &m_Vbo);
        glBindVertexArray(m_Vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
        constexpr unsigned char white[] = {255,255,255,255};
        glGenTextures(1, &m_White);
        glBindTexture(GL_TEXTURE_2D, m_White);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUseProgram(m_Program);
        glUniform1i(glGetUniformLocation(m_Program, "image"), 0);
        End();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return true;
    }
    void Canvas2D::Begin(int width, int height)
    {
        m_Width = width;
        m_Height = height;
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(m_Program);
        glBindVertexArray(m_Vao);
        glActiveTexture(GL_TEXTURE0);
    }
    void Canvas2D::Draw(unsigned int texture, float x, float y, float width, float height,
                        const std::array<float, 4>& color, float uvX, float uvY)
    {
        if (!texture || m_Width <= 0 || m_Height <= 0)
            return;
        const float screenWidth = static_cast<float>(m_Width);
        const float screenHeight = static_cast<float>(m_Height);
        glUniform4f(m_Rect, 2*x/screenWidth-1, 1-2*y/screenHeight,
                    2*width/screenWidth, -2*height/screenHeight);
        glUniform4fv(m_Color, 1, color.data());
        glUniform2f(m_Uv, uvX, uvY);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    void Canvas2D::Image(const Texture2D& texture, float x, float y, float width, float height)
    {
        Draw(texture.Id(), x, y, width, height, {1,1,1,1});
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
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        glDisable(GL_BLEND);
    }
    void Canvas2D::Shutdown()
    {
        if (m_White) glDeleteTextures(1, &m_White);
        if (m_Vbo) glDeleteBuffers(1, &m_Vbo);
        if (m_Vao) glDeleteVertexArrays(1, &m_Vao);
        if (m_Program) glDeleteProgram(m_Program);
        m_White = m_Vbo = m_Vao = m_Program = 0;
    }
}
