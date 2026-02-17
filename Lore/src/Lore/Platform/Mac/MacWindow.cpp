#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MacWindow.h"

#include "Lore/Core.h"

#include "Lore/Events/Event.h"
#include "Lore/Events/ApplicationEvent.h"
#include "Lore/Events/MouseEvent.h"
#include "Lore/Events/KeyEvent.h"

#include <GLFW/glfw3.h>

#include "Lore/Renderer/RendererAPI.h"
#include "Lore/Platform/OpenGL/OpenGLContext.h"
#include "Lore/Platform/Metal/MetalContext.h"

namespace Lore {

	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* description) {
		LR_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window* Window::Create(const WindowProps& props) {
		return new MacWindow(props);
	}

	MacWindow::MacWindow(const WindowProps& props) {
		Init(props);
	}

	MacWindow::~MacWindow() {
		Shutdown();
	}

	void MacWindow::Init(const WindowProps& props) {
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		LR_CORE_INFO("Creating mac window: {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized) {
			int success = glfwInit();
			LR_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#ifdef __APPLE__
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
		}
		else if (RendererAPI::GetAPI() == RendererAPIType::Metal) {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			m_Context = new OpenGLContext(m_Window);
		}
		else if (RendererAPI::GetAPI() == RendererAPIType::Metal) {
			m_Context = new MetalContext(m_Window);
		}
		m_Context->Init();
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;
			glfwGetFramebufferSize(window, (int*)&data.FramebufferWidth, (int*)&data.FramebufferHeight);

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, 1);
				data.EventCallback(event);
				break;
			}
			}
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}

			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}

			default:
				break;
			}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
			});

		glfwGetFramebufferSize(m_Window, (int*)&m_Data.FramebufferWidth, (int*)&m_Data.FramebufferHeight);
	}

	void MacWindow::Shutdown() {
		glfwDestroyWindow(m_Window);
	}

	void MacWindow::OnUpdate() {
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void MacWindow::SetVSync(bool enabled) {
		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			if (enabled)
				glfwSwapInterval(1);
			else
				glfwSwapInterval(0);
		}
		// Metal VSync is controlled by CAMetalLayer.displaySyncEnabled (enabled by default)

		m_Data.VSync = enabled;
	}

	bool MacWindow::IsVSync() const {
		return m_Data.VSync;
	}

}

#endif
