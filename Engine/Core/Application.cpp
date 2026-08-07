#include "Application.h"
#include "Time.h"
#include <Input/Input.h>

#include <GLFW/glfw3.h>
namespace MyEngine
{
	Application::Application() = default;
	Application::~Application() = default;

	bool Application::Initialize() {
		if (!m_Window.Create(1280, 720, "MyFarmEngine")) {
			return false;
			}
		Input::Initialize(m_Window.GetNativeWindow());
		if (!m_Renderer.Initialize())
			return false;
		return true;
	}
	void Application::Run() {
		while (!m_Window.ShouldClose()) {
			Time::Update();
			if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
			{
				break;
			}
			m_Renderer.BeginFrame();
			m_Window.PollEvents();
			m_Window.SwapBuffers();
			m_Renderer.EndFrame();
		}
	}
	void Application::Shutdown() {
		m_Renderer.Shutdown();
		m_Window.Destroy();
	}
}