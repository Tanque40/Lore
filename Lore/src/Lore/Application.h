#pragma once

#include "Lore/Core.h"
#include "Lore/Events/Event.h"
#include "Lore/Events/ApplicationEvent.h"
#include "Lore/Window.h"
#include "Lore/LayerStack.h"

#include "Lore/ImGui/ImGuiLayer.h"

#include "Lore/Renderer/Shader.h"

namespace Lore {

	class  Application {
	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		static Application* s_Instance;

		unsigned int m_VertexArray, m_VertexBuffer, m_IndexBuffer;
		Shader* m_Shader;
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
