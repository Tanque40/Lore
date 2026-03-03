#include "lrpch.h"

#include "Lore/Renderer/Renderer.h"
#include "Lore/Renderer/Buffer.h"
#include "Lore/Platform/OpenGL/OpenGLBuffer.h"

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalBuffer.h"
#endif

namespace Lore {

	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			LR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexBuffer(vertices, size);
			break;
#ifdef LORE_PLATFORM_MAC
		case RendererAPI::API::Metal:
			return new MetalVertexBuffer(vertices, size);
			break;
#endif
		}

		LR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count) {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			LR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLIndexBuffer(indices, count);
			break;
#ifdef LORE_PLATFORM_MAC
		case RendererAPI::API::Metal:
			return new MetalIndexBuffer(indices, count);
			break;
#endif
		}

		LR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
