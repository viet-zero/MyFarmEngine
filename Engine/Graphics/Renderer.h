#pragma once
namespace MyEngine
{
	class Renderer {
	public:
		bool Initialize();
		void BeginFrame();
		void EndFrame();
		void Shutdown();
	};
}