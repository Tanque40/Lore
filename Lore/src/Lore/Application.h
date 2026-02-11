#pragma once

#include "Lore/Core.h"
#include "Lore/Events/Event.h"
#include "Lore/Window.h"

namespace Lore {

	class LORE_API Application {
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;

	public:
		Application();
		virtual ~Application();

		void Run();

	};

	// To be defined in CLIENT
	Application* CreateApplication();
}
