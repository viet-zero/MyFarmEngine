#pragma once

struct GLFWwindow;

namespace MyEngine
{
    class Window
    {
    public:
        Window() = default;
        ~Window() = default;

        bool Create(unsigned int width,
            unsigned int height,
            const char* title);

        void Destroy();

        void PollEvents();

        void SwapBuffers();

        bool ShouldClose() const;

        GLFWwindow* GetNativeWindow() const;

    private:
        GLFWwindow* m_Window = nullptr;
    };
}