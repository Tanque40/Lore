#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalRendererAPI.h"
#include "MetalContext.h"
#include "MetalBuffer.h"
#include "Lore/Renderer/VertexArray.h"

#import <Metal/Metal.h>

namespace Lore {

	void MetalRendererAPI::Init() {
		LR_CORE_INFO("Metal Renderer API initialized.");
	}

	void MetalRendererAPI::SetClearColor(float r, float g, float b, float a) {
		if (MetalContext::Get()) {
			MetalContext::Get()->SetClearColor(r, g, b, a);
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

	void MetalRendererAPI::DrawIndexed(const VertexArray* vertexArray) {
		if (!MetalContext::Get() || !MetalContext::Get()->GetCurrentEncoder())
			return;

		const IndexBuffer* indexBuffer = vertexArray->GetIndexBuffer();
		if (!indexBuffer)
			return;

		const MetalIndexBuffer* metalIB = static_cast<const MetalIndexBuffer*>(indexBuffer);
		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)MetalContext::Get()->GetCurrentEncoder();
		id<MTLBuffer> mtlIndexBuffer = (__bridge id<MTLBuffer>)metalIB->GetMetalBuffer();

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

}

#endif
