#pragma once

#include "Lore/Renderer/RendererAPI.h"
#include <glm/glm.hpp>

namespace Lore {

	class OpenGLRendererAPI : public RendererAPI {
	public:
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
	};

}
