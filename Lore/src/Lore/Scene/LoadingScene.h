#pragma once

#include "Lore/Scene/Scene.h"

namespace Lore {

	// Pantalla de carga por defecto: limpia el viewport y muestra una barra de progreso con
	// la fase actual. El SceneManager le pasa el LoadContext de la escena que se está
	// cargando. Se puede heredar para una pantalla de carga propia.
	class LoadingScene : public Scene {
	protected:
		const LoadContext* m_Context = nullptr;
		std::string m_TargetName;
		float m_Elapsed = 0.0f;

	public:
		LoadingScene() : Scene("Loading") {}

		void Track(const LoadContext* context, const std::string& targetName) {
			m_Context = context;
			m_TargetName = targetName;
		}

		void OnUpdate(TimeStep ts) override;
		void OnImGuiRender() override;
	};

}
