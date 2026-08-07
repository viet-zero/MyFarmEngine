#include "Time.h"
#include <GLFW/glfw3.h>
namespace MyEngine
{
	float Time::s_DeltaTime = 0.0f;
	float Time::s_LastFrameTime = 0.0f;
	void Time::Update() {
		float current = static_cast<float>(glfwGetTime());
		s_DeltaTime = current - s_LastFrameTime;
		s_LastFrameTime = current;
	}
	float Time::GetDeltaTime() {
		return s_DeltaTime;
	}
	float Time::GetTime() {
		return static_cast<float>(glfwGetTime());
	}
}