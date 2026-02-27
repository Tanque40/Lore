#pragma once

#include "Lore/Renderer/RendererAPI.h"
#include "Lore/Renderer/RenderCommand.h"
#include "Lore/Renderer/VertexArray.h"
#include "Lore/Renderer/OrthographicCamera.h"
#include "Lore/Renderer/Shader.h"

namespace Lore {

	class Renderer {
	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_SceneData;
	public:
		static void BeginScene(OrthographicCamera& camera);
		static void Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader);
		static void EndScene();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};

}
