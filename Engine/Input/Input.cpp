#include "Input.h"
#include <GLFW/glfw3.h>

namespace MyEngine
{
    GLFWwindow* Input::s_Window = nullptr;
    void Input::Initialize(GLFWwindow* window) { s_Window = window; }
    bool Input::IsKeyPressed(int key)
    {
        return s_Window && glfwGetKey(s_Window, key) == GLFW_PRESS;
    }
}
