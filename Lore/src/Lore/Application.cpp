#include "lrpch.h"

#include "Application.h"

#include "Lore/Input.h"

#include "Lore/Renderer/Renderer.h"
#include "Lore/Renderer/RenderCommand.h"
#include "Lore/Renderer/RendererAPI.h"

namespace Lore {

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application() {
		LR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// Create Vertex Array
		m_VertexArray = VertexArray::Create();

		float vertices[4 * 3]{
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			-0.5f, 0.5f, 0.0f,
			 0.5f, 0.5f, 0.0f
		};

		VertexBuffer* vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
			});
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[6] = { 0, 1, 2, 2, 1, 3 };
		IndexBuffer* indexBuffer = IndexBuffer::Create(indices, 6);
		m_VertexArray->SetIndexBuffer(indexBuffer);

		// Create shaders based on API
		if (RendererAPI::GetAPI() == RendererAPIType::OpenGL) {
			std::string vertexSrc = R"(
				#version 410 core

				layout(location = 0) in vec3 a_Position;

				out vec3 v_Color;

				void main() {
					gl_Position = vec4(a_Position, 1.0);
					v_Color = vec3(0.8, 0.2, 0.3);
				}
			)";

			std::string fragmentSrc = R"(
				#version 410 core

				in vec3 v_Color;

				out vec4 color;

				void main() {
					color = vec4(v_Color, 1.0);
				}
			)";

			m_Shader = Shader::Create(vertexSrc, fragmentSrc);
		}
		else if (RendererAPI::GetAPI() == RendererAPIType::Metal) {
			// Metal uses a single shader source with both vertex and fragment functions
			std::string vertexSrc = R"(
				#include <metal_stdlib>
				using namespace metal;

				struct VertexIn {
					float3 position [[attribute(0)]];
				};

				struct VertexOut {
					float4 position [[position]];
					float3 color;
				};

				vertex VertexOut vertexShader(VertexIn in [[stage_in]]) {
					VertexOut out;
					out.position = float4(in.position, 1.0);
					out.color = float3(0.8, 0.2, 0.3);
					return out;
				}

				fragment float4 fragmentShader(VertexOut in [[stage_in]]) {
					return float4(in.color, 1.0);
				}
			)";

			// In Metal, both functions are in one source; pass empty string for fragment
			m_Shader = Shader::Create(vertexSrc, "");
		}
	}

	Application::~Application() {
		delete m_Shader;
		delete m_VertexArray;
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay) {
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.m_Handled) {
				break;
			}
		}
	}

	void Application::Run() {
		while (m_Running) {

			RenderCommand::SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			RenderCommand::Clear();

			Renderer::BeginScene();

			if (m_Shader && m_VertexArray)
				Renderer::Submit(m_Shader, m_VertexArray);

			Renderer::EndScene();

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
}
