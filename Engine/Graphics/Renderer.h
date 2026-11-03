#pragma once

namespace MyEngine
{
    class Renderer
    {
    public:
        bool Initialize();
        void Resize(int width, int height);
        void BeginFrame();
        void EndFrame();
        void Shutdown();

    private:
        int m_Width = 0;
        int m_Height = 0;
    };
}
