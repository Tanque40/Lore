#include "lrpch.h"

#include "Lore/Renderer/VertexArray.h"

#include "Lore/Renderer/Renderer.h"

#ifdef LORE_PLATFORM_WINDOWS
#include "Lore/Platform/OpenGL/OpenGLVertexArray.h"
#endif

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalVertexArray.h"
#endif

namespace Lore {

	VertexArray* VertexArray::Create() {
		switch (Renderer::GetAPI()) {
		case RendererAPI::API::None:
			LR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
#ifdef LORE_PLATFORM_WINDOWS
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexArray();
			break;
#endif // LORE_PLATFORM_WINDOWS
#ifdef LORE_PLATFORM_MAC
		case RendererAPI::API::Metal:
			return new MetalVertexArray();
			break;
#endif
		}

		LR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
