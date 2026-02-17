#include "lrpch.h"

#include "Lore/Application.h"
#include "Lore/ImGui/ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#ifdef LORE_PLATFORM_MAC
#include "Lore/ImGui/ImGuiMetalHelper.h"
#endif

#include "Lore/Renderer/RendererAPI.h"

#include <GLFW/glfw3.h>

namespace Lore {

	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {
	}

	ImGuiLayer::~ImGuiLayer() {
	}

	void ImGuiLayer::OnAttach() {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 410");
		}
#ifdef LORE_PLATFORM_MAC
		else if (RendererAPI::GetAPI() == RendererAPIType::Metal) {
			ImGui_ImplGlfw_InitForOther(window, true);
			ImGuiMetalHelper_Init();
		}
#endif
	}

	void ImGuiLayer::OnDetach() {
		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			ImGui_ImplOpenGL3_Shutdown();
		}
#ifdef LORE_PLATFORM_MAC
		else if (RendererAPI::GetAPI() == RendererAPIType::Metal) {
			ImGuiMetalHelper_Shutdown();
		}
#endif
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnImGuiRender() {
		static bool show = true;
		ImGui::ShowDemoWindow(&show);
	}

	void ImGuiLayer::Begin() {
		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			ImGui_ImplOpenGL3_NewFrame();
		}
#ifdef LORE_PLATFORM_MAC
		else if (RendererAPI::GetAPI() == RendererAPIType::Metal) {
			ImGuiMetalHelper_NewFrame();
		}
#endif
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End() {
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();

		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
		io.DisplayFramebufferScale = ImVec2(
			(float)app.GetWindow().GetFramebufferWidth() / (float)app.GetWindow().GetWidth(),
			(float)app.GetWindow().GetFramebufferHeight() / (float)app.GetWindow().GetHeight()
		);

		ImGui::Render();

		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}
#ifdef LORE_PLATFORM_MAC
		else if (RendererAPI::GetAPI() == RendererAPIType::Metal) {
			ImGuiMetalHelper_RenderDrawData();

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		}
#endif
	}

}
