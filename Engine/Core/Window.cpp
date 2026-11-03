#include "Window.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <limits>

namespace MyEngine
{
    bool Window::Create(unsigned int width, unsigned int height,
                        const char* title, bool vsync)
    {
        if (m_Window)
            return true;
        if (width == 0 || height == 0 || title == nullptr ||
            width > static_cast<unsigned int>(std::numeric_limits<int>::max()) ||
            height > static_cast<unsigned int>(std::numeric_limits<int>::max()))
        {
            std::cerr << "Invalid window dimensions or title.\n";
            return false;
        }
        glfwSetErrorCallback([](int code, const char* description)
        {
            std::cerr << "GLFW error " << code << ": " << description << '\n';
        });
        if (!glfwInit())
            return false;
        m_GlfwInitialized = true;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        m_Window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height),
                                    title, nullptr, nullptr);
        if (m_Window == nullptr)
        {
            Destroy();
            return false;
        }
        glfwMakeContextCurrent(m_Window);
        if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)) ||
            !GLAD_GL_VERSION_3_3)
        {
            std::cerr << "OpenGL 3.3 initialization failed.\n";
            Destroy();
            return false;
        }
        glfwSwapInterval(vsync ? 1 : 0);
        std::cout << "OpenGL " << glGetString(GL_VERSION)
                  << " | GPU " << glGetString(GL_RENDERER)
                  << " | VSync " << (vsync ? "on" : "off") << '\n';
        return true;
    }

    void Window::Destroy()
    {
        if (m_Window)
        {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }
        if (m_GlfwInitialized)
        {
            glfwTerminate();
            m_GlfwInitialized = false;
        }
    }
    void Window::PollEvents() { glfwPollEvents(); }
    void Window::SwapBuffers() { glfwSwapBuffers(m_Window); }
    bool Window::ShouldClose() const
    {
        return !m_Window || glfwWindowShouldClose(m_Window);
    }
    void Window::GetFramebufferSize(int& width, int& height) const
    {
        glfwGetFramebufferSize(m_Window, &width, &height);
    }
    GLFWwindow* Window::GetNativeWindow() const { return m_Window; }
}
