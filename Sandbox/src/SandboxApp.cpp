#include <Lore.h>

#include <imgui.h>
#include <string>

#include "Maze/Algorithms/BinaryTree.h"

#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Lore::Layer {
private:
	// For calculating FPS
	float m_FPS = 0.0f;
	float m_FrameTime = 0.0f;

	uint32_t m_GridDimension = 10;
	Maze::Grid m_Grid;
	std::string m_GridString = "";

	// Compute-First pipeline resources
	std::shared_ptr<Lore::ComputeShader> m_ComputeShader;
	std::shared_ptr<Lore::ComputeTexture> m_ComputeTexture;

	uint32_t m_Width = 800;
	uint32_t m_Height = 600;

public:
	ExampleLayer() : Layer("Example"), m_Grid(m_GridDimension, m_GridDimension) {
		// Initialize compute shader and texture
#ifdef LORE_PLATFORM_MAC
		m_ComputeShader.reset(Lore::ComputeShader::Create("Sandbox/src/Shaders/Metal/Main.compute.metal"));
#elif defined(LORE_PLATFORM_WINDOWS)
		m_ComputeShader.reset(Lore::ComputeShader::Create("src/Shaders/OpenGL/Main.compute.glsl"));
#endif
		m_Width = Lore::Application::Get().GetWindow().GetWidth();
		m_Height = Lore::Application::Get().GetWindow().GetHeight();
		m_ComputeTexture.reset(Lore::ComputeTexture::Create(m_Width, m_Height));
	}

	~ExampleLayer() {
		m_ComputeShader.reset();
		m_ComputeTexture.reset();
	}

	void OnAttach() override {
	}

	void OnUpdate(Lore::TimeStep ts) override {
		// Update FPS and frame time
		m_FrameTime = ts.GetSeconds();
		m_FPS = 1.0f / m_FrameTime;

		// 1. Bind compute resources
		m_ComputeShader->Bind();
		m_ComputeTexture->BindAsImage(0);

		// 2. Dispatch compute shader
		uint32_t groupsX = (m_Width + 15) / 16;
		uint32_t groupsY = (m_Height + 15) / 16;
		Lore::RenderCommand::DispatchCompute(groupsX, groupsY, 1);

		// 3. Memory barrier (ensures compute writes are visible)
		Lore::RenderCommand::ComputeBarrier();

		// 4. Clear and blit the compute texture to screen
		Lore::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		Lore::RenderCommand::Clear();
		Lore::Renderer::BlitToScreen(m_ComputeTexture);
	}

	void OnImGuiRender() override {
		Lore::Application& app = Lore::Application::Get();
		Lore::FramebufferSpecification spec = app.GetFramebuffer().GetSpecification();
		ImGui::Begin("Compute-First Demo");

		if (!ImGui::CollapsingHeader("Stats")) {
			ImGui::BeginChild("Stats Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
			ImGui::Text("Resolution: %dx%d", m_Width, m_Height);
			ImGui::Text("Framebuffer Size: %dx%d", spec.Width, spec.Height);
			ImGui::Text("FPS: %.2f (%.2f ms)", m_FPS, m_FrameTime * 1000.0f);
			ImGui::EndChild();
		}

		if (!ImGui::CollapsingHeader("Maze")) {

			ImGui::Text("%s", m_GridString.c_str());
		}

		ImGui::End();
	}

	void OnEvent(Lore::Event& event) override {
		Lore::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Lore::WindowResizeEvent>(LR_BIND_EVENT_FN(ExampleLayer::WindowResize));
	}

	bool WindowResize(Lore::WindowResizeEvent& e) {
		m_Width = e.GetWidth();
		m_Height = e.GetHeight();
		m_ComputeTexture->Resize(m_Width, m_Height);
		return false;
	}
};

class Sandbox : public Lore::Application {

public:
	Sandbox() {
		PushLayer(new ExampleLayer());
	}
	~Sandbox() {}

};


Lore::Application* Lore::CreateApplication() {
	return new Sandbox();
}
