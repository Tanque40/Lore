#pragma once

#include <glm/glm.hpp>

#include "Lore/Renderer/VertexArray.h"

namespace Lore {

	class RendererAPI {
	public:
		enum class API {
			None = 0, OpenGL = 1, Metal = 2
		};

	private:
		static API s_API;

	public:
		virtual ~RendererAPI() = default;

		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;

		inline static API GetAPI() { return s_API; }
	};

}
