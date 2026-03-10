#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalBuffer.h"
#include "MetalContext.h"

#import <Metal/Metal.h>

namespace Lore {

	// * ===================== VertexBuffer =====================

	MetalVertexBuffer::MetalVertexBuffer(float* vertices, uint32_t size) {
		id<MTLDevice> device = (__bridge id<MTLDevice>)MetalContext::Get()->GetDevice();

		id<MTLBuffer> buffer = [device newBufferWithBytes:vertices
		length:size
		options:MTLResourceStorageModeShared];
		LR_CORE_ASSERT(buffer, "Failed to create Metal vertex buffer!");
		m_Buffer = (__bridge_retained void*)buffer;
	}

	MetalVertexBuffer::~MetalVertexBuffer() {
		if (m_Buffer) {
			CFRelease(m_Buffer);
			m_Buffer = nullptr;
		}
	}

	void MetalVertexBuffer::Bind() const {
		if (!MetalContext::Get() || !MetalContext::Get()->GetCurrentEncoder())
			return;

		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)MetalContext::Get()->GetCurrentEncoder();
		id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)m_Buffer;
		[encoder setVertexBuffer:buffer offset:0 atIndex:0];
	}

	void MetalVertexBuffer::Unbind() const {
		// Metal doesn't have a concept of unbinding buffers
	}

	// * ===================== IndexBuffer =====================

	MetalIndexBuffer::MetalIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count) {
		id<MTLDevice> device = (__bridge id<MTLDevice>)MetalContext::Get()->GetDevice();

		id<MTLBuffer> buffer = [device newBufferWithBytes:indices
												  length:count * sizeof(uint32_t)
												 options:MTLResourceStorageModeShared];
		LR_CORE_ASSERT(buffer, "Failed to create Metal index buffer!");
		m_Buffer = (__bridge_retained void*)buffer;
	}

	MetalIndexBuffer::~MetalIndexBuffer() {
		if (m_Buffer) {
			CFRelease(m_Buffer);
			m_Buffer = nullptr;
		}
	}

	void MetalIndexBuffer::Bind() const {
		// Index buffers in Metal are passed directly to drawIndexedPrimitives
		// No need to bind them to the encoder separately
	}

	void MetalIndexBuffer::Unbind() const {
		// Metal doesn't have a concept of unbinding buffers
	}

MetalStorageBuffer::MetalStorageBuffer(uint32_t size, uint32_t binding) : m_Binding(binding) {
		id<MTLDevice> device = (__bridge id<MTLDevice>)MetalContext::Get()->GetDevice();

		id<MTLBuffer> buffer = [device newBufferWithLength:size
												   options:MTLResourceStorageModeShared];
		LR_CORE_ASSERT(buffer, "Failed to create Metal storage buffer!");
		m_Buffer = (__bridge_retained void*)buffer;
	}

	MetalStorageBuffer::~MetalStorageBuffer() {
		if (m_Buffer) {
			CFRelease(m_Buffer);
			m_Buffer = nullptr;
		}
	}

	void MetalStorageBuffer::Bind(uint32_t slot) const {
		if (!MetalContext::Get())
			return;

		// Stage the buffer in MetalContext; it will be bound to the compute encoder in DispatchCompute
		MetalContext::Get()->SetComputeBuffer(m_Buffer, slot);
	}

	void MetalStorageBuffer::Unbind() const {
		// Metal no requiere unbind
	}

	void MetalStorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset) {
		id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)m_Buffer;
		void* gpuPtr = (uint8_t*)[buffer contents] + offset;
		memcpy(gpuPtr, data, size);
	}

}

#endif // LORE_PLATFORM_MAC
