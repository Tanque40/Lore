#pragma once

#include <atomic>

#include "Lore/Renderer/RendererAPI.h"

#include <glm/glm.hpp>


namespace Lore {

	class MetalRendererAPI : public RendererAPI {
	private:
		// Lo escribe el completion handler del command buffer (otro hilo).
		std::atomic<float> m_LastComputeGpuMs{ -1.0f };
	public:
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

		// Compute pipeline
		virtual void DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) override;
		virtual void ComputeBarrier() override;
		virtual float GetLastComputeGpuMs() const override { return m_LastComputeGpuMs.load(std::memory_order_relaxed); }

		// Blit
		virtual void BlitToScreen(void* nativeTexture, uint32_t width, uint32_t height) override;
	};

}
