#include "lrpch.h"

#ifdef LORE_PLATFORM_WINDOWS

#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Lore {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle) {
		LR_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init() {
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		LR_CORE_ASSERT(status, "Failed to initialize glad");

		char* vendor = (char*)glGetString(GL_VENDOR);
		char* renderer = (char*)glGetString(GL_RENDERER);
		char* version = (char*)glGetString(GL_VERSION);

		LR_CORE_INFO("OpenGL Info:");
		LR_CORE_INFO("\tVendor: {0}", vendor);
		LR_CORE_INFO("\tRenderer: {0}", renderer);
		LR_CORE_INFO("\tVersion: {0}", version);
	}

	void OpenGLContext::SwapBuffers() {
		glfwSwapBuffers(m_WindowHandle);
	}

}

#endif // LORE_PLATFORM_WINDOWS
