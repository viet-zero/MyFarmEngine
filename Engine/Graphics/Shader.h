#pragma once
#include <filesystem>

namespace MyEngine
{
    class Shader
    {
    public:
        Shader() = default;
        ~Shader() { Reset(); }
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        bool Load(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
        void Reset();
        unsigned int Id() const { return m_Id; }
    private:
        unsigned int m_Id = 0;
    };
}
