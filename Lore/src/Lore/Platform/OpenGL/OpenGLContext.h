#pragma once

#include "Lore/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Lore {

	class OpenGLContext : public GraphicsContext {
	private:
		GLFWwindow* m_WindowHandle;
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		virtual ~OpenGLContext() = default;

		virtual void Init() override;
		virtual void SwapBuffers() override;
	};

}
