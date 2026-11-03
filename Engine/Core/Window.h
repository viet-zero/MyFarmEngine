#pragma once

struct GLFWwindow;

namespace MyEngine
{
    // Phase 1 supports one active window/application at a time.
    class Window
    {
    public:
        Window() = default;
        ~Window() { Destroy(); }
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool Create(unsigned int width, unsigned int height, const char* title,
                    bool vsync = true);
        void Destroy();
        void PollEvents();
        void SwapBuffers();
        bool ShouldClose() const;
        void GetFramebufferSize(int& width, int& height) const;
        GLFWwindow* GetNativeWindow() const;

    private:
        GLFWwindow* m_Window = nullptr;
        bool m_GlfwInitialized = false;
    };
}
