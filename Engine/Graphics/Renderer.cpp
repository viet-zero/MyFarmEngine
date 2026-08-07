#include "Renderer.h"

#include <glad/gl.h>
namespace MyEngine
{
	bool Renderer::Initialize() {
		glViewport(0,0,1280,720);
		return true;
	}
	void Renderer::BeginFrame() {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	void Renderer::EndFrame()
	{
	}

	void Renderer::Shutdown()
	{
	}

}