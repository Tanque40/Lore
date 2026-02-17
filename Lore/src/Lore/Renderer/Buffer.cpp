#include "lrpch.h"
#include "Buffer.h"
#include "RendererAPI.h"

#include "Lore/Platform/OpenGL/OpenGLBuffer.h"

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalBuffer.h"
#endif

namespace Lore {

	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
		switch (RendererAPI::GetAPI()) {
		case RendererAPIType::OpenGL: return new OpenGLVertexBuffer(vertices, size);
#ifdef LORE_PLATFORM_MAC
		case RendererAPIType::Metal:  return new MetalVertexBuffer(vertices, size);
#endif
		default:
			LR_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) {
		switch (RendererAPI::GetAPI()) {
		case RendererAPIType::OpenGL: return new OpenGLIndexBuffer(indices, count);
#ifdef LORE_PLATFORM_MAC
		case RendererAPIType::Metal:  return new MetalIndexBuffer(indices, count);
#endif
		default:
			LR_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

}
