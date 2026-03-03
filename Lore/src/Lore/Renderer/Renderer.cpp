#include "lrpch.h"

#include "Lore/Renderer/Renderer.h"
#include "Lore/Renderer/RenderCommand.h"

namespace Lore {

	Renderer::SceneData* Renderer::s_SceneData = new SceneData();

	void Renderer::BeginScene(OrthographicCamera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader, const glm::mat4& transform) {
		shader->Bind();
		shader->SetUniformMat4f("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		shader->SetUniformMat4f("u_Transform", transform);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::EndScene() {
	}

	void Renderer::BlitToScreen(const std::shared_ptr<ComputeTexture>& texture) {
		RenderCommand::BlitToScreen(texture->GetNativeHandle(), texture->GetWidth(), texture->GetHeight());
	}
}
