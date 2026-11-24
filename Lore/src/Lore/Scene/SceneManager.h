#pragma once

#include "Lore/Layer.h"
#include "Lore/Core/JobSystem.h"
#include "Lore/Scene/Scene.h"
#include "Lore/Scene/LoadingScene.h"

namespace Lore {

	// Layer que tiene la escena activa y hace las transiciones. Reemplaza al "GameLayer
	// único": el juego (Sandbox) sólo define escenas y le pide al manager que las cargue.
	//
	// Las peticiones (SetScene / LoadAsync) se aplican al inicio del siguiente OnUpdate, así
	// que es seguro llamarlas desde dentro de la escena activa (p. ej. desde un botón de
	// ImGui): la escena que llama no se destruye mientras su propio código sigue corriendo.
	class SceneManager : public Layer {
	private:
		std::unique_ptr<Scene> m_Active;

		// Escena que se está cargando en segundo plano y su pantalla de carga.
		std::unique_ptr<Scene> m_Loading;
		std::unique_ptr<LoadingScene> m_LoadingScreen;
		std::unique_ptr<LoadContext> m_LoadContext;
		JobHandle m_LoadJob;
		float m_LoadStartTime = 0.0f;

		// Petición pendiente para el siguiente frame.
		enum class RequestKind { None, Set, LoadAsync };
		RequestKind m_RequestKind = RequestKind::None;
		std::unique_ptr<Scene> m_RequestScene;
		std::unique_ptr<LoadingScene> m_RequestLoadingScreen;

		float m_LastLoadSeconds = 0.0f;

	public:
		SceneManager() : Layer("SceneManager") {}
		~SceneManager() override;

		// Cambia de escena llamando OnLoad en el hilo principal (para escenas ligeras).
		void SetScene(std::unique_ptr<Scene> scene);

		// Muestra la pantalla de carga de inmediato, corre scene->OnLoad en el JobSystem y,
		// cuando termina, llama OnAttach en el hilo principal y la activa. La escena activa
		// se descarta al empezar la carga (sus recursos de GPU no se necesitan mientras se
		// ve la pantalla de carga). Si ya hay una carga en curso, la petición se ignora.
		void LoadAsync(std::unique_ptr<Scene> scene, std::unique_ptr<LoadingScene> loadingScreen = nullptr);

		template<typename T, typename... Args>
		void LoadAsync(Args&&... args) {
			LoadAsync(std::make_unique<T>(std::forward<Args>(args)...));
		}

		bool IsLoading() const { return m_Loading != nullptr; }
		Scene* GetActiveScene() const { return m_Active.get(); }
		// Duración de la última carga asíncrona, de la petición a la activación.
		float GetLastLoadSeconds() const { return m_LastLoadSeconds; }

		void OnUpdate(TimeStep ts) override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:
		void ApplyRequest();
		void FinishLoadIfReady();
		void SwitchTo(std::unique_ptr<Scene> scene);
		void CancelLoad();
	};

}
