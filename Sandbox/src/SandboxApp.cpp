#include <Lore.h>

#include <imgui.h>
#include <string>

#include "Maze/Algorithms/BinaryTree.h"

#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Lore::Layer {
private:
	Maze::Grid m_Grid{ 50, 50 };
	std::string m_GridString = "";

	// Compute-First pipeline resources
	std::shared_ptr<Lore::ComputeShader> m_ComputeShader;
	std::shared_ptr<Lore::ComputeTexture> m_ComputeTexture;

	uint32_t m_Width = 800;
	uint32_t m_Height = 600;

public:
	ExampleLayer() : Layer("Example") {
#ifdef LORE_PLATFORM_MAC
		m_ComputeShader.reset(Lore::ComputeShader::Create("Sandbox/src/Shaders/Metal/Main.compute.metal"));
#elif defined(LORE_PLATFORM_WINDOWS)
		m_ComputeShader.reset(Lore::ComputeShader::Create("src/Shaders/OpenGL/Main.compute.glsl"));
#endif
		m_ComputeTexture.reset(Lore::ComputeTexture::Create(m_Width, m_Height));
	}

	~ExampleLayer() {
		m_ComputeShader.reset();
		m_ComputeTexture.reset();
	}

	void OnAttach() override {
		m_Grid = Maze::BinaryTree::On(m_Grid);
		m_GridString = m_Grid.ToString();
	}

	void OnUpdate(Lore::TimeStep ts) override {
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
		ImGui::Begin("Compute-First Demo");
		ImGui::Text("Resolution: %dx%d", m_Width, m_Height);
		ImGui::Text("%s", m_GridString.c_str());
		ImGui::End();
	}

	void OnEvent(Lore::Event& event) override {
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
