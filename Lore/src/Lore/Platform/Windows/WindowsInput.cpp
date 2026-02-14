#include "lrpch.h"

#ifdef LORE_PLATFORM_WINDOWS

#include "WindowsInput.h"

#include "Lore/Application.h"
#include <GLFW/glfw3.h>

namespace Lore {

	Input* Input::s_Instance = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(int keycode) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int buttoncode) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, buttoncode);

		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl() {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float WindowsInput::GetMouseXImpl() {
		auto [x, y] = GetMousePositionImpl();

		return x;
	}

	float WindowsInput::GetMouseYImpl() {
		auto [x, y] = GetMousePositionImpl();

		return y;
	}


}

#endif
