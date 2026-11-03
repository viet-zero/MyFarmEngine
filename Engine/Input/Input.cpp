#include "Input.h"
#include <GLFW/glfw3.h>
#include <array>
#include <utility>

namespace { std::array<bool, GLFW_KEY_LAST + 1> keyPresses{}; }

namespace MyEngine
{
    GLFWwindow* Input::s_Window = nullptr;
    void Input::Initialize(GLFWwindow* window)
    {
        if (s_Window) glfwSetKeyCallback(s_Window, nullptr);
        s_Window = window;
        keyPresses.fill(false);
        if (s_Window)
            glfwSetKeyCallback(s_Window, [](GLFWwindow*, int key, int, int action, int)
            {
                if (action == GLFW_PRESS && key >= 0 && key <= GLFW_KEY_LAST)
                    keyPresses[static_cast<std::size_t>(key)] = true;
            });
    }
    bool Input::IsKeyPressed(int key)
    {
        return s_Window && key >= GLFW_KEY_SPACE && key <= GLFW_KEY_LAST &&
            glfwGetKey(s_Window, key) == GLFW_PRESS;
    }
    bool Input::ConsumeKeyPress(int key)
    {
        return s_Window && key >= 0 && key <= GLFW_KEY_LAST &&
            std::exchange(keyPresses[static_cast<std::size_t>(key)], false);
    }
}
