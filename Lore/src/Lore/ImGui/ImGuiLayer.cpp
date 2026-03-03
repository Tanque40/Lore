#include "lrpch.h"

#include "Lore/Application.h"
#include "Lore/ImGui/ImGuiLayer.h"
#include "Lore/Renderer/RendererAPI.h"
#include "Lore/Renderer/Framebuffer.h"

#include <imgui.h>
#include <imgui_internal.h>
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

		// Render the viewport panel showing the framebuffer content
		RenderViewport();
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

		// Start the dockspace
		BeginDockspace();
	}

	void ImGuiLayer::End() {
		// End the dockspace
		EndDockspace();

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

	void ImGuiLayer::BeginDockspace() {
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoBackground;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		// Setup default layout on first run
		if (!m_DockspaceInitialized) {
			m_DockspaceInitialized = true;

			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

			ImGuiID dock_left, dock_right;
			ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, &dock_left, &dock_right);

			ImGui::DockBuilderDockWindow("SandBox Test", dock_left);
			ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_left);
			ImGui::DockBuilderDockWindow("Viewport", dock_right);

			ImGui::DockBuilderFinish(dockspace_id);
		}
	}

	void ImGuiLayer::EndDockspace() {
		ImGui::End(); // End the DockSpace window
	}

	void ImGuiLayer::RenderViewport() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		Application& app = Application::Get();
		Framebuffer& fb = app.GetFramebuffer();
		const auto& spec = fb.GetSpecification();

		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0 &&
			((uint32_t)viewportPanelSize.x != spec.Width || (uint32_t)viewportPanelSize.y != spec.Height)) {
			fb.Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
		}

		void* texID = fb.GetColorAttachmentRendererID();
		if (texID) {
#ifdef LORE_PLATFORM_WINDOWS
			// OpenGL: flip Y because textures are bottom-up
			ImGui::Image(texID, viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
#elif defined(LORE_PLATFORM_MAC)
			// Metal: textures are top-down, no flip needed
			ImGui::Image(texID, viewportPanelSize);
#endif
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

}
