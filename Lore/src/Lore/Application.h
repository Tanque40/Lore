#pragma once

#include "Core.h"

namespace Lore {

	class LORE_API Application {
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}
