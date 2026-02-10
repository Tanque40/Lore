#include "lrpch.h"

#include "Application.h"

#include "Lore/Events/ApplicationEvent.h"
#include "Log.h"

namespace Lore {

	Application::Application() {
	}

	Application::~Application() {
	}

	void Application::Run() {
		WindowResizeEvent e(1280, 720);
		LR_TRACE(e.ToString());

		while (true) {
		}
	}
}
