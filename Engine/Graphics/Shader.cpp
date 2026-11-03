#include "Shader.h"
#include <glad/gl.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace
{
    unsigned int Compile(unsigned int type, const std::filesystem::path& path)
    {
        std::ifstream file(path);
        const std::string source{std::istreambuf_iterator<char>(file), {}};
        if (!file || source.empty())
        {
            std::cerr << "Cannot read shader: " << path << '\n';
            return 0;
        }
        const char* text = source.c_str();
        const auto shader = glCreateShader(type);
        glShaderSource(shader, 1, &text, nullptr);
        glCompileShader(shader);
        int valid = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &valid);
        if (!valid)
        {
            char message[4096] = {};
            glGetShaderInfoLog(shader, sizeof(message), nullptr, message);
            std::cerr << "Shader compile failed: " << path << '\n' << message << '\n';
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }
}

namespace MyEngine
{
    bool Shader::Load(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
    {
        const auto vertex = Compile(GL_VERTEX_SHADER, vertexPath);
        const auto fragment = Compile(GL_FRAGMENT_SHADER, fragmentPath);
        if (!vertex || !fragment)
        {
            if (vertex) glDeleteShader(vertex);
            if (fragment) glDeleteShader(fragment);
            return false;
        }
        const auto program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        int valid = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &valid);
        if (!valid)
        {
            char message[4096] = {};
            glGetProgramInfoLog(program, sizeof(message), nullptr, message);
            std::cerr << "Shader link failed: " << message << '\n';
            glDeleteProgram(program);
            return false;
        }
        Reset();
        m_Id = program;
        return true;
    }
    void Shader::Reset()
    {
        if (m_Id) glDeleteProgram(m_Id);
        m_Id = 0;
    }
}
