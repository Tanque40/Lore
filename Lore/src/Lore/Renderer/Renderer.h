#pragma once

#include "Lore/Renderer/RendererAPI.h"
#include "Lore/Renderer/RenderCommand.h"
#include "Lore/Renderer/VertexArray.h"

namespace Lore {

	class Renderer {
	public:
		static void BeginScene();
		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);
		static void EndScene();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};

}
