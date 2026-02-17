#include "lrpch.h"
#include "VertexArray.h"
#include "RendererAPI.h"

#include "Lore/Platform/OpenGL/OpenGLVertexArray.h"

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalVertexArray.h"
#endif

namespace Lore {

	VertexArray* VertexArray::Create() {
		switch (RendererAPI::GetAPI()) {
		case RendererAPIType::OpenGL: return new OpenGLVertexArray();
#ifdef LORE_PLATFORM_MAC
		case RendererAPIType::Metal:  return new MetalVertexArray();
#endif
		default:
			LR_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

}
