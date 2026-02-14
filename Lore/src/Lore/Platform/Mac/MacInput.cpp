#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MacInput.h"

#include "Lore/Application.h"
#include <GLFW/glfw3.h>

namespace Lore {

	Input* Input::s_Instance = new MacInput();

	bool MacInput::IsKeyPressedImpl(int keycode) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool MacInput::IsMouseButtonPressedImpl(int buttoncode) {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, buttoncode);

		return state == GLFW_PRESS;
	}

	std::pair<float, float> MacInput::GetMousePositionImpl() {
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float MacInput::GetMouseXImpl() {
		auto [x, y] = GetMousePositionImpl();

		return x;
	}

	float MacInput::GetMouseYImpl() {
		auto [x, y] = GetMousePositionImpl();

		return y;
	}


}

#endif
