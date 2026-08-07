#pragma once

#include "Window.h"
#include "Graphics/Renderer.h"
namespace MyEngine
{
	class Application
	{
	public:
		Application();
		~Application();

		bool Initialize();
		void Run();
		void Shutdown();
	private:
		Window m_Window;
		Renderer m_Renderer;
	};
}