#pragma once

struct GLFWwindow;
namespace MyEngine {
	class Input {
	public:
		static void Initialize(GLFWwindow* window);
		static bool IsKeyPressed(int key);
	private:
		static GLFWwindow* s_Window;
	};
}