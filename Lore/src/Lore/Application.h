#pragma once

#include "Lore/Events/Event.h"
#include "Lore/Events/ApplicationEvent.h"
#include "Lore/Window.h"
#include "Lore/LayerStack.h"

#include "Lore/Core/TimeStep.h"

#include "Lore/ImGui/ImGuiLayer.h"

namespace Lore {

	class  Application {
	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		TimeStep m_TimeStep;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}
