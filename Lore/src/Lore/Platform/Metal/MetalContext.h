#pragma once

#ifdef LORE_PLATFORM_MAC

#include <glm/glm.hpp>

#include "Lore/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Lore {

	class MetalContext : public GraphicsContext {
	private:
		static MetalContext* s_Instance;

		GLFWwindow* m_WindowHandle;
		void* m_Device = nullptr;               // id<MTLDevice>
		void* m_CommandQueue = nullptr;          // id<MTLCommandQueue>
		void* m_Layer = nullptr;                 // CAMetalLayer*
		void* m_CurrentDrawable = nullptr;       // id<CAMetalDrawable>
		void* m_CurrentEncoder = nullptr;        // id<MTLRenderCommandEncoder>
		void* m_CurrentCommandBuffer = nullptr;  // id<MTLCommandBuffer>
		void* m_CurrentPassDescriptor = nullptr; // MTLRenderPassDescriptor*
		void* m_OffscreenTexture = nullptr;       // id<MTLTexture> for offscreen rendering

		// Compute state
		void* m_CurrentComputePipeline = nullptr;  // id<MTLComputePipelineState>
		void* m_CurrentComputeTexture = nullptr;   // id<MTLTexture> (weak ref, owned by ComputeTexture)
		void* m_BlitPipelineState = nullptr;       // id<MTLRenderPipelineState> for blit
		const void* m_ComputeUniformData = nullptr;
		size_t m_ComputeUniformSize = 0;

		glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 1.0f };

	public:
		MetalContext(GLFWwindow* windowHandle);
		virtual ~MetalContext();

		virtual void Init() override;
		virtual void SwapBuffers() override;

		// Called at the start of each frame to acquire a drawable and create encoder
		void BeginFrame();

		// Accessors for Metal objects (stored as void* for C++ compatibility)
		void* GetDevice() const { return m_Device; }
		void* GetCommandQueue() const { return m_CommandQueue; }
		void* GetLayer() const { return m_Layer; }
		void* GetCurrentDrawable() const { return m_CurrentDrawable; }
		void* GetCurrentEncoder() const { return m_CurrentEncoder; }
		void* GetCurrentCommandBuffer() const { return m_CurrentCommandBuffer; }
		void* GetCurrentPassDescriptor() const { return m_CurrentPassDescriptor; }

		void SetClearColor(const glm::vec4& color) { m_ClearColor = color; }

		// Offscreen rendering support
		void SetOffscreenTexture(void* texture) { m_OffscreenTexture = texture; }
		void ClearOffscreenTexture() { m_OffscreenTexture = nullptr; }
		void EndCurrentPass();

		// Compute state management
		void SetCurrentComputePipeline(void* pipeline) { m_CurrentComputePipeline = pipeline; }
		void SetCurrentComputeTexture(void* texture) { m_CurrentComputeTexture = texture; }
		void* GetCurrentComputePipeline() const { return m_CurrentComputePipeline; }
		void* GetCurrentComputeTexture() const { return m_CurrentComputeTexture; }
		void SetComputeUniforms(const void* data, size_t size) { m_ComputeUniformData = data; m_ComputeUniformSize = size; }
		const void* GetComputeUniformData() const { return m_ComputeUniformData; }
		size_t GetComputeUniformSize() const { return m_ComputeUniformSize; }

		// Lazily-initialized blit pipeline for full-screen quad rendering
		void* GetBlitPipelineState();

		static MetalContext* Get() { return s_Instance; }
	};

}

#endif // LORE_PLATFORM_MAC
