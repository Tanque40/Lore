#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Window.h"
#include "Lore/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Lore {

	class MacWindow : public Window {

	private:
		GLFWwindow* m_Window;

		struct WindowData {
			std::string Title;
			unsigned int Width, Height;
			unsigned int FramebufferWidth, FramebufferHeight;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		GraphicsContext* m_Context;

	public:
		MacWindow(const WindowProps& props);
		virtual ~MacWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }
		inline unsigned int GetFramebufferWidth() const override { return m_Data.FramebufferWidth; }
		inline unsigned int GetFramebufferHeight() const override { return m_Data.FramebufferHeight; }

		// Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline virtual void* GetNativeWindow() const override { return m_Window; }

	private:
		void Init(const WindowProps& props);
		void Shutdown();

	};

}

#endif
