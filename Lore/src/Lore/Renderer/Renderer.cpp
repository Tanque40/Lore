#include "lrpch.h"

#include "Lore/Renderer/Renderer.h"

namespace Lore {

	void Renderer::BeginScene() {

	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray) {
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::EndScene() {
	}
}
