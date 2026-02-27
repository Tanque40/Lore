#include <Lore.h>

#include <imgui.h>
#include <string>

#include "Maze/Algorithms/BinaryTree.h"

class ExampleLayer : public Lore::Layer {
private:
	Maze::Grid m_Grid{ 10, 10 };
	std::string m_GridString = "";

	std::shared_ptr<Lore::Shader> m_Shader;
	std::shared_ptr<Lore::VertexArray> m_VertexArray;
	std::shared_ptr<Lore::VertexBuffer> m_VertexBuffer;
	std::shared_ptr<Lore::IndexBuffer> m_IndexBuffer;

	Lore::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition{ 0.0f, 0.0f, 0.0f };
	float m_CameraSpeed = 0.05f;
	float m_CameraRotationSpeed = 1.0f;

public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f) {
		m_VertexArray.reset(Lore::VertexArray::Create());

		float vertices[3 * 7]{
			-0.5f, -0.5f, 0.0f,  0.8, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f,  0.8, 0.2f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f,  0.8, 0.2f, 0.8f, 1.0f
		};

		m_VertexBuffer.reset(Lore::VertexBuffer::Create(vertices, sizeof(vertices)));

		Lore::BufferLayout layout = {
			{ Lore::ShaderDataType::Float3, "a_Position" },
			{ Lore::ShaderDataType::Float4, "a_Color" },
		};

		m_VertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		unsigned int indices[3] = {
			0, 1, 2
		};
		m_IndexBuffer.reset(Lore::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		std::string vertexSrc = R"(
			#version 410 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec4 v_Color;

			void main() {
				gl_Position =  u_ViewProjection * vec4(a_Position, 1.0);
				v_Color = a_Color;
			}
		)";

		std::string fragmentSrc = R"(
			#version 410 core

			in vec4 v_Color;

			out vec4 color;

			void main() {
				color = v_Color;
			}
		)";

		m_Shader.reset(new Lore::Shader(vertexSrc, fragmentSrc));
	}

	~ExampleLayer() {
		m_VertexArray.reset();
		m_VertexBuffer.reset();
		m_IndexBuffer.reset();
		m_Shader.reset();
	}

	void OnAttach() override {
		m_Grid = Maze::BinaryTree::On(m_Grid);
		m_GridString = m_Grid.ToString();
		//LR_TRACE("\n{}", m_GridString);
	}

	void OnUpdate() override {
		if (Lore::Input::IsKeyPressed(LR_KEY_LEFT))
			m_CameraPosition.x -= m_CameraSpeed;
		if (Lore::Input::IsKeyPressed(LR_KEY_RIGHT))
			m_CameraPosition.x += m_CameraSpeed;
		if (Lore::Input::IsKeyPressed(LR_KEY_UP))
			m_CameraPosition.y += m_CameraSpeed;
		if (Lore::Input::IsKeyPressed(LR_KEY_DOWN))
			m_CameraPosition.y -= m_CameraSpeed;

		if (Lore::Input::IsKeyPressed(LR_KEY_Q))
			m_Camera.SetRotation(m_Camera.GetRotation() + m_CameraRotationSpeed);
		if (Lore::Input::IsKeyPressed(LR_KEY_E))
			m_Camera.SetRotation(m_Camera.GetRotation() - m_CameraRotationSpeed);

		Lore::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Lore::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);

		Lore::Renderer::BeginScene(m_Camera);

		Lore::Renderer::Submit(m_VertexArray, m_Shader);

		Lore::Renderer::EndScene();
	}

	void OnImGuiRender() override {
		ImGui::Begin("SandBox Test");
		ImGui::Text("%s", m_GridString.c_str());
		ImGui::End();
	}

	void OnEvent(Lore::Event& event) override {
		Lore::EventDispatcher dispatcher(event);
		//dispatcher.Dispatch<Lore::KeyPressedEvent>(LR_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));

	}

	bool OnKeyPressed(Lore::KeyPressedEvent& event) {
		if (event.GetKeyCode() == LR_KEY_LEFT)
			m_CameraPosition.x -= m_CameraSpeed;

		if (event.GetKeyCode() == LR_KEY_RIGHT)
			m_CameraPosition.x += m_CameraSpeed;

		if (event.GetKeyCode() == LR_KEY_UP)
			m_CameraPosition.y += m_CameraSpeed;

		if (event.GetKeyCode() == LR_KEY_DOWN)
			m_CameraPosition.y -= m_CameraSpeed;

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
