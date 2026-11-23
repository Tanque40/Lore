#pragma once

#include "Lore/Core/TimeStep.h"
#include "Lore/Events/Event.h"

#include "Lore/Scene/LoadContext.h"

namespace Lore {

	// Una escena es dueña de un "estado" completo del juego (menú, carga, nivel...). El
	// SceneManager tiene una activa a la vez y le reenvía el loop de la aplicación.
	//
	// Ciclo de vida:
	//   OnLoad(ctx)  -> en un hilo del JobSystem si se cargó con LoadAsync. Sólo CPU:
	//                   generar datos, leer archivos. NADA de la API gráfica.
	//   OnAttach()   -> en el hilo principal, justo antes de activarse. Aquí se crean y
	//                   llenan los recursos de GPU con lo que preparó OnLoad.
	//   OnUpdate / OnImGuiRender / OnEvent mientras está activa.
	//   OnDetach()   -> en el hilo principal, al salir. Después se destruye.
	class Scene {
	protected:
		std::string m_Name;

	public:
		explicit Scene(std::string name = "Scene") : m_Name(std::move(name)) {}
		virtual ~Scene() = default;

		virtual void OnLoad(LoadContext& context) {}
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(TimeStep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		const std::string& GetName() const { return m_Name; }
	};

}
