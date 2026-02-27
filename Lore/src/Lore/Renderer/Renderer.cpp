#include "lrpch.h"

#include "Lore/Renderer/Renderer.h"

namespace Lore {

	Renderer::SceneData* Renderer::s_SceneData = new SceneData();

	void Renderer::BeginScene(OrthographicCamera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader) {
		shader->Bind();
		shader->SetUniformMat4f("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::EndScene() {
	}
}
