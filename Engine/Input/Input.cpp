#include "Input.h"
#include <GLFW/glfw3.h>
#include <array>
#include <utility>
namespace { std::array<bool,GLFW_KEY_LAST+1> keyPresses{}; std::array<bool,GLFW_MOUSE_BUTTON_LAST+1> mousePresses{}; }
namespace MyEngine
{
    GLFWwindow* Input::s_Window=nullptr;
    void Input::Initialize(GLFWwindow* window)
    {
        if(s_Window)
        {
            glfwSetKeyCallback(s_Window,nullptr);
            glfwSetMouseButtonCallback(s_Window,nullptr);
            glfwSetWindowFocusCallback(s_Window,nullptr);
        }
        s_Window=window;ClearPresses();
        if(s_Window)
        {
            glfwSetWindowFocusCallback(s_Window,[](GLFWwindow*,int focused){if(!focused) Input::ClearPresses();});
            glfwSetMouseButtonCallback(s_Window,[](GLFWwindow*,int button,int action,int){
                if(action==GLFW_PRESS && button>=0 && button<=GLFW_MOUSE_BUTTON_LAST)
                    mousePresses[static_cast<std::size_t>(button)]=true;
            });
            glfwSetKeyCallback(s_Window,[](GLFWwindow*,int key,int,int action,int)
            {
                if(action==GLFW_PRESS && key>=0 && key<=GLFW_KEY_LAST) keyPresses[static_cast<std::size_t>(key)]=true;
            });
        }
    }
    void Input::ClearPresses() { keyPresses.fill(false);mousePresses.fill(false); }
    bool Input::ConsumeMousePress(int button)
    {
        if(!s_Window || button<0 || button>GLFW_MOUSE_BUTTON_LAST) return false;
        const bool pending=std::exchange(mousePresses[static_cast<std::size_t>(button)],false);
        return glfwGetWindowAttrib(s_Window,GLFW_FOCUSED) && pending;
    }
    bool Input::IsKeyPressed(int key)
    {
        return s_Window && glfwGetWindowAttrib(s_Window,GLFW_FOCUSED) && key>=GLFW_KEY_SPACE && key<=GLFW_KEY_LAST &&
               glfwGetKey(s_Window,key)==GLFW_PRESS;
    }
    bool Input::ConsumeKeyPress(int key)
    {
        if(!s_Window || key<0 || key>GLFW_KEY_LAST) return false;
        const bool pending=std::exchange(keyPresses[static_cast<std::size_t>(key)],false);
        return glfwGetWindowAttrib(s_Window,GLFW_FOCUSED) && pending;
    }
}
