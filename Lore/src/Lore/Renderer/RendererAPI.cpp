#include "lrpch.h"
#include "RendererAPI.h"

#include "Lore/Platform/OpenGL/OpenGLRendererAPI.h"

#ifdef LORE_PLATFORM_MAC
#include "Lore/Platform/Metal/MetalRendererAPI.h"
#endif

namespace Lore {

#ifdef LORE_PLATFORM_MAC
	RendererAPIType RendererAPI::s_API = RendererAPIType::Metal;
#else
	RendererAPIType RendererAPI::s_API = RendererAPIType::OpenGL;
#endif

	RendererAPI* RendererAPI::Create() {
		switch (s_API) {
		case RendererAPIType::OpenGL: return new OpenGLRendererAPI();
#ifdef LORE_PLATFORM_MAC
		case RendererAPIType::Metal:  return new MetalRendererAPI();
#endif
		default:
			LR_CORE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}

}
