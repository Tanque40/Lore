#include "lrpch.h"

#include "Lore/Application.h"
#include "Lore/ImGui/ImGuiLayer.h"
#include "Lore/Renderer/RendererAPI.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

#ifdef LORE_PLATFORM_WINDOWS
#include <backends/imgui_impl_opengl3.h>
#endif

#ifdef LORE_PLATFORM_MAC
#include "Lore/ImGui/ImGuiMetalBridge.h"
#include "Lore/Platform/Metal/MetalContext.h"
#endif

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
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Multi-viewport not yet supported with Metal backend

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

#ifdef LORE_PLATFORM_MAC
		if (RendererAPI::GetAPI() == RendererAPI::API::Metal) {
			ImGui_ImplGlfw_InitForOther(window, true);
			MetalContext* ctx = MetalContext::Get();
			ImGuiMetalBridge::Init(ctx->GetDevice());
		}
#endif

#ifdef LORE_PLATFORM_WINDOWS
		if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL) {
			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 410");
		}
#endif
	}

	void ImGuiLayer::OnDetach() {
#ifdef LORE_PLATFORM_MAC
		if (RendererAPI::GetAPI() == RendererAPI::API::Metal) {
			ImGuiMetalBridge::Shutdown();
		}
#endif
#ifdef LORE_PLATFORM_WINDOWS
		if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL) {
			ImGui_ImplOpenGL3_Shutdown();
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
#ifdef LORE_PLATFORM_MAC
		if (RendererAPI::GetAPI() == RendererAPI::API::Metal) {
			MetalContext* ctx = MetalContext::Get();
			ImGuiMetalBridge::NewFrame(ctx->GetCurrentPassDescriptor());
		}
#endif
#ifdef LORE_PLATFORM_WINDOWS
		if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL) {
			ImGui_ImplOpenGL3_NewFrame();
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

#ifdef LORE_PLATFORM_MAC
		if (RendererAPI::GetAPI() == RendererAPI::API::Metal) {
			MetalContext* ctx = MetalContext::Get();
			ImGuiMetalBridge::RenderDrawData(ctx->GetCurrentCommandBuffer(), ctx->GetCurrentEncoder());
		}
#endif

#ifdef LORE_PLATFORM_WINDOWS
		if (RendererAPI::GetAPI() == RendererAPI::API::OpenGL) {
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}
#endif
	}

}
