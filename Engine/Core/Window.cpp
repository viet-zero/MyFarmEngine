#include "Window.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace MyEngine
{
	bool Window::Create(unsigned int width,
						unsigned int height,
						const char* title)
	{
		if (!glfwInit())
		{
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		m_Window = glfwCreateWindow(
			width,
			height,
			title,
			nullptr,
			nullptr);
		if (m_Window = nullptr)
		{
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(m_Window);
		if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) 
		{
			glfwDestroyWindow(m_Window);
			glfwTerminate();
			return false;
		}
		return true;
	}

	void Window::Destroy()
	{
		if (m_Window) {
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
		}
		glfwTerminate();
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	void Window::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(m_Window);
	}
	GLFWwindow* Window::GetNativeWindow() const
	{
		return m_Window;
	}
}