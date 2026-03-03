#pragma once

#include "Lore/Renderer/RendererAPI.h"
#include "Lore/Renderer/VertexArray.h"
#include "Lore/Renderer/OrthographicCamera.h"
#include "Lore/Renderer/Shader.h"
#include "Lore/Renderer/ComputeTexture.h"

namespace Lore {

	class Renderer {
	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_SceneData;
	public:
		static void BeginScene(OrthographicCamera& camera);
		static void Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader, const glm::mat4& transform = glm::mat4(1.0f));
		static void EndScene();

		/// Blit a ComputeTexture to the screen using a full-screen triangle.
		static void BlitToScreen(const std::shared_ptr<ComputeTexture>& texture);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};

}
