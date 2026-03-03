#pragma once

#include "Lore/Renderer/RendererAPI.h"

namespace Lore {

	class RenderCommand {
	private:
		static RendererAPI* s_RendererAPI;

	public:
		inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }
		inline static void Clear() { s_RendererAPI->Clear(); }
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) { s_RendererAPI->SetViewport(x, y, width, height); }
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) { s_RendererAPI->DrawIndexed(vertexArray); }

		// Compute pipeline
		inline static void DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) { s_RendererAPI->DispatchCompute(groupX, groupY, groupZ); }
		inline static void ComputeBarrier() { s_RendererAPI->ComputeBarrier(); }

		// Blit a compute texture to screen
		inline static void BlitToScreen(void* nativeTexture, uint32_t width, uint32_t height) { s_RendererAPI->BlitToScreen(nativeTexture, width, height); }

	};

}
