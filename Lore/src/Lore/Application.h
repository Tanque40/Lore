#pragma once

#include "Lore/Core.h"
#include "Lore/Events/Event.h"
#include "Lore/Events/ApplicationEvent.h"
#include "Lore/Window.h"
#include "Lore/LayerStack.h"

namespace Lore {

	class LORE_API Application {
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;

	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

	private:
		bool OnWindowClose(WindowCloseEvent& e);
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}
