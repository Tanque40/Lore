#include "lrpch.h"

#include "Lore/Scene/SceneManager.h"

#include "Lore/Application.h"

namespace Lore {

	SceneManager::~SceneManager() {
		CancelLoad();
		if (m_Active)
			m_Active->OnDetach();
	}

	void SceneManager::SetScene(std::unique_ptr<Scene> scene) {
		m_RequestKind = RequestKind::Set;
		m_RequestScene = std::move(scene);
		m_RequestLoadingScreen.reset();
	}

	void SceneManager::LoadAsync(std::unique_ptr<Scene> scene, std::unique_ptr<LoadingScene> loadingScreen) {
		if (IsLoading()) {
			LR_CORE_WARN("SceneManager: ya hay una carga en curso, se ignora '{0}'", scene ? scene->GetName() : "?");
			return;
		}
		m_RequestKind = RequestKind::LoadAsync;
		m_RequestScene = std::move(scene);
		m_RequestLoadingScreen = std::move(loadingScreen);
	}

	void SceneManager::SwitchTo(std::unique_ptr<Scene> scene) {
		if (m_Active)
			m_Active->OnDetach();
		m_Active = std::move(scene);
		if (m_Active)
			m_Active->OnAttach();
	}

	void SceneManager::ApplyRequest() {
		RequestKind kind = m_RequestKind;
		m_RequestKind = RequestKind::None;
		if (kind == RequestKind::None || !m_RequestScene)
			return;

		if (kind == RequestKind::Set) {
			CancelLoad();
			LoadContext context;
			m_RequestScene->OnLoad(context);
			SwitchTo(std::move(m_RequestScene));
			return;
		}

		// LoadAsync
		m_Loading = std::move(m_RequestScene);
		m_LoadContext = std::make_unique<LoadContext>();
		m_LoadingScreen = m_RequestLoadingScreen ? std::move(m_RequestLoadingScreen) : std::make_unique<LoadingScene>();
		m_LoadingScreen->Track(m_LoadContext.get(), m_Loading->GetName());
		m_LoadStartTime = static_cast<float>(Application::Get().GetWindow().GetTime());

		// La pantalla de carga pasa a ser la escena activa (y la anterior se libera) ANTES
		// de lanzar el trabajo, así el siguiente frame ya la dibuja.
		SwitchTo(nullptr);
		m_LoadingScreen->OnAttach();

		Scene* target = m_Loading.get();
		LoadContext* context = m_LoadContext.get();
		LR_CORE_INFO("SceneManager: cargando '{0}' en segundo plano", target->GetName());
		m_LoadJob = JobSystem::Submit([target, context]() {
			target->OnLoad(*context);
			context->SetProgress(1.0f);
			});
	}

	void SceneManager::FinishLoadIfReady() {
		if (!m_Loading || !m_LoadJob.IsDone())
			return;

		m_LoadJob = JobHandle();
		m_LoadingScreen->OnDetach();
		m_LoadingScreen.reset();
		m_LoadContext.reset();

		// OnAttach (subidas a GPU) corre en el hilo principal.
		SwitchTo(std::move(m_Loading));

		m_LastLoadSeconds = static_cast<float>(Application::Get().GetWindow().GetTime()) - m_LoadStartTime;
		LR_CORE_INFO("SceneManager: '{0}' lista en {1:.2f} s", m_Active->GetName(), m_LastLoadSeconds);
	}

	void SceneManager::CancelLoad() {
		if (!m_Loading)
			return;
		m_LoadContext->Cancel();
		m_LoadJob.Wait();
		m_LoadJob = JobHandle();
		m_Loading.reset();
		if (m_LoadingScreen)
			m_LoadingScreen->OnDetach();
		m_LoadingScreen.reset();
		m_LoadContext.reset();
	}

	void SceneManager::OnUpdate(TimeStep ts) {
		ApplyRequest();
		FinishLoadIfReady();

		if (m_LoadingScreen)
			m_LoadingScreen->OnUpdate(ts);
		else if (m_Active)
			m_Active->OnUpdate(ts);
	}

	void SceneManager::OnImGuiRender() {
		if (m_LoadingScreen)
			m_LoadingScreen->OnImGuiRender();
		else if (m_Active)
			m_Active->OnImGuiRender();
	}

	void SceneManager::OnEvent(Event& event) {
		if (m_LoadingScreen)
			m_LoadingScreen->OnEvent(event);
		else if (m_Active)
			m_Active->OnEvent(event);
	}

}
