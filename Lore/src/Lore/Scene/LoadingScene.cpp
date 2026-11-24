#include "lrpch.h"

#include "Lore/Scene/LoadingScene.h"

#include "Lore/Renderer/RenderCommand.h"

#include <imgui.h>

namespace Lore {

	void LoadingScene::OnUpdate(TimeStep ts) {
		m_Elapsed += ts.GetSeconds();
		RenderCommand::SetClearColor({ 0.05f, 0.05f, 0.07f, 1.0f });
		RenderCommand::Clear();
	}

	void LoadingScene::OnImGuiRender() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 center(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f, viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(420.0f, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowFocus();

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize;
		ImGui::Begin("##Loading", nullptr, flags);

		ImGui::Text("Cargando %s", m_TargetName.c_str());
		float progress = m_Context ? m_Context->GetProgress() : 0.0f;
		std::string stage = m_Context ? m_Context->GetStage() : std::string();

		char overlay[32];
		std::snprintf(overlay, sizeof(overlay), "%.0f%%", progress * 100.0f);
		ImGui::ProgressBar(progress, ImVec2(-FLT_MIN, 0.0f), overlay);

		// Puntos animados: dejan claro que la app sigue viva aunque una fase tarde.
		int dots = static_cast<int>(m_Elapsed * 3.0f) % 4;
		ImGui::TextDisabled("%s%.*s", stage.c_str(), dots, "...");
		ImGui::TextDisabled("%.1f s", m_Elapsed);

		ImGui::End();
	}

}
