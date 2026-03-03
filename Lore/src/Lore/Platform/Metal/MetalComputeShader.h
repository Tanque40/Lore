#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Renderer/ComputeShader.h"

namespace Lore {

	class MetalComputeShader : public ComputeShader {
	public:
		MetalComputeShader(const std::string& computePath);
		virtual ~MetalComputeShader();

		virtual void Bind() override;
		virtual void Unbind() override;

		void* GetComputePipelineState() const { return m_ComputePipelineState; }

	private:
		void* m_Library = nullptr;               // id<MTLLibrary>
		void* m_ComputePipelineState = nullptr;  // id<MTLComputePipelineState>
	};

}

#endif // LORE_PLATFORM_MAC
