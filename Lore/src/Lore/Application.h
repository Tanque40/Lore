#pragma once

#include "Lore/Core.h"
#include "Lore/Events/Event.h"
#include "Lore/Events/ApplicationEvent.h"
#include "Lore/Window.h"
#include "Lore/LayerStack.h"

#include "Lore/ImGui/ImGuiLayer.h"

#include "Lore/Renderer/Shader.h"
#include "Lore/Renderer/Buffer.h"
#include "Lore/Renderer/VertexArray.h"

namespace Lore {

	class  Application {
	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		static Application* s_Instance;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
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
