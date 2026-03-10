#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalRendererAPI.h"
#include "MetalContext.h"
#include "MetalBuffer.h"
#include "Lore/Renderer/VertexArray.h"

#import <Metal/Metal.h>

namespace Lore {

	void MetalRendererAPI::SetClearColor(const glm::vec4& color) {
		if (MetalContext::Get()) {
			MetalContext::Get()->SetClearColor(color);
		}
	}

	void MetalRendererAPI::Clear() {
		// In Metal, clearing is done via the render pass descriptor's loadAction
		// which is already set to MTLLoadActionClear in BeginFrame.
		// We just need to make sure BeginFrame has been called.
		if (MetalContext::Get()) {
			MetalContext::Get()->BeginFrame();
		}
	}

	void MetalRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) {
		if (!MetalContext::Get() || !MetalContext::Get()->GetCurrentEncoder())
			return;

		// Bind vertex buffers to the encoder
		const auto& vertexBuffers = vertexArray->GetVertexBuffers();
		for (size_t i = 0; i < vertexBuffers.size(); i++) {
			vertexBuffers[i]->Bind();
		}

		const std::shared_ptr<IndexBuffer>& indexBuffer = vertexArray->GetIndexBuffer();
		if (!indexBuffer)
			return;

		// GetNativeHandle() returns the id<MTLBuffer> directly as void*
		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)MetalContext::Get()->GetCurrentEncoder();
		id<MTLBuffer> mtlIndexBuffer = (__bridge id<MTLBuffer>)indexBuffer->GetNativeHandle();

		[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
		indexCount:indexBuffer->GetCount()
		indexType:MTLIndexTypeUInt32
		indexBuffer:mtlIndexBuffer
		indexBufferOffset:0];
	}

	void MetalRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		if (!MetalContext::Get() || !MetalContext::Get()->GetCurrentEncoder())
			return;

		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)MetalContext::Get()->GetCurrentEncoder();

		MTLViewport viewport;
		viewport.originX = x;
		viewport.originY = y;
		viewport.width = width;
		viewport.height = height;
		viewport.znear = 0.0;
		viewport.zfar = 1.0;
		[encoder setViewport:viewport];
	}

	void MetalRendererAPI::DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) {
		MetalContext* ctx = MetalContext::Get();
		if (!ctx) return;

		// End any active render pass before dispatching compute work
		if (ctx->GetCurrentEncoder()) {
			ctx->EndCurrentPass();
		}

		id<MTLCommandQueue> commandQueue = (__bridge id<MTLCommandQueue>)ctx->GetCommandQueue();
		id<MTLComputePipelineState> computePipeline = (__bridge id<MTLComputePipelineState>)ctx->GetCurrentComputePipeline();
		id<MTLTexture> computeTexture = (__bridge id<MTLTexture>)ctx->GetCurrentComputeTexture();

		if (!computePipeline) {
			LR_CORE_ERROR("No compute pipeline bound! Call ComputeShader::Bind() before DispatchCompute.");
			return;
		}

		// Create a new command buffer for compute work
		id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
		commandBuffer.label = @"LoreComputeCommandBuffer";

		id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
		computeEncoder.label = @"LoreComputeEncoder";

		[computeEncoder setComputePipelineState:computePipeline];

		if (computeTexture) {
			[computeEncoder setTexture:computeTexture atIndex:0];
		}

		// Bind staged compute buffers (e.g. SVO storage buffer)
		for (int i = 0; i < MetalContext::MAX_COMPUTE_BUFFERS; ++i) {
			if (ctx->IsComputeBufferBound(i)) {
				id<MTLBuffer> buf = (__bridge id<MTLBuffer>)ctx->GetComputeBuffer(i);
				[computeEncoder setBuffer:buf offset:0 atIndex:i];
			}
		}

		const void* uniformData = ctx->GetComputeUniformData();
		if (uniformData) {
			[computeEncoder setBytes:uniformData length:ctx->GetComputeUniformSize() atIndex:1];
		}

		// Threadgroup size of 16x16x1 — compute shaders should use matching local size
		MTLSize threadgroupSize = MTLSizeMake(16, 16, 1);
		MTLSize threadgroupCount = MTLSizeMake(groupX, groupY, groupZ);

		[computeEncoder dispatchThreadgroups:threadgroupCount threadsPerThreadgroup:threadgroupSize];
		[computeEncoder endEncoding];

		[commandBuffer commit];
		[commandBuffer waitUntilCompleted];
	}

	void MetalRendererAPI::ComputeBarrier() {
		// Metal synchronization is implicit after waitUntilCompleted in DispatchCompute.
		// No additional barrier needed.
	}

	void MetalRendererAPI::BlitToScreen(void* nativeTexture, uint32_t width, uint32_t height) {
		MetalContext* ctx = MetalContext::Get();
		if (!ctx || !ctx->GetCurrentEncoder()) return;

		void* blitPipeline = ctx->GetBlitPipelineState();
		if (!blitPipeline) {
			LR_CORE_ERROR("Failed to get blit pipeline state!");
			return;
		}

		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)ctx->GetCurrentEncoder();
		id<MTLRenderPipelineState> pipelineState = (__bridge id<MTLRenderPipelineState>)blitPipeline;
		id<MTLTexture> texture = (__bridge id<MTLTexture>)nativeTexture;

		[encoder setRenderPipelineState:pipelineState];
		[encoder setFragmentTexture:texture atIndex:0];

		// Draw full-screen triangle (3 vertices, no vertex buffer)
		[encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
	}

}

#endif // LORE_PLATFORM_MAC
