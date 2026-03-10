#pragma once

#ifdef LORE_PLATFORM_WINDOWS

#include "Lore/Renderer/RendererAPI.h"
#include <glm/glm.hpp>

namespace Lore {

	class OpenGLRendererAPI : public RendererAPI {
	public:
		virtual ~OpenGLRendererAPI();

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

		// Compute pipeline
		virtual void DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) override;
		virtual void ComputeBarrier() override;

		// Blit
		virtual void BlitToScreen(void* nativeTexture, uint32_t width, uint32_t height) override;

	private:
		void InitBlitResources();

		uint32_t m_BlitProgram = 0;
		uint32_t m_EmptyVAO = 0;
	};

}

#endif // LORE_PLATFORM_WINDOWS
