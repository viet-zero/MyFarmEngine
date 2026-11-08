#pragma once

struct GLFWwindow;
namespace MyEngine {
	class Input {
	public:
		static void Initialize(GLFWwindow* window);
		static bool IsKeyPressed(int key);
        // Consume a press event, including taps released before the next Update.
        static bool ConsumeKeyPress(int key);
        static void ClearPresses();
        static bool ConsumeMousePress(int button);
	private:
		static GLFWwindow* s_Window;
	};
}
